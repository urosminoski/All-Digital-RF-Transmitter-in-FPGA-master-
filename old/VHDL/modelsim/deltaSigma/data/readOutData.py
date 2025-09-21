import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt

def fft_dB(x):
    y_fft = np.fft.fft(x)
    y_fft_dB = 20*np.log10(np.abs(y_fft))
    y_fft_dB -= np.max(y_fft_dB)
    y_fft_dB = y_fft_dB[:len(y)//2]
    return y_fft_dB

# Open the text file for reading
with open("output_data.txt", "r") as file:
    # Read each line, strip whitespace, and convert it to an integer
    y = np.array([int(line.strip()) for line in file])
y = y[:-2]
    
# Open the text file for reading
with open("input_data.txt", "r") as file:
    # Read each line, strip whitespace, and convert it to an integer
    x = np.array([float(line.strip()) for line in file])
    
w = signal.blackman(len(x), True)
x_fft = np.fft.fft(x*1)
x_fft_dB = 20*np.log10(np.abs(x_fft))
x_fft_dB -= np.max(x_fft_dB)
x_fft_dB = x_fft_dB[:len(x)//2]
    
w1 = signal.blackman(len(y), False)
w2 = signal.barthann(len(y), False)
w3 = signal.bartlett(len(y), False)
y1 = fft_dB(w1*y)
y2 = fft_dB(w2*y)
y3 = fft_dB(w3*y)

freqs = np.linspace(0, 0.5, len(y)//2)

# plt.plot(freqs, x_fft_dB)
plt.plot(freqs, y1)
# plt.plot(freqs, y2)
# plt.plot(freqs, y3)
plt.show