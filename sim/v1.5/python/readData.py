import numpy as np
import scipy.signal as signal
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# Enable LaTeX rendering
plt.rc('text', usetex=True)
plt.rc('font', family='serif')

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

OSR = 8

# Plotting
plt.figure(figsize=(14, 12))

# First subplot for XdB
plt.subplot(2, 1, 1)  # 2 rows, 1 column, subplot 1
plt.plot(freqs, XdB-np.max(XdB), label='XdB', color='blue')
plt.plot([0.5/OSR, 0.5/OSR], [np.min(XdB-np.max(XdB)), np.max(XdB-np.max(XdB))], '--k', linewidth=1)
plt.title(r'Spectrum of sin wave', fontsize=16)
plt.xlabel(r'$f \, / \, f_s$', fontsize=14)
plt.ylabel(r'$|X|_{dB}$', fontsize=14)
plt.grid()

# Second subplot for YdB
plt.subplot(2, 1, 2)  # 2 rows, 1 column, subplot 2
plt.plot(freqs, YdB-np.max(YdB), label='YdB', color='red')
plt.plot([0.5/OSR, 0.5/OSR], [np.min(YdB-np.max(YdB)), np.max(YdB-np.max(YdB))], '--k', linewidth=1)
plt.title(r'Spectrum of modulated sin wave', fontsize=16)
plt.xlabel(r'$f \, / \, f_s$', fontsize=14)
plt.ylabel(r'$|Y|_{dB}$', fontsize=14)
plt.grid()
#plt.show();

plt.savefig("./figs/sinData_deltaSigma.png");


y_serial = np.loadtxt('./data/sinData_serialData.txt')

w = signal.hann(len(y_serial), False)
y_serial_win = y_serial*w

N = len(y_serial)

YsdB = 20*np.log10(np.abs(np.fft.fftshift(np.fft.fft(y_serial_win))))[N//2:]

freqs = (np.arange(N) / N - 0.5)[N//2:]
# Plotting
plt.figure(figsize=(14, 6))

# First subplot for XdB
# plt.subplot(2, 1, 1)  # 2 rows, 1 column, subplot 1
plt.plot(freqs, YsdB-np.max(YsdB), label='XdB', color='blue')
# plt.plot([0.5/OSR, 0.5/OSR], [np.min(XdB-np.max(XdB)), np.max(XdB-np.max(XdB))], '--k', linewidth=1)
plt.title(r'Spectrum of sin wave', fontsize=16)
plt.xlabel(r'$f \, / \, f_s$', fontsize=14)
plt.ylabel(r'$|X|_{dB}$', fontsize=14)
plt.grid()

plt.savefig("./figs/sinData_serialData.png");
