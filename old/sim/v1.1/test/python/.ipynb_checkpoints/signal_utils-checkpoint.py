import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as signal
from remezlp import remezlp
import subprocess
from numdifftools.fornberg import fd_weights_all
# from math import factorial
from scipy.special import factorial

def genTestSpectrum(n, freq, amp, randPhase=False):
    # Generate test spectrum
    # Arguments:
    #    n    - number of samples
    #    freq - tuple of digital frequencies (startFreq, endFreq) in range (0, 0.5)
    #    amp  - tuple of amplitudes (ampStart, ampEnd)
    #    randPhase - if True, assign random phase to each sample
    x = np.zeros(n, dtype='complex128')
    freqStart, freqEnd = freq
    ampStart, ampEnd = amp
    indStart = round(n*freqStart)
    indEnd = round(n*freqEnd)
    x[indStart:indEnd] = ampStart - np.linspace(0, 1, num=indEnd-indStart) * (ampStart-ampEnd)
    if randPhase:
        x[indStart:indEnd] *= np.exp(1j*2*np.pi*np.random.rand(indEnd-indStart)) # randomize phase
    x[0] = abs(x[1])/2
    timeDomain = np.fft.ifft(x)
    return(timeDomain)
    
def expander(x, I):
    # Upsample the signal by factor I
    #   x - input signal
    #   I - upsampling factor
    nx = len(x)
    xI = np.zeros(nx*I, dtype=complex)
    xI[::I] = x
    return xI

def compressor(x, D):
    # Downsample the signal by factor D
    #   x - input signal
    #   D - downsampling factor
    return x[0::D]

def plotdB(x, win=False, epsilon=1e-12):
    if win==True:
        w = signal.windows.hamming(len(x), False)
        x *= w
    x += epsilon
    XdB = 20*np.log10(np.abs(np.fft.fftshift(np.fft.fft(x)) + epsilon))
    XdB -= np.max(XdB)
    freqs = np.arange(len(x))/len(x) - 0.5

    plt.plot(freqs, XdB)
    plt.grid()
    # plt.show();

def plot(x, win=False, epsilon=1e-15):
    if win==True:
        w = signal.hann(len(x), False)
        x *= w
    Xfft = np.abs(np.fft.fftshift(np.fft.fft(x)))
    Xfft /= np.max(Xfft)
    freqs = np.arange(len(x))/len(x) - 0.5
    plt.plot(freqs, Xfft)
    plt.grid()
    

def makePolyphase(coeff, M):
    # Split coefficients into polyphase components
    n = int(np.ceil(len(coeff)/M))
    tmp = np.zeros(n*M)
    tmp[:len(coeff)] = coeff
    polyCoeff = np.zeros((M,n))
    for i in range(M):
        polyCoeff[i,:] = tmp[i::M]
    return polyCoeff

def polyI(x, firCoeff, I):
    polyFIR = makePolyphase(firCoeff, I)
    xPoly = np.zeros(len(x)*I, dtype=x.dtype)
    for i in range(I):
        tmp = signal.lfilter(polyFIR[i], 1.0, np.concatenate((x, x[:int(len(firCoeff)/I-1)])))
        tmp = tmp[int(len(firCoeff)/I-1):]
        xPoly[i::I] = tmp
    return xPoly

def polyD(x, firCoeff, D):
    polyFIR = makePolyphase(firCoeff, I)
    x_ = np.concatenate((x, x[:int(len(firCoeff)-1)]))
    y = []
    for i in range(D):
        FIRin = x_[i::D]
        tmp = signal.lfilter(polyFIR[D-1-i], 1.0, FIRin)
        y = np.append(y, tmp)
    y = np.array(y)
    return y

# def fftdB(x, epsilon=1e-12):
#     return 20*np.log10(np.abs(np.fft.fftshift(np.fft.fft(x)) + epsilon))

