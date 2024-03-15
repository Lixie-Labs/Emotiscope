import mido
from PIL import Image, ImageOps
import numpy as np

# Open the MIDI file
midi_file = mido.MidiFile('input.mid')

# Initialize the note dictionary
note_dict = {}

# Process the MIDI messages
current_time = 0
for msg in midi_file:
    current_time += msg.time
    if msg.type == 'note_on' and msg.velocity > 0:
        note_dict[msg.note] = {'start_time': current_time, 'end_time': current_time}
    elif (msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0)) and msg.note in note_dict:
        note_dict[msg.note]['end_time'] = current_time

# Initialize the numpy array for the PNG image
image_data = np.zeros((int(midi_file.length*200), 64), dtype=np.uint8)

# Set the pixels for each note in the numpy array
for note, value in note_dict.items():
    if isinstance(value, dict):
        start_time = value['start_time']
        end_time = value['end_time']
        if 45 <= note <= 109:
            start_frame = int(start_time * 200)
            end_frame = int(end_time * 200)
            image_data[start_frame:end_frame, note-45] = 255

# Create an image from the numpy array
image = Image.fromarray(image_data.T)

# Flip the image vertically
image = ImageOps.flip(image)

# Save the image as a PNG file
image.save('output.png')