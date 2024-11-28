
from plotSettings import *
import numpy as np
from scipy.signal import freqz
from remezhb import *

Fpass = 0.2
AdB = 60.0

h = remezhb( Fpass, AdB)
print(h)

w, H = freqz(h, 1, worN = 1024, fs=1)

plot(w, 20.0*np.log10(abs(H)), 'k', linewidth=1.5, aa=True)
xlabel(r'$F$')
ylabel(r'$|H(e^{\jj 2 \pi F}|^2~\mr{[dB]}$')

ylim(-80.0,5.0)
xlim(0.0, 0.5)

tight_layout()
exportGraphics('hb_filter')
show()