def interpolate(x, I, F, delta, show=False, prt=False):
    N = len(x)
    Fpass, Fstop = F[0], F[1]
    deltaPass, deltaStop = delta[0], delta[1]

    Fpass_arr = []
    Fstop_arr = []
    dF = []
    N_fir = []
    firCoeffs = []

    iterator = int(np.log(I)/np.log(2))
    for i in range(iterator):
        Fstop = (1 - Fpass)/2
        Fpass /= 2
        firCoeff = remezlp(Fpass, Fstop, deltaPass, deltaStop, even_n = False, nPoints=N, Nmax=N)
        firCoeff /= np.sum(np.array(firCoeff))
        # print(len(firCoeff))
        x = expander(x, 2)
        x = signal.lfilter(firCoeff, 1.0, np.concatenate((x, x[:len(firCoeff)-1])))[len(firCoeff)-1:]
        # x = polyI(x, firCoeff, 2)

        firCoeffs.append(firCoeff)
        Fpass_arr.append(Fpass)
        Fstop_arr.append(Fstop)
        dF.append(Fstop - Fpass)
        N_fir.append(firCoeff.size)
        if show:
            plotdB(x, win = True)
            plt.plot([-0.5,0.5], [-AdB, -AdB], '--r')
            plt.ylim([-150, 10])
            plt.show();
    if prt:
        print(f"Fpass = {Fpass_arr}")
        print(f"Fstop = {Fstop_arr}")
        print(f"dF = {dF}")
        print(f"N_fir = {N_fir}")
        print(firCoeffs)

    return x

def delay(x, firCoeff, I, l):
    polyFIR = makePolyphase(firCoeff, I)
    delayFIR = polyFIR[l]
    delay = int((len(firCoeff) - 1)/I/2)
    y = signal.lfilter(delayFIR, 1.0, x)
    # y = y[delay:]
    return y, delay

def convert_1b(x, LUT, quant_type):
    y = []
    for i in range(len(x)):
        try:
            pos = int(x[i] + 8) + (0 if quant_type == "mid-tread" else -1)
            tmp = [-1 if val == 0 else 1 for val in LUT[-1 - pos]]
            y = np.concatenate((y, tmp))
        except IndexError:
            print(f"IndexError: pos={pos} is out of bounds for LUT with length {len(LUT)}")
            continue  # Skip the current iteration if an error occurs
    return np.array(y)

def convert_1b_optimized(x, LUT, quant_type):
    y = []
    offset = 0 if quant_type == "mid-tread" else -1
    for i in range(len(x)):
        pos = int(x[i] + 8) + offset
        if 0 <= pos < len(LUT):
            y.extend(LUT[-1 - pos])
        else:
            print(f"IndexError: pos={pos} is out of bounds for LUT with length {len(LUT)}")
    return np.array(y)

def paralelToSerialConverter(x, LUT):
    y = []

    for i in range(len(x)):
        pos = x[i]
        y = np.concatenate((y, LUT[-1-pos]))
    return np.array(y)

def rfiq(a, b):
    z = []
    for i in range(len(a)):
        z.append(a[i])
        z.append(b[i])
        z.append(-a[i])
        z.append(-b[i])
    return np.array(z)

def process_lut(lut):
    """
    Processes a 2D LUT (array of 1s and 0s), summing each row where 1 -> +1 and 0 -> -1.
    
    :param lut: List of lists containing 1s and 0s
    :return: List of row-wise sums
    """
    return np.array([sum(1 if elem == 1 else -1 for elem in row) for row in lut])

def fftdB(x, win=False):
    if win:
        x = x*signal.windows.hamming(len(x), False)
    xfftdB = 20*np.log10(np.abs(np.fft.fftshift(np.fft.fft(x * signal.windows.hann(len(x), False)))) + 1e-15)
    xfftdB -= np.max(xfftdB)
    angle = np.angle(np.fft.fftshift(np.fft.fft(x)))#[np.argmax(np.abs(np.fft.fftshift(np.fft.fft(x))))]
    freqs = np.arange(len(x)) / len(x) - 0.5
    return freqs, xfftdB, angle

