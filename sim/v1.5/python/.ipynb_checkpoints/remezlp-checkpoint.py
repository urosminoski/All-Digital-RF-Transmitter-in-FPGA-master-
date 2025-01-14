def remezlp(Fpass, Fstop, deltaPass, deltaStop, forceOrder='none', nPoints=8192, Nmax=200):
    """
    Design an optimized low-pass FIR filter using the Remez exchange algorithm.
    
    Parameters:
    -----------
    Fpass : float
        Normalized passband edge frequency (0 < Fpass < 0.5).
    Fstop : float
        Normalized stopband edge frequency (Fstop > Fpass).
    deltaPass : float
        Allowed ripple in the passband.
    deltaStop : float
        Allowed ripple in the stopband.
    forceOrder : str, optional
        Force filter order to be 'even' or 'odd'. Default is 'none'.
    nPoints : int, optional
        Number of frequency points for validation. Default is 8192.
    Nmax : int, optional
        Maximum number of filter taps. Default is 200.

    Returns:
    --------
    np.ndarray
        FIR filter coefficients. Returns an empty array if the design fails.
    """
    if Fpass >= Fstop or deltaPass <= 0 or deltaStop <= 0:
        raise ValueError("Invalid filter specifications: Fpass < Fstop, deltaPass > 0, deltaStop > 0 required.")

    # Initial guess for the number of taps
    N = int(-20 * np.log10(deltaStop) / (23 * (Fstop - Fpass)))

    # Adjust the order based on forceOrder
    if forceOrder == 'even' and N % 2 == 0:
        N += 1
    elif forceOrder == 'odd' and N % 2 == 1:
        N += 1

    # Iteratively design the filter
    while N <= Nmax:
        # Design filter using remez
        try:
            b = remez(
                numtaps=N,
                bands=[0.0, Fpass, Fstop, 0.5],
                desired=[1, 0],
                weight=[1, deltaPass / deltaStop],
                fs=1.0  # Normalized frequency
            )
        except ValueError as e:
            print(f"Error in remez design: {e}")
            return np.array([])

        # Validate the filter using frequency response
        w, h = freqz(b, worN=nPoints, fs=1.0)
        H = np.abs(h)

        passband_ok = np.all(H[w <= Fpass] >= (1 - deltaPass))
        stopband_ok = np.all(H[w >= Fstop] <= deltaStop)

        if passband_ok and stopband_ok:
            return b  # Filter meets specifications
        else:
            N += 2 if forceOrder in ['even', 'odd'] else 1

    return np.array([])  # Return empty array if design fails