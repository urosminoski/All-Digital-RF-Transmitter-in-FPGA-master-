import scipy.signal
import numpy

def remezlp( Fpass, Fstop, deltaPass, deltaStop, forceOrder='none', nPoints=8192, Nmax=200):
    """
    Design low pass FIR filter with given specifications.
    Parameter forceOrder can be used to force even or odd filter order
    """
    
    if (Fpass>Fstop) or (deltaPass<0) or (deltaStop<0):
        # Specifications are not valid
        return numpy.array([])
    
    remez = scipy.signal.remez
    freqz = scipy.signal.freqz
    
    # Filter order initial guess
    N = int(-20*numpy.log10(deltaStop)/(23*(Fstop-Fpass)))
    # Scipy Remez uses number of taps instead of filter order.
    if forceOrder == 'even':
        if N%2==0:
            N += 1
    if forceOrder == 'odd':
        if N%2==1:
            N += 1
            
    while N<Nmax:
        # Design the filter.
        b = scipy.signal.remez(N, [0.0, Fpass, Fstop, 0.5], [1,0], [1, deltaPass/deltaStop])
        
        # Check if filter meets specifications
        w, h = freqz(b, 1, worN = nPoints, fs=1)
        H = abs(h)
        specOK = True
        # Check specifications in pass band
        if numpy.sum(((w<Fpass)*abs(H-1.0))>deltaPass)>0:
            specOK = False
        if numpy.sum(((w>Fstop)*H)>deltaStop)>0:
            specOK = False
    
        if specOK:
            return b
        else:
            if (forceOrder == 'even') or (forceOrder == 'odd'):
                N += 2
            else:
                N += 1
    return []
    

