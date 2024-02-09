import asyncio
import websockets
import requests  # For making HTTP GET requests
import time
import wave
import os

async def communicate_with_server():
    uri = "ws://192.168.1.49:80/ws"

    # Connect to the WebSocket server
    async with websockets.connect(uri) as websocket:
        # Send the 'start_debug_recording' command to the server
        await websocket.send("start_debug_recording")
        print("Sent: start_debug_recording")

        # Wait for a response from the server
        while True:
            response = await websocket.recv()
            print(f"Received: {response}")

            # Check if the response is 'debug_recording_ready'
            if response == "debug_recording_ready":
                print("Debug recording is ready. Downloading audio...")

                time.sleep(3)

                # Make a GET request to download the audio file
                audio_response = requests.get("http://192.168.1.49/audio")

                # Ensure the request was successful
                if audio_response.status_code == 200:
                    # Save the content to 'audio.bin'
                    with open("audio.bin", "wb") as audio_file:
                        audio_file.write(audio_response.content)
                    print("Audio file downloaded and saved as audio.bin.")
                    
                    # Convert audio.bin to audio.wav
                    convert_bin_to_wav("audio.bin", "audio.wav")
                    
                    # Delete the original audio.bin file
                    os.remove("audio.bin")
                    print("audio.bin has been deleted.")

                else:
                    print(f"Failed to download audio file. HTTP status code: {audio_response.status_code}")

                break  # Exit the loop after handling the audio file

def convert_bin_to_wav(input_filename, output_filename):
    # Read the .bin file
    with open(input_filename, "rb") as bin_file:
        data = bin_file.read()

    # Set parameters for the output WAV file
    sample_rate = 12800  # Sample rate in Hz
    channels = 1  # Mono audio
    sample_width = 2  # int16_t samples, so 2 bytes per sample

    # Create and configure the WAV file
    with wave.open(output_filename, "w") as wav_file:
        wav_file.setnchannels(channels)
        wav_file.setsampwidth(sample_width)
        wav_file.setframerate(sample_rate)
        wav_file.writeframes(data)

    print(f"Converted {input_filename} to {output_filename}.")

# Start the WebSocket communication
asyncio.get_event_loop().run_until_complete(communicate_with_server())
