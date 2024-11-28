import scipy.signal
import numpy

def remezhb( Fpass, Astop, nPoints=8192, Nmax=200):
    """
    Design half band FIR filter with given specifications.
    """
    
    if (Fpass>0.25) or (Fpass<0.0):
        # Specifications are not valid
        return numpy.array([])
    
    remez = scipy.signal.remez
    freqz = scipy.signal.freqz
    
    # Filter order initial guess
    N = int(abs(Astop)/(23*(0.5-2*Fpass))/2)
    deltaPass = 10.0**(-abs(Astop)/20.0)
    # Scipy Remez uses number of taps instead of filter order.
    # Force even number of taps
    if N%2==1:
        N += 1
            
    while N<Nmax:
        # Design the filter.
        b = scipy.signal.remez(N, [0.0, 2*Fpass], [1], [1])
        
        # Check if filter meets specifications
        w, h = freqz(b, 1, worN = nPoints, fs=1)
        H = abs(h)
        specOK = True
        # Check specifications in pass band
        if numpy.sum(((w<2*Fpass)*abs(H-1.0))>2*deltaPass)>0:
            specOK = False
        if specOK:
            tmp = numpy.zeros(2*N-1)
            tmp[::2] = b
            tmp[N-1] = 1.0
            tmp = 0.5*tmp
            return tmp
        else:
            N += 2
    return []
    