def rfiq_filter(x, NyqZones):
    rfi = np.ones(NyqZones)
    rfi[NyqZones//4:NyqZones//2] = 0
    rfi[NyqZones//2:3*NyqZones//4] = -1
    rfi[3*NyqZones//4:] = 0
    
    rfq = np.ones(NyqZones)
    rfq[:NyqZones//4] = 0
    rfq[NyqZones//2:3*NyqZones//4] = 0
    rfq[3*NyqZones//4:] = -1
    
    x_upsampled = np.zeros((len(x)-1)*NyqZones+1, dtype = complex) # Size is (N-1)*M+1, bcs after convolution ((N-1)*M+1) + (M-1) = N*M as it should be!
    x_upsampled[::NyqZones] = x
    
    x_rfi = np.convolve(rfi, x_upsampled.real)
    x_rfq = np.convolve(rfq, x_upsampled.imag)
    x_rfiq = x_rfi + x_rfq
    return x_rfiq

def frac_delay_1st(x, delta, kernel):
    """
    Single-term Taylor fractional delay: y = f + δ·f', where f' is conv(x, kernel).
    kernel must sum to zero and approximate the derivative.
    """
    f0 = x
    f1 = np.convolve(x, kernel, mode='same')
    return f0 + delta*f1

def float_to_q1n(signal, n_frac):
    # signal /= np.max(n_frac)
    """Convert float list to Q1.n_frac fixed-point signed integers"""
    scale = 1 << n_frac  # 2^n_frac
    max_val = (1 << (n_frac)) - 1  # maximum positive integer value
    min_val = -1 << n_frac         # minimum negative value

    result = []
    for x in signal:
        val = int(x * scale)
        val = min(max(val, min_val), max_val)  # saturate to range
        result.append(val)
    return result


def saturate_qmn(value: int, m_int: int, n_frac: int) -> int:
    """
    Saturate signed fixed-point integer in Qm.n format.

    Args:
        value   : int - raw integer value
        m_int   : int - number of integer bits (includes sign bit)
        n_frac  : int - number of fractional bits

    Returns:
        Saturated integer that fits in Qm.n range
    """
    total_bits = m_int + n_frac
    min_val = -(1 << (m_int - 1)) * (1 << n_frac)         # -2^(m-1) * 2^n = raw int min
    max_val = ((1 << (m_int - 1))) * (1 << n_frac) - 1 # (2^(m-1) - 2^-n) as int
    # print(min_val, max_val)

    return max(min(value, max_val), min_val)

def fir_direct_q1n(x_q, h_q, n_frac, saturate=True):
    """
    Fixed-point Direct FIR filter in Q1.n_frac format (1 integer bit + n_frac fractional).
    
    Parameters:
        x       : input samples (float or int)
        h       : coefficients (float or int)
        n_frac  : number of fractional bits
    Returns:
        y_q1n   : output list of Q1.n_frac integers (saturated)
    """
    N = len(h_q)

    y_q = np.zeros(len(x_q))
    for n in range(len(x_q)):
        acc = 0  # Accumulator in Q2.(2*n_frac)
        for i in range(N):
            if n - i >= 0:
                product = h_q[i] * x_q[n - i]  # Q1.n × Q1.n = Q2.2n
                acc = saturate_qmn(acc+product, m_int=2, n_frac=2*n_frac)
                # acc += product

        # Shift from Q2.2n → Q1.n
        if saturate:
            acc >>= n_frac+1

        # Saturate back to Q1.n range
        # y = saturate_q1n(acc, n_frac)
        y_q[n] = acc

    return y_q

def fir_transposed_q1n(x_q, h_q, n_frac, saturate=True):
    """
    Fixed-point Transposed FIR filter in Q1.n_frac format (1 integer bit + n_frac fractional).
    
    Parameters:
        x       : input samples (float or int)
        h       : coefficients (float or int)
        n_frac  : number of fractional bits
    Returns:
        y_q1n   : output list of Q1.n_frac integers (saturated)
    """
    N = len(h_q)
    # delay_line[i] will hold s_{i+1}[n-1], for i=0..N-2
    delay_line = np.zeros(N-1, dtype="int")
    y_q = np.zeros(len(x_q), dtype="int")

    for n in range(len(x_q)):
        # 1) accumulate tap-0 + the oldest partial sum (delay_line[0])
        acc = saturate_qmn(h_q[0] * x_q[n] + delay_line[0], m_int=2, n_frac=2*n_frac)

        # 2) shift through the rest of the delay line:
        #    new s1 = h[1]*xn + old s2
        #    new s2 = h[2]*xn + old s3
        #    … 
        for i in range(N - 2):
            delay_line[i] = saturate_qmn(h_q[i+1] * x_q[n] + delay_line[i+1], m_int=2, n_frac=2*n_frac)

        # 3) last stage has no following partial sum:
        delay_line[N-2] = h_q[N-1] * x_q[n]

        # 4) bring back from Q2.2n to Q1.n_frac
        if saturate:
            acc >>= n_frac+1

        # 5) saturate into Q1.n_frac range
        y_q[n] = acc #saturate_q1n(acc, n_frac)

    return y_q

def poly_decimation_q1n(x_q, h_q, D, n_frac):
    x_q = np.asarray(x_q)
    pad = (-len(x_q)) % D
    if pad > 0:
        x_q = np.concatenate([x_q, np.zeros(pad, dtype=x_q.dtype)])
        
    poly_h_q = makePolyphase(h_q, D)  # shape: (D, n)

    Nout = int(np.ceil(len(x_q) / D))
    acc = np.zeros(Nout, dtype=x_q.dtype)

    n = poly_h_q.shape[1]  # taps per phase

    for i in range(D):
        # grab every D-th sample starting at offset i
        FIR_in = x_q[i::D]
        # FIR_in = x_q[i::D]
        # filter with the reversed polyphase order
        tmp = fir_transposed_q1n(FIR_in, poly_h_q[D-1-i], n_frac=2*n_frac, saturate=False)

        # clip tmp to avoid overshooting acc length
        # acc = saturate_qmn(acc + tmp, m_int = 2, n_frac=2*n_frac)
        acc += tmp
        # for k in range(len(acc)):
        #     acc[k] = saturate_qmn(acc[k] + tmp[k], m_int=2, n_frac=2*n_frac)

    # shift & saturate
    # for k in range(Nout):
    #     v = acc[k] >> n_frac+1
    #     acc[k] = v

    return acc

def poly_interpolation_q1n(x_q, h_q, I, n_frac):
    x_q = np.asarray(x_q)
    poly_h_q = makePolyphase(h_q, I)
    xout = np.zeros(len(x_q)*I, dtype=x_q.dtype)
    for i in range(I):
        FIR_in = x_q
        tmp = fir_transposed_q1n(FIR_in, poly_h_q[i], n_frac=2*n_frac, saturate=False)
        xout[i::I] = tmp

    return xout

LUT1 = np.array([
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 8
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 7
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1],  # Level 6
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1],  # Level 5
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1],  # Level 4
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0],  # Level 3
    [0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1],  # Level 2
    [0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1],  # Level 1
    [0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1],  # Level 0
    [0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1],  # Level -1
    [0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1],  # Level -2
    [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],  # Level -3
    [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],  # Level -4
    [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],  # Level -5
    [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],  # Level -6
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],  # Level -7
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]   # Level -8
])

