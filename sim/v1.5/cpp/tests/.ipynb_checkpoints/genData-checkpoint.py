#!/usr/bin/env python3

import numpy as np

N = 2**12
n = np.arange(N)

f = 3.2 * 1e3
fs = 10 * 1e3
F = f/fs

x_complex = 1.0 * np.exp(1j * 2*np.pi*F*n)
x_real = x_complex.real

# Write real data to a file
real_file = "./data/dataReal.txt"
with open(real_file, "w") as file:
    for value in x_real:
        file.write(f"{value}\n")  # Write each value on a new line

# Write complex data to a file
complex_file = "./data/dataComplex.txt"
with open(complex_file, "w") as file:
    for value in x_complex:
        file.write(f"{value.real} {value.imag}\n")  # Write real and imaginary parts separated by a space

