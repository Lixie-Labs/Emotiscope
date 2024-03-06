#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>

#define CHUNK_SIZE 64
float magnitudes[64]; // Global array to store the magnitudes

// Your function (to be implemented by you)
void process_goertzel_chunk(float* chunk_samples);

// Basic WAV header structure
struct WAVHeader {
    char riff_header[4]; // Contains "RIFF"
    uint32_t wav_size;
    char wave_header[4]; // Contains "WAVE"
    char fmt_header[4]; // Contains "fmt "
    uint32_t fmt_chunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t sample_alignment;
    uint16_t bit_depth;
};

// Function to read WAV file and return a vector of 16-bit samples
std::vector<int16_t> read_wav_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::vector<int16_t> samples;

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return samples;
    }

    // Read WAV header
    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    // Check file format
    if (std::strncmp(header.riff_header, "RIFF", 4) != 0 || std::strncmp(header.wave_header, "WAVE", 4) != 0) {
        std::cerr << "Invalid file format. This is not a valid WAV file." << std::endl;
        return samples;
    }

    // Check for 16-bit PCM audio format
    if (header.audio_format != 1 || header.bit_depth != 16) {
        std::cerr << "Unsupported WAV format. Only 16-bit PCM is supported." << std::endl;
        return samples;
    }

    // Check sample rate
    if (header.sample_rate != 12800) {
        std::cerr << "Unexpected sample rate. Expected 12800 Hz." << std::endl;
        return samples;
    }

    // Check for mono channel
    if (header.num_channels != 1) {
        std::cerr << "Invalid number of channels. Only mono is supported." << std::endl;
        return samples;
    }

    // Skip over any additional header chunks until "data" chunk is found
    char sub_chunk_id[4];
    uint32_t sub_chunk_size;
    file.read(sub_chunk_id, 4);
    file.read(reinterpret_cast<char*>(&sub_chunk_size), sizeof(uint32_t));
    while (std::strncmp(sub_chunk_id, "data", 4) != 0) {
        // Skip over this chunk
        file.seekg(sub_chunk_size, std::ios_base::cur);

        // Read next chunk
        file.read(sub_chunk_id, 4);
        if (file.eof()) {
            std::cerr << "Reached end of file without finding 'data' chunk." << std::endl;
            return samples;
        }
        file.read(reinterpret_cast<char*>(&sub_chunk_size), sizeof(uint32_t));
    }

    // Now at the start of the "data" chunk, read in sample data
    int16_t sample;
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(sample))) {
        samples.push_back(sample);
    }

    return samples;
}

// Function to write magnitudes to a binary file
void write_magnitudes_to_bin(const std::string& filename, const std::vector<uint32_t>& magnitudes) {
    std::ofstream out_file(filename, std::ios::binary);
    for (auto magnitude : magnitudes) {
        out_file.write(reinterpret_cast<const char*>(&magnitude), sizeof(magnitude));
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_wav_file>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = input_filename.substr(0, input_filename.rfind('.')) + ".bin";

    // Read WAV file into a vector of 16-bit samples
    std::vector<int16_t> samples = read_wav_file(input_filename);

    // Prepare a vector to hold quantized magnitudes
    std::vector<uint32_t> quantized_magnitudes;

    // Process each chunk
    for (size_t i = 0; i + CHUNK_SIZE <= samples.size(); i += CHUNK_SIZE) {
        float chunk_samples[CHUNK_SIZE];
        
        // Convert and scale 16-bit samples to float [-1.0, 1.0]
        for (size_t j = 0; j < CHUNK_SIZE; ++j) {
            chunk_samples[j] = samples[i + j] / 32768.0f;
        }

        // Process chunk using your function
        process_goertzel_chunk(chunk_samples);

        // Quantize magnitudes and add to the vector
        for (float magnitude : magnitudes) {
            quantized_magnitudes.push_back(static_cast<uint32_t>(magnitude * UINT32_MAX));
        }
    }

    // Write the quantized magnitudes to a binary file
    write_magnitudes_to_bin(output_filename, quantized_magnitudes);

    std::cout << "Processing completed. Output written to " << output_filename << std::endl;
    return 0;
}
