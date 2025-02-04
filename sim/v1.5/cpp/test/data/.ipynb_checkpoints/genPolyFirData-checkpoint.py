#!/usr/bin/env python3

import numpy as np
import scipy.signal
from remezlp import remezlp

def generate_poly_fir_data(I, Fpass, Fstop, AdB, N=1024, Nmax=500):
    """
    Generate polyphase FIR coefficients for multiple stages.

    Parameters:
    - I: Decimation factor (powers of 2)
    - Fpass: Passband edge frequency
    - Fstop: Stopband edge frequency
    - AdB: Attenuation in dB
    - N: Number of points for Remez algorithm (default: 1024)
    - Nmax: Maximum number of filter taps (default: 500)

    Returns:
    - List of arrays, where each array contains FIR coefficients for a stage
    """
    deltaPass = 10 ** (-AdB / 20)
    deltaStop = deltaPass

    iterator = int(np.log(I) / np.log(2))
    Fpass_ = Fpass
    Fstop_ = Fstop
    fir_coefficients = []

    for i in range(iterator):
        Fpass_ /= 2
        Fstop_ /= 2
        firCoeff = remezlp(Fpass_, Fstop_, deltaPass, deltaStop, nPoints=N, Nmax=Nmax)
        firCoeff /= np.max(np.abs(firCoeff))
        print(len(firCoeff))
        fir_coefficients.append(firCoeff)

    return fir_coefficients


def write_poly_fir_data_to_file(filename, fir_coefficients):
    """
    Write polyphase FIR coefficients to a text file.

    Parameters:
    - filename: Output file path
    - fir_coefficients: List of FIR coefficient arrays
    """
    with open(filename, "w") as file:
        for coeff in fir_coefficients:
            # Write each FIR coefficient array as a line
            coeff_line = " ".join(f"{c:.16f}" for c in coeff)
            file.write(f"{coeff_line}\n")
    print(f"Polyphase FIR coefficients written to {filename}")


if __name__ == "__main__":
    # Configuration parameters
    N = 1024
    I = 8
    Fmax = 0.4
    Fpass = Fmax
    Fstop = 1 - Fpass
    AdBs = [20, 30, 40, 50, 60, 70, 80]

    for AdB in AdBs:
        # Generate FIR coefficients
        fir_coefficients = generate_poly_fir_data(I, Fpass, Fstop, AdB, N=N)
    
        # Write FIR coefficients to file
        output_file = f"./data/input/polyFirCofficients_{AdB}dB.txt"
        write_poly_fir_data_to_file(output_file, fir_coefficients)
