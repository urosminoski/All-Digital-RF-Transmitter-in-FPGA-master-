import numpy as np

N = 8*1024
M = 3
OSR = 8
n = np.arange(N)

x = 2**(M-1) * np.sin(2*np.pi*np.floor(2/7 * N/OSR) * n/N)

# Write the data to input_data.txt
file_path = "./input_data.txt"
with open(file_path, "w") as f:
    for value in x:
        f.write(f"{value}\n")

file_path