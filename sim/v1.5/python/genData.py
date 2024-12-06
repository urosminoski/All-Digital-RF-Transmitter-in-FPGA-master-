#!/usr/bin/env python3

import numpy as np

N = 8*1024
M = 3
OSR = 8
F = 0.32

# x = 2**(M-1) * np.sin(2*np.pi*np.floor(2/7 * N/OSR) * np.arange(N)/N)
x = 2**(M-1) * np.sin(2*np.pi*F*np.arange(N)/OSR)

# Write to file
with open("../data/sinData.txt", "w") as file:
    for value in x:
        file.write(f"{value}\n")

print("Signal written to sinData.txt")