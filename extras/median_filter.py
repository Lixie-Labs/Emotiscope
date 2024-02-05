import numpy as np
import librosa
import librosa.display
import matplotlib.pyplot as plt

def median_filter(spectrogram, filter_size=7):
    """Applies a median filter to each column of a spectrogram in a real-time manner."""
    num_rows, num_columns = spectrogram.shape
    filtered_spectrogram = np.zeros_like(spectrogram)

    for j in range(num_columns):
        # Determine the range of columns to include in the median calculation
        start = max(0, j - filter_size // 2)
        end = j + 1  # Include up to the current column
        # Calculate median for each row within the determined range
        for i in range(num_rows):
            filtered_spectrogram[i, j] = np.median(spectrogram[i, start:end])
    
    return filtered_spectrogram

print("LOAD")
# Load the audio file
file_path = 'input.mp3'
audio, sr = librosa.load(file_path, sr=None)

print("STFT")
# Compute the Short-Time Fourier Transform (STFT)
stft = librosa.stft(audio)

print("SPECTROGRAM")
# Convert to spectrogram
spectrogram = np.abs(stft)

print("MEDIAN")
# Apply median filter to each column of the spectrogram in a real-time manner
filtered_spectrogram = median_filter(spectrogram, filter_size=150)

print("PLOT")
# Plot the original and filtered spectrograms
plt.figure(figsize=(15, 8))

plt.subplot(2, 1, 1)
librosa.display.specshow(librosa.amplitude_to_db(spectrogram, ref=np.max), sr=sr, y_axis='log', x_axis='time')
plt.title('Original Spectrogram')
plt.colorbar(format='%+2.0f dB')

plt.subplot(2, 1, 2)
librosa.display.specshow(librosa.amplitude_to_db(filtered_spectrogram, ref=np.max), sr=sr, y_axis='log', x_axis='time')
plt.title('Filtered Spectrogram')
plt.colorbar(format='%+2.0f dB')

plt.tight_layout()
plt.show()
