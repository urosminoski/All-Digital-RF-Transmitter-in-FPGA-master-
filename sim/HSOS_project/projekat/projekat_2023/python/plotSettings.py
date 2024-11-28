import numpy as np
import os
from matplotlib.pyplot import *
from scipy.signal import *


######################################
rc('text', usetex=True)
rc('text.latex', preamble=r"""
\usepackage{amsmath}
\usepackage[Symbolsmallscale]{upgreek}
\let\omega\upomega
\let\pi\uppi
\let\alpha\upalpha
\let\beta\upbeta
\let\theta\uptheta
\let\sigma\upsigma
\let\varsigma\upvarsigma
\let\epsilon\upepsilon
\let\mu\upmu
\let\delta\updelta
\let\phi\upphi
\newcommand{\mr}[1]{\mathrm{#1}}
\newcommand{\jj}{\mathrm{j}}
\newcommand*{\tr}{^\mr{T}}
\newcommand*{\dd}{\mr{d}}
""")

fig_width_pt = 550  # Get this from LaTeX using \showthe\columnwidth
inches_per_pt = 1.0/72.27               # Convert pt to inch
golden_mean = (np.sqrt(5)-1.0)/2.0         # Aesthetic ratio
fig_width = fig_width_pt*inches_per_pt  # width in inches
fig_height = fig_width*golden_mean      # height in inches
fig_size =  [fig_width,fig_height]
params = {'backend': 'ps',
           'axes.labelsize': 14,
           'font.size': 14,
           'legend.fontsize': 14,
           'xtick.labelsize': 14,
           'ytick.labelsize': 14,
           'text.usetex': True,
           'figure.figsize': fig_size}

rcParams.update(params)

def exportGraphics(fileName, transparent=False):
    savefig(fileName+".eps", transparent=transparent)
    cmd = "epstopdf "+fileName+".eps"
    os.system(cmd)
    cmd = "rm "+fileName+".eps"
    os.system(cmd)
    cmd = "pdfcrop --margins 5 "+fileName+".pdf "+fileName+".pdf"
    os.system(cmd)

######################################

