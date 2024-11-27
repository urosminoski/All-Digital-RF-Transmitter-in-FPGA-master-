
from plotSettings import *
from mpl_toolkits.axes_grid1.inset_locator import inset_axes
import numpy as np
from scipy.signal import freqz
from remezlp import *

Fpass = 0.2
Fstop = 0.3
AdB = 60.0
Amax = 20.0*np.log10(1.0+10.0**(-AdB/20.0))
Amin = 20.0*np.log10(1.0-10.0**(-AdB/20.0))

deltaPass = 10.0**(-AdB/20.0)
deltaStop = deltaPass

#h = remezlp( Fpass, Fstop, deltaPass, deltaStop, forceOrder='even')
h = remezlp( Fpass, Fstop, deltaPass, deltaStop)
print(int(AdB/(23.0*(Fstop-Fpass))),len(h))
print(h)

w, H = freqz(h, 1, worN = 1024, fs=1)

plot(w, 20.0*np.log10(abs(H)), 'k', linewidth=1.5, aa=True)
xlabel(r'$F$')
ylabel(r'$|H(e^{\jj 2 \pi F}|^2~\mr{[dB]}$')


ylim(-80.0,5.0)
xlim(0.0, 0.5)

# Plot inset
ax = gca()
axins = inset_axes(ax, width="35%",height="15%", bbox_to_anchor=(0.35, -198, 0.4, 200),
                    bbox_transform=ax.transData, loc=2, borderpad=0)


axins.plot(w, 20*np.log10(abs(H)), color='black', linewidth=1.5, aa=True )


axins.set_xlim(0, Fpass+0.01) # apply the x-limits
axins.set_ylim(Amin, Amax) # apply the y-limits



tight_layout()
exportGraphics('hb_filter_remezlp')
show()