LUT2 = np.array([
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 8
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0],  # Level 7
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0],  # Level 6
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0],  # Level 5
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0],  # Level 4
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0],  # Level 3
    [0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0],  # Level 2
    [0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0],  # Level 1
    [0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0],  # Level 0
    [1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1],  # Level -1
    [1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],  # Level -2
    [1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],  # Level -3
    [1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],  # Level -4
    [1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],  # Level -5
    [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],  # Level -6
    [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],  # Level -7
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]   # Level -8
])

LUT3 = np.array([
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 8
    [1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1],  # Level 7
    [1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1],  # Level 6
    [1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1],  # Level 5
    [1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1],  # Level 4
    [1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1],  # Level 3
    [1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1],  # Level 2
    [1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1],  # Level 1
    [0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0],  # Level 0
    [0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0],  # Level -1
    [0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0],  # Level -2
    [0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0],  # Level -3   
    [0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0],  # Level -4
    [0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0],  # Level -5
    [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0],  # Level -6
    [0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0],  # Level -7
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]   # Level -8    
])

# LUT3 = np.array([
#     [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 8
#     [1, 1, 1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1],  # Level 7
#     [1, 1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, 1, 1],  # Level 6
#     [1, 1, 1, -1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, -1, 1, 1, 1],  # Level 5
#     [1, -1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1, 1, 1, -1, 1, 1, 1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1, 1],  # Level 4
#     [1, 1, 1, 1, -1, -1, -1, -1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1, 1, 1, -1, 1, 1, 1, -1, -1, -1, -1, 1, 1, 1, 1],  # Level 3
#     [1, -1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, -1, 1],  # Level 2
#     [1, 1, 1, -1, -1, -1, -1, 1, -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, -1, -1, 1, -1, 1, -1, -1, -1, -1, 1, 1, 1],  # Level 1
#     [-1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1, -1],  # Level 0
#     [-1, -1, -1, 1, 1, 1, 1, -1, 1, -1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, -1, 1, -1, 1, 1, 1, 1, -1, -1, -1],  # Level -1
#     [-1, 1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, -1, 1, -1, 1, -1],  # Level -2
#     [-1, -1, -1, -1, 1, 1, 1, 1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, 1, 1, 1, -1, -1, -1, -1],  # Level -3   
#     [-1, 1, -1, -1, -1, -1, -1, 1, 1, -1, -1, -1, -1, -1, 1, -1, -1, 1, -1, -1, -1, -1, -1, 1, 1, -1, -1, -1, -1, -1, 1, -1],  # Level -4
#     [-1, -1, -1, 1, -1, -1, -1, -1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, -1, -1, -1, -1, 1, -1, -1, -1],  # Level -5
#     [-1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1],  # Level -6
#     [-1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1],  # Level -7
#     [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]   # Level -8    
# ])

