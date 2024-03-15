import os
import shutil
import mido

# Define the input and output directories
input_dir = 'STEP 1 - INPUT'
output_dir = 'STEP 2 - NOTE RANGE TRIMMED'

# Define the note range limits
lower_limit = 45
upper_limit = 109

# Iterate over all MIDI files in the input directory
for filename in os.listdir(input_dir):
	if filename.endswith('.mid'):
		# Load the MIDI file
		midi_file = mido.MidiFile(os.path.join(input_dir, filename))
		
		# Create a new MIDI file with the same name in the output directory
		output_filename = os.path.join(output_dir, filename)
		shutil.copyfile(os.path.join(input_dir, filename), output_filename)
		new_midi_file = mido.MidiFile(output_filename)
		
		print("TRIMMING NOTE RANGE:", filename, "TO:", lower_limit, "-", upper_limit)
		# Iterate over all tracks in the MIDI file
		for track in new_midi_file.tracks:
			# Skip the drum channel
			if track.name == 'Drums':
				continue

			i = 0
			while i < len(track):
				msg = track[i]
				# Skip the drum channel (channel 10 in MIDI)
				if msg.type in ['note_on', 'note_off'] and msg.channel == 9:
					i += 1
					continue
				# Remove notes outside the specified range
				if msg.type in ['note_on', 'note_off'] and (msg.note < lower_limit or msg.note > upper_limit):
					if i < len(track) - 1:
						# Add the time of the removed note to the next note
						track[i + 1].time += msg.time
					del track[i]
				else:
					i += 1

		# Save the modified MIDI file
		new_midi_file.save(output_filename)
		