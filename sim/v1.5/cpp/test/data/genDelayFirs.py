#!/usr/bin/env python3

import numpy as np
import scipy.signal
from remezlp import remezlp

def generate_poly_fir_data(I, Fpass, Fstop, AdB, N=1024, Nmax=500):
    """
    Generate polyphase FIR coefficients for a given stage.

    Parameters:
    - I: Decimation factor (powers of 2)
    - Fpass: Passband edge frequency
    - Fstop: Stopband edge frequency
    - AdB: Attenuation in dB
    - N: Number of points for Remez algorithm (default: 1024)
    - Nmax: Maximum number of filter taps (default: 500)

    Returns:
    - 1D numpy array of FIR coefficients
    """
    deltaPass = 10 ** (-AdB / 20)
    deltaStop = deltaPass

    Fpass /= I
    Fstop /= I
    firCoeff = remezlp(Fpass, Fstop, deltaPass, deltaStop, nPoints=N, Nmax=Nmax)
    firCoeff /= np.max(np.abs(firCoeff))

    return firCoeff


def write_poly_fir_data_to_file(filename, fir_coefficients, I=None, AdB=None):
    """
    Write polyphase FIR coefficients to a text file.

    Parameters:
    - filename: Output file path
    - fir_coefficients: Either a dictionary of FIR coefficient arrays or a 1D numpy array
    - I: Decimation factor (only used if fir_coefficients is a 1D array)
    - AdB: Attenuation in dB (only used if fir_coefficients is a 1D array)
    """
    with open(filename, "w") as file:
        if isinstance(fir_coefficients, dict):
            for (I, AdB), coeff in fir_coefficients.items():
                file.write(f"I={I}, AdB={AdB}\n")
                for c in coeff:
                    file.write(f"{c:.16f}\n")
        elif isinstance(fir_coefficients, np.ndarray):
            if I is not None and AdB is not None:
                file.write(f"I={I}, AdB={AdB}\n")
            for c in fir_coefficients:
                file.write(f"{c:.16f}\n")

    print(f"Polyphase FIR coefficients written to {filename}")



if __name__ == "__main__":
    # Configuration parameters
    N = 1024*8
    Fmax = 0.4
    Fpass = Fmax
    Fstop = 1 - Fpass
    AdBs = [20, 30, 40, 50, 60, 70, 80]

    for AdB in AdBs:
        # Generate FIR coefficients
        delayFir_5 = generate_poly_fir_data(2, Fpass, Fstop, AdB, N=N)   # Delay for T/2
        delayFir_25 = generate_poly_fir_data(4, Fpass, Fstop, AdB, N=N)  # Delay for T/4
    
        # Write FIR coefficients to file
        output_file_5 = f"./data/input/delayFirCoefficients_5_{AdB}dB.txt"
        output_file_25 = f"./data/input/delayFirCoefficients_25_{AdB}dB.txt"
        write_poly_fir_data_to_file(output_file_5, delayFir_5)
        write_poly_fir_data_to_file(output_file_25, delayFir_25)
