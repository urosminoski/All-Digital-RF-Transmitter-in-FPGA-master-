#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

# Read the file into a NumPy array
y = np.loadtxt("data/output_signal.txt")

# Perform FFT and convert to dB scale
y_fft = np.fft.fftshift(np.fft.fft(y))
y_fft /= np.max(np.abs(y_fft))
y_fft_dB = 20 * np.log10(np.abs(y_fft))  # Add small epsilon to avoid log10(0)
y_fft_dB = y_fft_dB[len(y_fft_dB)//2:]

# Generate frequency bins
freqs = np.fft.fftshift(np.fft.fftfreq(len(y)))
freqs = freqs[len(freqs)//2:]

plt.plot(freqs, y_fft_dB)
plt.show()

# # Create subplots
# fig, axs = plt.subplots(2, 1, figsize=(10, 8))

# # Plot the time-domain signal
# axs[0].plot(y)
# axs[0].set_title("Time-Domain Signal")
# axs[0].set_xlabel("Sample Index")
# axs[0].set_ylabel("Amplitude")
# axs[0].grid(True)

# # Plot the frequency-domain signal (FFT in dB)
# axs[1].plot(freqs, y_fft_dB)
# axs[1].set_title("Frequency-Domain Signal (FFT in dB)")
# axs[1].set_xlabel("Frequency")
# axs[1].set_ylabel("Magnitude (dB)")
# axs[1].grid(True)

# # Adjust layout to prevent overlap
# plt.tight_layout()

# # Show the plots
# plt.show()
