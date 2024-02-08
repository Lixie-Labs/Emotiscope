import struct
import wave

def binary_to_wav(input_file, output_file, sample_rate=12800, num_channels=1, sample_width=2):
    # Open the binary file for reading
    with open(input_file, 'rb') as f:
        # Read all the binary data
        binary_data = f.read()
    
    # Calculate the number of frames (samples)
    num_frames = len(binary_data) // (sample_width * num_channels)
    
    # Create a new WAV file
    with wave.open(output_file, 'wb') as wav_file:
        # Set the WAV file parameters
        wav_file.setnchannels(num_channels)
        wav_file.setsampwidth(sample_width)
        wav_file.setframerate(sample_rate)
        wav_file.setnframes(num_frames)
        
        # Convert binary data to int16 values and write to WAV file
        for i in range(0, len(binary_data), sample_width):
            # Unpack binary data as int16_t
            value = struct.unpack('<h', binary_data[i:i+sample_width])[0]
            # Write the value to the WAV file
            wav_file.writeframes(struct.pack('<h', value))

# Example usage:
binary_to_wav("audio.bin", "output.wav")
