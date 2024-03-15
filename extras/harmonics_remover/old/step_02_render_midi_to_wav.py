import os
import glob
import numpy as np
from scipy.io import wavfile
from midi2audio import FluidSynth

# Set the input and output directories
input_dir = "STEP 2 - NOTE RANGE TRIMMED"
output_dir = "STEP 3 - RENDERED TO WAV"

# Set the desired sample rate and bit depth
sample_rate = 12800
bit_depth = 16

# Initialize the FluidSynth synthesizer
fs = FluidSynth()

# Iterate over all MIDI files in the input directory
for midi_file in glob.glob(os.path.join(input_dir, "*.mid")):
	# Get the output file path
	output_file = os.path.join(output_dir, os.path.basename(midi_file).replace(".mid", ".wav"))

	# Render the MIDI notes to an audio file
	print("RENDERING:", midi_file, "TO:", output_file)
	fs.midi_to_audio(midi_file, output_file)

	# Read the audio file
	_, audio_buffer = wavfile.read(output_file)

	# If audio_buffer is stereo, convert to mono
	if audio_buffer.ndim > 1:
		audio_buffer = np.mean(audio_buffer, axis=1)

	# Downsample the audio buffer
	print("DOWNSAMPLING:", output_file, "FROM:", fs.sample_rate, "TO:", sample_rate)
	downsampled_buffer = np.interp(
		np.linspace(0, len(audio_buffer), len(audio_buffer) * sample_rate // fs.sample_rate),
		np.arange(len(audio_buffer)),
		audio_buffer
	)

	# Normalize the audio buffer
	normalized_buffer = downsampled_buffer / np.max(np.abs(downsampled_buffer))

	# Convert the audio buffer to 16-bit integers
	audio_data = (normalized_buffer * (2 ** (bit_depth - 1) - 1)).astype(np.int16)

	# Save the audio data to a WAV file
	wavfile.write(output_file, sample_rate, audio_data)