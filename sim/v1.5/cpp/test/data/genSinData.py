#!/usr/bin/env python3

import numpy as np

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
    x = 2**(M-1) * np.exp(1j * 2 * np.pi * F * np.arange(N) / OSR)
    # x+= 2**(M-1) * np.exp(1j * 2 * np.pi * 0.1 * np.arange(N) / OSR)
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


if __name__ == "__main__":
    # Signal parameters
    N = 2**12
    M = 3
    OSR = 8
    f = 2.5 * 1e3
    fs = 10 * 1e3

    # Generate and write signal
    signal = generate_signal(N, M, OSR, f, fs)
    write_signal_to_file("./data/input/sinDataComplex_OSR8.txt", signal, OSR, fs)
    write_signal_to_file("./data/input/sinData_OSR8.txt", signal.real, OSR, fs)