LUT4 = np.array([
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 7.5
    [1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1],  # Level 6.5
    [1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1],  # Level 5.5
    [1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1],  # Level 4.5
    [1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1],  # Level 3.5
    [1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1],  # Level 2.5
    [1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1],  # Level 1.5
    [1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1],  # Level 0.5
    [0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0],  # Level -0.5
    [0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0],  # Level -1.5
    [0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0],  # Level -2.5
    [0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0],  # Level -3.5
    [0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0],  # Level -4.5
    [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0],  # Level -5.5
    [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0],  # Level -6.5
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]   # Level -7.5
])

LUT5 = np.array([
    [1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1],  # Level 7.5
    [1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1],  # Level 6.5
    [1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1],  # Level 5.5
    [1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1],  # Level 4.5
    [1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1],  # Level 3.5
    [1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1],  # Level 2.5
    [1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1],  # Level 1.5
    [0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0],  # Level 0.5
    [1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1],  # Level -0.5
    [0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0],  # Level -1.5
    [0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0],  # Level -2.5
    [0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0],  # Level -3.5
    [0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0],  # Level -4.5
    [0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0],  # Level -5.5
    [0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0],  # Level -6.5
    [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0]   # Level -7.5 
])

LUT1_signed = np.array([
    [ 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1],
    [-1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1],
    [-1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1],
    [-1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1],
    [-1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1,  1],
    [-1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1,  1, -1],
    [-1,  1, -1,  1, -1,  1,  1,  1,  1,  1, -1,  1, -1,  1, -1,  1],
    [-1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1,  1, -1,  1],
    [-1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1],
    [-1,  1, -1,  1, -1,  1, -1, -1, -1,  1, -1,  1, -1,  1, -1,  1],
    [-1,  1, -1,  1, -1, -1, -1, -1, -1,  1, -1,  1, -1,  1, -1,  1],
    [-1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1, -1,  1],
    [-1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1],
    [-1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1],
    [-1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1],
    [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1],
    [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]
])

LUT2_signed = np.array([
    [ 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1],
    [-1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1],
    [-1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1],
    [-1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1],
    [-1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1,  1,  1, -1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1],
    [-1,  1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1,  1, -1,  1,  1, -1,  1, -1,  1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1],
    [-1,  1, -1,  1, -1,  1,  1,  1,  1,  1, -1,  1, -1,  1, -1,  1,  1, -1,  1, -1,  1, -1,  1,  1,  1,  1,  1, -1,  1, -1,  1, -1],
    [-1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1,  1, -1,  1,  1, -1,  1, -1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1],
    [-1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1, -1],
    [ 1, -1,  1, -1,  1, -1, -1, -1,  1, -1,  1, -1,  1, -1,  1, -1, -1,  1, -1,  1, -1,  1, -1,  1, -1, -1, -1,  1, -1,  1, -1,  1],
    [ 1, -1,  1, -1,  1, -1, -1, -1, -1, -1,  1, -1,  1, -1,  1, -1, -1,  1, -1,  1, -1,  1, -1, -1, -1, -1, -1,  1, -1,  1, -1,  1],
    [ 1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1, -1,  1, -1, -1,  1, -1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1],
    [ 1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1, -1, -1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1],
    [ 1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1],
    [ 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1],
    [ 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1],
    [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]
])

