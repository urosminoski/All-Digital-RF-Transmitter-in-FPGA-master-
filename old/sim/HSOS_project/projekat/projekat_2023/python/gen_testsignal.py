
import numpy as np
from matplotlib.pylab import *

fs = 61.44e6
fsigmax = 25e6

N = 1024

indmin = int(N/2-fsigmax/(fs/2)*N/2)
indmax = int(N/2+fsigmax/(fs/2)*N/2)

spectrum = np.zeros(N)
spectrum[indmin:indmax] = 1.0 + np.arange(indmax-indmin)/(indmax-indmin)

samples = np.fft.ifft(np.fft.fftshift(spectrum * np.exp(1j*2*np.pi*np.random.randn(N))))

outFile = open("testsignal.txt", "w")
for sample in samples:
    outFile.write( str(sample) + "\n")
    
outFile.close()

plot(np.fft.fftshift(abs(np.fft.fft(samples))))
show()



