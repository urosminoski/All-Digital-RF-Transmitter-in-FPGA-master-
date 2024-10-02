import numpy as np

def float_to_fixed(x, n_int, n_frac):

    # Scaling and roundig of input x to int and n_int+n_frac bits
    x_fixed = np.round(x * 2**n_frac)

    # Saturate if overflow
    max_val = 2**(n_int+n_frac)-1
    min_val = -(max_val+1)

    if x_fixed > max_val:
        x_fixed = max_val
    elif x_fixed < min_val:
        x_fixed = min_val

    return x_fixed

def fixed_to_float(x, n_frac):
    return x/2**n_frac

def mul_fixed(x_fixed, y_fixed, n_int, n_frac):
    x = fixed_to_float(x_fixed, n_frac)
    y = fixed_to_float(y_fixed, n_frac)
    result = x*y
    return float_to_fixed(result, 2*n_int, 2*n_frac)
    
    