LUT3_signed = np.array([
    [ 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1],
    [ 1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1],
    [ 1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1],
    [ 1,  1,  1, -1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1, -1,  1,  1,  1],
    [ 1, -1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1, -1,  1,  1, -1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1, -1,  1],
    [ 1,  1,  1,  1, -1, -1, -1, -1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1, -1, -1, -1, -1,  1,  1,  1,  1],
    [ 1, -1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1,  1, -1, -1,  1,  1, -1,  1, -1,  1,  1,  1,  1,  1, -1,  1, -1,  1, -1],
    [ 1,  1,  1, -1, -1, -1, -1,  1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1,  1, -1,  1, -1, -1, -1, -1,  1,  1,  1],
    [-1,  1,  1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1,  1,  1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1,  1,  1, -1],
    [-1, -1, -1,  1,  1,  1,  1, -1,  1, -1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1,  1, -1,  1,  1,  1,  1, -1, -1, -1],
    [-1,  1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1,  1, -1],
    [-1, -1, -1, -1,  1,  1,  1,  1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1,  1,  1,  1,  1, -1, -1, -1, -1],
    [-1,  1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1,  1, -1, -1,  1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1,  1, -1],
    [-1, -1, -1,  1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1,  1, -1, -1, -1],
    [-1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1],
    [-1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1],
    [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]
])

LUT4_signed = np.array([
    [ 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1],
    [ 1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1],
    [ 1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1],
    [ 1,  1, -1,  1,  1,  1,  1, -1,  1,  1,  1,  1, -1,  1,  1],
    [ 1, -1,  1,  1,  1,  1, -1,  1, -1,  1,  1,  1,  1, -1,  1],
    [ 1, -1,  1,  1, -1,  1,  1, -1,  1,  1, -1,  1,  1, -1,  1],
    [ 1, -1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1,  1],
    [ 1, -1, -1,  1,  1, -1,  1, -1,  1, -1,  1,  1, -1, -1,  1],
    [-1,  1,  1, -1, -1,  1, -1,  1, -1,  1, -1, -1,  1,  1, -1],
    [-1,  1, -1,  1, -1,  1, -1, -1, -1,  1, -1,  1, -1,  1, -1],
    [-1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1],
    [-1,  1, -1, -1, -1, -1,  1, -1,  1, -1, -1, -1, -1,  1, -1],
    [-1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1,  1, -1, -1],
    [-1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1],
    [-1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1],
    [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]
])

LUT5_signed = np.array([
    [ 1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1],
    [ 1,  1, -1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1, -1,  1,  1],
    [ 1,  1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1],
    [ 1,  1, -1,  1, -1,  1,  1,  1,  1, -1,  1,  1,  1,  1, -1,  1, -1,  1,  1],
    [ 1, -1,  1,  1, -1,  1,  1,  1, -1,  1, -1,  1,  1,  1, -1,  1,  1, -1,  1],
    [ 1, -1,  1,  1, -1, -1,  1,  1,  1, -1,  1,  1,  1, -1, -1,  1,  1, -1,  1],
    [ 1, -1,  1, -1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1,  1, -1,  1],
    [-1,  1, -1,  1,  1,  1,  1, -1, -1, -1, -1, -1,  1,  1,  1,  1, -1,  1, -1],
    [ 1, -1,  1, -1, -1, -1, -1,  1,  1,  1,  1,  1, -1, -1, -1, -1,  1, -1,  1],
    [-1,  1, -1,  1, -1,  1, -1,  1, -1, -1, -1,  1, -1,  1, -1,  1, -1,  1, -1],
    [-1,  1, -1, -1,  1,  1, -1, -1, -1,  1, -1, -1, -1,  1,  1, -1, -1,  1, -1],
    [-1,  1, -1, -1,  1, -1, -1, -1,  1, -1,  1, -1, -1, -1,  1, -1, -1,  1, -1],
    [-1, -1,  1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1,  1, -1,  1, -1, -1],
    [-1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1],
    [-1, -1,  1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1,  1, -1, -1],
    [-1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1]
])

LUTs = [LUT1, LUT2, LUT3, LUT4, LUT5]
# LUTs = [LUT1_signed, LUT2_signed, LUT3_signed, LUT4_signed, LUT5_signed]
