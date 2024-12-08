import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt

# Load the numbers from the file into a NumPy array
x = np.loadtxt('./data/sinData.txt')
y = np.loadtxt('./data/sinData_deltaSigma.txt')

w = signal.hann(len(x), False)
x_win = x*w
y_win = y*w

N = len(x)

XdB = 20*np.log10(np.abs(np.fft.fftshift(np.fft.fft(x_win))))[N//2:]
YdB = 20*np.log10(np.abs(np.fft.fftshift(np.fft.fft(y_win))))[N//2:]

freqs = (np.arange(N) / N - 0.5)[N//2:]

# Plotting
plt.figure(figsize=(14, 12))

# First subplot for XdB
plt.subplot(2, 1, 1)  # 2 rows, 1 column, subplot 1
plt.plot(freqs, XdB-np.max(XdB), label='XdB', color='blue')
plt.title('FFT of x')
plt.xlabel('f/fs')
plt.ylabel('|X|dB')
plt.grid()

# Second subplot for YdB
plt.subplot(2, 1, 2)  # 2 rows, 1 column, subplot 2
plt.plot(freqs, YdB-np.max(YdB), label='YdB', color='red')
plt.title('FFT of y')
plt.xlabel('f/fs')
plt.ylabel('|Y|dB')
plt.grid()
plt.show();