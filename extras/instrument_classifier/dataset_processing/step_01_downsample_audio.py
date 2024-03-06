from pydub import AudioSegment
import os

# Define the input and output directories
input_directory = "Step 1 - INPUT"
output_directory = "Step 2 - DOWNSAMPLED"

# Ensure the output directory exists
if not os.path.exists(output_directory):
    os.makedirs(output_directory)

# Iterate over all .OGG files in the input directory
for filename in os.listdir(input_directory):
    if filename.endswith(".ogg"):
        input_path = os.path.join(input_directory, filename)
        output_path = os.path.join(output_directory, os.path.splitext(filename)[0] + ".wav")

        # Load the OGG file
        audio = AudioSegment.from_ogg(input_path)

        # Downsample to 12800 Hz and convert to mono
        downsampled_audio = audio.set_frame_rate(12800).set_channels(1)

        # Export as 16-bit WAV (lossless with signed 16-bit samples)
        downsampled_audio.export(output_path, format="wav", parameters=["-acodec", "pcm_s16le"])

        print(f"Processed and saved: {output_path}")
