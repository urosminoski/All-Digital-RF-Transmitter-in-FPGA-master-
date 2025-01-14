#!/usr/bin/env python3

import json
import numpy as np

def replace_zeros_with_minus_one(lut):
    """
    Replace all occurrences of 0 with -1 in a LUT.

    Args:
        lut (list of lists): The input LUT.
    
    Returns:
        list of lists: LUT with 0 replaced by -1.
    """
    return [[-1 if value == 0 else value for value in row] for row in lut]

def generate_signal(N, M, OSR, f, fs):
    """
    Generate a sinusoidal signal.

    Parameters:
    - N: Number of samples
    - M: Amplitude factor
    - OSR: Oversampling ratio
    - f: Frequency of the signal (Hz)
    - fs: Sampling frequency (Hz)

    Returns:
    - A numpy array containing the generated signal
    """
    F = f / fs
    # x = 2**(M-1) * np.sin(2 * np.pi * F * np.arange(N) / OSR)
    x = 2**(M-1) * np.exp(1j * 2 * np.pi * F * np.arange(N) / OSR)
    return x


def write_signal_to_file(filename, signal, OSR, fs):
    """
    Write a signal to a file with metadata.

    Parameters:
    - filename: Path to the output file
    - signal: The signal to write (real or complex values)
    - OSR: Oversampling ratio
    - fs: Sampling frequency (Hz)
    """
    with open(filename, "w") as file:
        # Write metadata
        file.write(f"# OSR={OSR}\n")
        file.write(f"# fs={fs}\n")

         # Check if the signal contains complex values
        is_complex = np.iscomplexobj(signal)
        file.write(f"# complex={1 if is_complex else 0}\n")
        
        # Write signal data
        if is_complex:
            # Write complex signal values
            for value in signal:
                file.write(f"{float(value.real)} {float(value.imag)}\n")
        else:
            # Write real signal values
            for value in signal:
                file.write(f"{float(value)}\n")

    print(f"Signal with metadata written to {filename}")

def save_luts_to_json(luts, filenames):
    """
    Save multiple LUTs to JSON files.

    Parameters:
    - luts: List of LUTs to save
    - filenames: Corresponding list of file names
    """
    for lut, filename in zip(luts, filenames):
        with open(filename, 'w') as f:
            # lut = replace_zeros_with_minus_one(lut)
            json.dump(lut, f)
        print(f"LUT written to {filename}")


def main():
    # Signal parameters
    N = 8 * 1024 + 111
    M = 0
    OSR = 1
    f = 4 * 1e3
    fs = 10 * 1e3

    # Generate and write signal
    signal = generate_signal(N, M, OSR, f, fs)
    write_signal_to_file("./data/input/sinDataComplex.txt", signal, OSR, fs)
    write_signal_to_file("./data/input/sinData.txt", signal.real, OSR, fs)


if __name__ == "__main__":
    main()
