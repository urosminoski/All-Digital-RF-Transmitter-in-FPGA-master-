
import numpy as np
from matplotlib.pylab import *

def readSamples(fileName):
    samples = []
    inFile = open(fileName, "r")
    for line in inFile:
        samples.append( complex(line) )
    return np.array(samples)

samples = readSamples("testsignal.txt")
samples = samples.real
plot(np.fft.fftshift(abs(np.fft.fft(samples))))
show()



