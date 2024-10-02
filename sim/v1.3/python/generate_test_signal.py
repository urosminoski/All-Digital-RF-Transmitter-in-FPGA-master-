import numpy as np
import matplotlib.pyplot as plt

F = 0.2         # Digital frequency
OSR = 8         # Oversamplig ratio
N = 8192        # Number of samples
M = 5           # Resolution of decimator

n = np.arange(N)
x = 2**(3) * np.sin(2 * np.pi * np.floor(2/7 * N/OSR/2) * n/N)

# Save x to a text file
np.savetxt("../data/input_signal.txt", x, fmt="%.15f")

x_fft = np.fft.fft(x)
x_fft /= np.max(np.abs(x_fft))
x_fft_dB = 20*np.log10(np.abs(x_fft))
x_fft_dB = x_fft_dB[:N//2]

freqs = n / N
freqs = freqs[:N//2]

# Create subplots
fig, axs = plt.subplots(2, 1, figsize=(10, 8))

# Plot the time-domain signal
axs[0].plot(x)
axs[0].set_title("Time-Domain Signal")
axs[0].set_xlabel("Sample Index")
axs[0].set_ylabel("Amplitude")
axs[0].grid(True)

# Plot the frequency-domain signal (FFT in dB)
axs[1].plot(freqs, x_fft_dB)
axs[1].set_title("Frequency-Domain Signal (FFT in dB)")
axs[1].set_xlabel("Frequency")
axs[1].set_ylabel("Magnitude (dB)")
axs[1].grid(True)

# Adjust layout to prevent overlap
plt.tight_layout()

# Show the plots
plt.show()