#!/usr/bin/env python3

import numpy as np

def writeReal(fileName, data):
    with open(fileName, "w") as file:
        for value in data:
            file.write(f"{float(value)}\n")  # Write each value on a new line

def writeComplex(fileName, data):
    with open(fileName, "w") as file:
        for value in data:
            file.write(f"{float(value.real)} {float(value.imag)}\n")  # Write real and imaginary parts separated by a space

N = 2**12
n = np.arange(N)

f = 3.2 * 1e3
fs = 10 * 1e3
F = f/fs

x_complex = 1.0 * np.exp(1j * 2*np.pi*F*n)
x_real = x_complex.real

real_file = "./data/input/dataReal.txt"
complex_file = "./data/input/dataComplex.txt"

writeReal(real_file, x_real)
writeComplex(complex_file, x_complex)


