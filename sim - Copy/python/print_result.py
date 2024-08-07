import numpy as np
import matplotlib.pyplot as plt

# Read the file into a NumPy array
y = np.loadtxt("../data/output_signal.txt")

print(y[:10])

# Perform FFT and convert to dB scale
y_fft = np.fft.fftshift(np.fft.fft(y)) + 1e-12
y_fft_dB = 20 * np.log10(np.abs(y_fft) /np.max(y_fft))  # Add small epsilon to avoid log10(0)

# Generate frequency bins
freqs = np.fft.fftshift(np.fft.fftfreq(len(y)))

# Create subplots
fig, axs = plt.subplots(2, 1, figsize=(10, 8))

# Plot the time-domain signal
axs[0].plot(y)
axs[0].set_title("Time-Domain Signal")
axs[0].set_xlabel("Sample Index")
axs[0].set_ylabel("Amplitude")
axs[0].grid(True)

# Plot the frequency-domain signal (FFT in dB)
axs[1].plot(freqs, y_fft_dB)
axs[1].set_title("Frequency-Domain Signal (FFT in dB)")
axs[1].set_xlabel("Frequency")
axs[1].set_ylabel("Magnitude (dB)")
axs[1].grid(True)

# Adjust layout to prevent overlap
plt.tight_layout()

# Show the plots
plt.show()
