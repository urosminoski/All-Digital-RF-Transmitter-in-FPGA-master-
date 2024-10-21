import matplotlib as mpl
mpl.rc('text', usetex = True)
mpl.rc('font', family = 'serif', size = 18)

import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as signal
from fxpmath import Fxp
from matplotlib.ticker import MaxNLocator, FuncFormatter

LUT1 = [
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 8
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 7
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1],  # Level 6
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1],  # Level 5
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1],  # Level 4
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0],  # Level 3
    [0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1],  # Level 2
    [0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0],  # Level 1
    [0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1],  # Level 0
    [0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0],  # Level -1
    [0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0],  # Level -2
    [0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0],  # Level -3
    [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0],  # Level -4
    [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0],  # Level -5
    [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0],  # Level -6
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],  # Level -7
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],  # Level -8
]

LUT2 = [
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 8
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0],  # Level 7
    [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0],  # Level 6
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0],  # Level 5
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0],  # Level 4
    [0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0],  # Level 3
    [0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0],  # Level 2
    [0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0],  # Level 1
    [0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0],  # Level 0
    [1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1],  # Level -1
    [1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],  # Level -2
    [1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],  # Level -3
    [1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],  # Level -4
    [1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],  # Level -5
    [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],  # Level -6
    [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],  # Level -7
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]   # Level -8
]

LUT3 = [
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 8
    [1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1],  # Level 7
    [1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1],  # Level 6
    [1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1],  # Level 5
    [1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1],  # Level 4
    [1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1],  # Level 3
    [1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1],  # Level 2
    [1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1],  # Level 1
    [0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0],  # Level 0
    [0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0],  # Level -1
    [0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0],  # Level -2
    [0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0],  # Level -3
    [0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0],  # Level -4
    [0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0],  # Level -5
    [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0],  # Level -6
    [0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0],  # Level -7
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]   # Level -8    
]

LUT4 = [
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # Level 7.5
    [1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1],  # Level 6.5
    [1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1],  # Level 5.5
    [1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1],  # Level 4.5
    [1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1],  # Level 3.5
    [1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1],  # Level 2.5
    [1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1],  # Level 1.5
    [1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1],  # Level 0.5
    [0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0],  # Level -0.5
    [0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0],  # Level -1.5
    [0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0],  # Level -2.5
    [0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0],  # Level -3.5
    [0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0],  # Level -4.5
    [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0],  # Level -5.5
    [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0],  # Level -6.5
    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],  # Level -7.5
]

LUT5 = [
    [1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1],  # Level 7.5
    [1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1],  # Level 6.5
    [1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1],  # Level 5.5
    [1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1],  # Level 4.5
    [1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1],  # Level 3.5
    [1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1],  # Level 2.5
    [1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1],  # Level 1.5
    [0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0],  # Level 0.5
    [1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1],  # Level -0.5
    [0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0],  # Level -1.5
    [0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0],  # Level -2.5
    [0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0],  # Level -3.5
    [0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0],  # Level -4.5
    [0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0],  # Level -5.5
    [0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0],  # Level -6.5
    [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0]   # Level -7.5    
]


def plot_fft_dB(signals_x=None, signals_y=None, title1=None, title2=None, legend1=None, legend2=None, 
                xlim1=(0, 0.5), ylim1=None, xlim2=(0, 0.5), ylim2=None):
    """
    Plots the FFT of multiple signals (signals_x and signals_y) in dB scale, normalizes them, and adds 2 dB to the max value.
    
    Args:
    - signals_x: List of signals or a single signal for the first subplot (each signal is a list or array). Can be None.
    - signals_y: List of signals or a single signal for the second subplot (each signal is a list or array). Can be None.
    - title1: Title for the first subplot (optional, default is 'FFT of Signal 1').
    - title2: Title for the second subplot (optional, default is 'FFT of Signal 2').
    - legend1: List of legends for the first subplot (optional, default is None).
    - legend2: List of legends for the second subplot (optional, default is None).
    - xlim1: Tuple specifying the x-axis limits for the first subplot (optional, default is (0, 0.5)).
    - ylim1: Tuple specifying the y-axis limits for the first subplot (optional, auto-calculated).
    - xlim2: Tuple specifying the x-axis limits for the second subplot (optional, default is (0, 0.5)).
    - ylim2: Tuple specifying the y-axis limits for the second subplot (optional, auto-calculated).
    
    Returns:
    - None
    """

    # If signals_x or signals_y is None, handle them gracefully
    plots_to_draw = 0
    if signals_x is not None:
        if not isinstance(signals_x, list):
            signals_x = [signals_x]
        plots_to_draw += 1

    if signals_y is not None:
        if not isinstance(signals_y, list):
            signals_y = [signals_y]
        plots_to_draw += 1

    if plots_to_draw == 0:
        raise ValueError("At least one of signals_x or signals_y must be provided.")

    N = len(signals_x[0]) if signals_x else len(signals_y[0])
    freqs = np.linspace(0, 0.5, N//2)
    fontsize = 8

    fig, axs = plt.subplots(plots_to_draw, 1, figsize=(12, 12))

    # If there's only one plot, axs will not be a list, so we handle it
    if plots_to_draw == 1:
        axs = [axs]

    subplot_idx = 0
    epsilon = 1e-12  # Small value to avoid log of zero

    # Plot for the first subplot (x signals)
    if signals_x is not None:
        if title1 is None:
            title1 = 'FFT of Signal 1'

        max_value_x = float('-inf')
        min_value_x = float('inf')
        for i, x in enumerate(signals_x):
            x_fft = np.fft.fft(x)
            x_dB = 20 * np.log10(np.abs(x_fft[:N//2]) + epsilon)  # Adding epsilon to avoid log of zero
            x_dB -= np.max(x_dB)
            max_value_x = max(max_value_x, np.max(x_dB))
            min_value_x = min(min_value_x, np.min(x_dB))
            axs[subplot_idx].plot(freqs, x_dB, label=legend1[i] if legend1 else f"Signal {i+1}")

        axs[subplot_idx].set_title(title1)
        axs[subplot_idx].set_ylabel('Magnitude [dB]')
        axs[subplot_idx].grid(True)

        # Apply custom y-limits for the first subplot, if provided
        if ylim1 is not None:
            axs[subplot_idx].set_ylim(ylim1)
        else:
            axs[subplot_idx].set_ylim(min_value_x - 2, max_value_x + 5)

        # Apply custom x-limits for the first subplot, if provided
        if xlim1 is not None:
            axs[subplot_idx].set_xlim(xlim1)
        else:
            axs[subplot_idx].set_xlim(0, 0.5)

        def format_func(value, tick_number):
            return f'{value:.1f}'

        axs[subplot_idx].yaxis.set_major_formatter(FuncFormatter(format_func))

        if legend1:
            axs[subplot_idx].legend(fontsize=fontsize)

        subplot_idx += 1

    # Plot for the second subplot (y signals)
    if signals_y is not None:
        if title2 is None:
            title2 = 'FFT of Signal 2'

        max_value_y = float('-inf')
        min_value_y = float('inf')
        for i, y in enumerate(signals_y):
            y_fft = np.fft.fft(y)
            y_dB = 20 * np.log10(np.abs(y_fft[:N//2]) + epsilon)  # Adding epsilon to avoid log of zero
            y_dB -= np.max(y_dB)
            max_value_y = max(max_value_y, np.max(y_dB))
            min_value_y = min(min_value_y, np.min(y_dB))
            axs[subplot_idx].plot(freqs, y_dB, label=legend2[i] if legend2 else f"Signal {i+1}")

        axs[subplot_idx].set_title(title2)
        axs[subplot_idx].set_xlabel('Normalized Frequency')
        axs[subplot_idx].set_ylabel('Magnitude [dB]')
        axs[subplot_idx].grid(True)

        # Apply custom y-limits for the second subplot, if provided
        if ylim2 is not None:
            axs[subplot_idx].set_ylim(ylim2)
        else:
            axs[subplot_idx].set_ylim(min_value_y - 2, max_value_y + 5)

        # Apply custom x-limits for the second subplot, if provided
        if xlim2 is not None:
            axs[subplot_idx].set_xlim(xlim2)
        else:
            axs[subplot_idx].set_xlim(0, 0.5)

        axs[subplot_idx].yaxis.set_major_formatter(FuncFormatter(format_func))

        if legend2:
            axs[subplot_idx].legend(fontsize=fontsize)

    plt.tight_layout()
    plt.show()


def plot_IIR_and_zeros_poles(data_pz, labels=None, fig_title=None):
    """
    Plots the frequency response (magnitude and phase) on the left 
    and the poles and zeros on the unit circle on the right, centered vertically.
    
    Args:
    - data_pz: List of tuples (b, a) where 'b' is the numerator coefficients and 'a' is the denominator coefficients.
    - labels: List of labels for each plot. Default is None.
    - fig_title: Title of the figure (optional).
    """
    from matplotlib.gridspec import GridSpec

    fontsize = 8

    # Create a figure with size 12x6 (width x height in inches)
    fig = plt.figure(figsize=(12, 6))

    # Set the figure title if provided
    if fig_title is not None:
        fig.suptitle(fig_title, fontsize=24, y=1.01)

    # Create a grid layout with 2 rows and 3 columns
    gs = GridSpec(2, 3, width_ratios=[2, 2, 3], height_ratios=[1, 1])

    # Create subplots for magnitude, phase, and poles/zeros plot
    ax_mag = fig.add_subplot(gs[0, 0:2])  # Magnitude response in the top left
    ax_phase = fig.add_subplot(gs[1, 0:2])  # Phase response in the bottom left
    ax_pz = fig.add_subplot(gs[:, 2])  # Poles and zeros plot on the right (spanning both rows)

    # Set the aspect ratio of the poles/zeros plot to be equal, so the unit circle appears circular
    ax_pz.set_aspect('equal', 'box')

    # Iterate through each (b, a) pair to compute and plot the frequency response and poles/zeros
    for i, (b, a) in enumerate(data_pz):
        # Compute the frequency response (w is the frequency, h is the frequency response)
        w, h = signal.freqz(b, a)

        # Label for the current plot (if provided)
        label = labels[i] if labels is not None else f"Filter {i+1}"

        # Plot the magnitude response (converted to dB)
        h_dB = 20 * np.log10(abs(h))
        h_dB -= np.max(h_dB)  # Normalize to the maximum value
        ax_mag.plot(w / np.pi / 2, h_dB, label=label)

        # Plot the phase response (angle in radians)
        ax_phase.plot(w / np.pi / 2, np.angle(h), label=label)

    # Set titles and labels for the magnitude and phase plots
    ax_mag.set_title("Magnitude Response")
    ax_phase.set_title("Phase Response")
    ax_pz.set_title("Poles and Zeros")

    # Set x and y labels for the magnitude and phase plots
    ax_mag.set_xlabel(r'$F$')
    ax_mag.set_ylabel(r'Magnitude $[dB]$')
    ax_mag.grid(True)  # Enable grid lines
    ax_mag.legend(fontsize=fontsize)  # Add legend for labels

    ax_phase.set_xlabel(r'$F$')
    ax_phase.set_ylabel('Phase [radians]')
    ax_phase.grid(True)  # Enable grid lines
    ax_phase.legend(fontsize=fontsize)  # Add legend for labels

    # Plot the unit circle on the poles/zeros plot (for reference)
    unit_circle = plt.Circle((0, 0), 1, color='gray', fill=False, linestyle='--')
    ax_pz.add_artist(unit_circle)

    # Set color map for distinct colors for each filter
    cmap = plt.colormaps.get_cmap('tab10')
    colors = [cmap(i) for i in range(len(data_pz))]

    # Plot the poles and zeros for each filter
    for i, (b, a) in enumerate(data_pz):
        label = labels[i] if labels is not None else f"Filter {i+1}"
        zeros = np.roots(b)  # Compute the zeros of the filter
        poles = np.roots(a)  # Compute the poles of the filter

        color = colors[i]  # Assign a color to the current filter

        # Plot zeros as circles and poles as x's on the complex plane
        ax_pz.scatter(np.real(zeros), np.imag(zeros), s=50, marker='o', color=color, label=f"{label} Zeros")
        ax_pz.scatter(np.real(poles), np.imag(poles), s=50, marker='x', color=color, label=f"{label} Poles")

    # Draw horizontal and vertical lines through the origin
    ax_pz.axvline(0, color='gray', lw=1)
    ax_pz.axhline(0, color='gray', lw=1)
    ax_pz.set_xlim([-1.5, 1.5])  # Set x-axis limits
    ax_pz.set_ylim([-1.5, 1.5])  # Set y-axis limits
    ax_pz.set_xlabel("Real")
    ax_pz.set_ylabel("Imaginary")
    ax_pz.grid(True)  # Enable grid lines
    ax_pz.legend(fontsize=fontsize)  # Add legend for labels

    # Adjust layout to prevent overlapping and set the space for the figure title
    plt.tight_layout()
    plt.subplots_adjust(top=0.88)  # Adjust the top to make space for the figure title
    plt.show()

def decompose_iir_to_parallel(b, a):
    """
    Decomposes an IIR filter into parallel sections using partial fraction expansion.

    Args:
    - b: Numerator coefficients of the IIR filter.
    - a: Denominator coefficients of the IIR filter.

    Returns:
    - b_new: List of numerator coefficients for each parallel section.
    - a_new: List of denominator coefficients for each parallel section.
    """
    # Perform partial fraction expansion to get residues (r), poles (p), and direct terms (k)
    r, p, k = signal.residuez(b, a)

    # Track which poles have been processed (initially none)
    processed = np.zeros(len(p), dtype=bool)

    # Lists to store the new numerator and denominator coefficients
    b_new = []
    a_new = []

    # Loop through each pole to create first-order or second-order sections
    for i in range(len(p)):
        if processed[i]:
            continue  # Skip if the pole has already been processed

        if p[i].imag <= 1e-10:
            # If the pole is real, create a first-order section
            b = [r[i].real]
            a = [1, -p[i].real]
        else:
            # For complex conjugate poles, create a second-order section
            for j in range(i + 1, len(p)):
                if p[i].real == p[j].real and p[i].imag == -p[j].imag:
                    # Complex conjugate found, form the second-order section
                    b = [2 * r[i].real, -2 * (r[i].real * p[i].real + r[i].imag * p[i].imag)]
                    a = [1, -2 * p[i].real, (p[i].real ** 2 + p[i].imag ** 2)]
                    processed[j] = True  # Mark the conjugate as processed
                    break

        # Append the new sections
        b_new.append(b)
        a_new.append(a)

        # Mark the pole as processed
        processed[i] = True

    return [b_new, a_new]

def decompose_iir_to_parallel_H(b, a):
    """
    Decomposes an IIR filter into parallel sections and computes the total frequency response.

    Args:
    - b: Numerator coefficients of the IIR filter.
    - a: Denominator coefficients of the IIR filter.

    Returns:
    - w: Frequency values.
    - h: Combined frequency response of all parallel sections.
    """
    # Decompose the filter into parallel sections
    [b_new, a_new] = decompose_iir_to_parallel(b, a)

    h = 0  # Initialize the total frequency response

    # Loop through each section and sum the frequency responses
    for b, a in zip(b_new, a_new):
        w_tmp, h_tmp = signal.freqz(b, a)
        h += h_tmp  # Add the current section's frequency response to the total

    return [w, h]

def combine_parallel_sections(b_sections, a_sections):
    """
    Combines multiple parallel filter sections into a single equivalent filter section.

    Args:
    - b_sections: List of `b` coefficients (numerators) for each parallel section.
    - a_sections: List of `a` coefficients (denominators) for each parallel section.

    Returns:
    - b_total: The combined `b` coefficients (numerator) of the equivalent filter.
    - a_total: The combined `a` coefficients (denominator) of the equivalent filter.
    """
    # Initialize the total denominator as the product of all denominators
    a_total = a_sections[0]
    for a in a_sections[1:]:
        a_total = np.polymul(a_total, a)

    # Initialize the total numerator (array of zeros with length of a_total - 1)
    b_total = np.zeros(len(a_total) - 1)

    # Loop through each section and sum the adjusted numerators
    for i, (b_i, a_i) in enumerate(zip(b_sections, a_sections)):
        # Multiply the numerator by the product of all denominators except the current one
        a_others = np.ones(1)
        for j, a_j in enumerate(a_sections):
            if i != j:
                a_others = np.polymul(a_others, a_j)

        # Adjust the numerator and add it to the total numerator
        b_adjusted = np.polymul(b_i, a_others)
        b_total = np.polyadd(b_total, b_adjusted)

    return b_total, a_total


def iir_direct_fxp(b, a, x, n_int, n_frac, precision='half'):
    """
    Implements an IIR filter using a direct form with fixed-point arithmetic.
    
    Args:
    - x: Input signal (list or array).
    - b: Numerator coefficients of the IIR filter.
    - a: Denominator coefficients of the IIR filter.
    - n_int: Number of integer bits in the fixed-point representation.
    - n_frac: Number of fractional bits in the fixed-point representation.
    - precision: 'half' (reduce to fixed-point) or 'full' (use full double-width precision).
    
    Returns:
    - y: Output signal after filtering, in fixed-point precision.
    """
    
    # Define fixed-point format for the input/output (single precision)
    fxp_type = {
        'signed': True, 
        'n_word': n_int + n_frac, 
        'n_frac': n_frac, 
        'overflow': 'saturate', 
        'rounding': 'trunc'
    }
    
    # Define fixed-point format for intermediate calculations (double precision)
    fxp_double_type = {
        'signed': True, 
        'n_word': 2 * (n_int + n_frac), 
        'n_frac': 2 * n_frac, 
        'overflow': 'saturate', 
        'rounding': 'trunc'
    }

    # Convert the numerator (b) and denominator (a) coefficients to fixed-point format
    b_fxp = [Fxp(val, **fxp_type) for val in b]
    a_fxp = [Fxp(val, **fxp_type) for val in a]

    # Initialize the internal state (y[n-1] or previous output) as 0 in double precision
    yd = Fxp(0, **fxp_double_type)
    
    # Initialize the output array (same length as the input)
    y = np.zeros(len(x))

    # Loop through each input sample
    for n in range(len(x)):
        # Convert the input signal to fixed-point
        x_fxp = Fxp(x[n], **fxp_type)

        # Perform filtering using double-width precision for intermediate calculations
        x_fxp_double = Fxp(x_fxp.get_val(), **fxp_double_type)  # Convert input to double precision
        y_tmp = b_fxp[0] * x_fxp_double - a_fxp[1] * yd  # Apply filter equation

        # Convert the result back to the original precision (half or full)
        if precision == 'half':
            y[n] = Fxp(y_tmp.get_val(), **fxp_type).get_val()  # Convert to single precision and store
        elif precision == 'full':
            y[n] = y_tmp.get_val()  # Keep double precision result
        
        # Update the internal state for the next sample
        yd = Fxp(y_tmp.get_val(), **fxp_double_type)
        
    return y
    
def iir_transposed_fxp(b, a, x, n_int, n_frac, precision='half'):
    """
    Implements an IIR filter using a transposed form with fixed-point arithmetic.
    
    Args:
    - x: Input signal (list or array).
    - b: Numerator coefficients of the IIR filter.
    - a: Denominator coefficients of the IIR filter.
    - n_int: Number of integer bits in the fixed-point representation.
    - n_frac: Number of fractional bits in the fixed-point representation.
    - precision: 'half' (reduce to fixed-point) or 'full' (use full double-width precision).
    
    Returns:
    - y: Output signal after filtering, in fixed-point precision.
    """
    
    # Define fixed-point format for the input/output (single precision)
    fxp_type = {
        'signed': True, 
        'n_word': n_int + n_frac, 
        'n_frac': n_frac, 
        'overflow': 'saturate', 
        'rounding': 'trunc'
    }
    
    # Define fixed-point format for intermediate calculations (double precision)
    fxp_double_type = {
        'signed': True, 
        'n_word': 2 * (n_int + n_frac), 
        'n_frac': 2 * n_frac, 
        'overflow': 'saturate', 
        'rounding': 'trunc'
    }

    # Convert the numerator (b) and denominator (a) coefficients to fixed-point format
    b_fxp = [Fxp(val, **fxp_type) for val in b]
    a_fxp = [Fxp(val, **fxp_type) for val in a]

    # Initialize the internal states (w, wd, wdd) to store previous results (all 0 in double precision)
    w = Fxp(0, **fxp_double_type)
    wd = Fxp(0, **fxp_double_type)
    wdd = Fxp(0, **fxp_double_type)
    
    # Initialize the output array (same length as the input)
    y = np.zeros(len(x))

    # Loop through each input sample
    for n in range(len(x)):
        # Convert the input signal to fixed-point
        x_fxp = Fxp(x[n], **fxp_type)

        # Perform filtering using double-width precision for intermediate calculations
        x_fxp_double = Fxp(x_fxp.get_val(), **fxp_double_type)  # Convert input to double precision
        w = x_fxp_double - a_fxp[1] * wd - a_fxp[2] * wdd  # Apply filter equation (transposed form)
        y_tmp = b_fxp[0] * w + b_fxp[1] * wd  # Compute the output using the transposed form

        # Convert the result back to the original precision (half or full)
        if precision == 'half':
            y[n] = Fxp(y_tmp.get_val(), **fxp_type).get_val()  # Convert to single precision and store
        elif precision == 'full':
            y[n] = y_tmp.get_val()  # Keep double precision result
        
        # Update the internal states for the next sample
        wdd = Fxp(wd.get_val(), **fxp_double_type)
        wd = Fxp(w.get_val(), **fxp_double_type)
        
    return y

def parallel_lfilter_fxp(b, a, x, n_int, n_frac):
    """
    Applies multiple IIR filters (parallel sections) to the input signal using fixed-point arithmetic.
    
    Args:
    - b: List of numerator coefficients for each filter section (can be 1st or 2nd order sections).
    - a: List of denominator coefficients for each filter section (must be either 2 or 3 coefficients).
    - x: Input signal (list or array).
    - n_int: Number of integer bits in the fixed-point representation.
    - n_frac: Number of fractional bits in the fixed-point representation.
    
    Returns:
    - y: Output signal after applying all the parallel filter sections.
    
    Example of calling:
    x = np.random.randn(100)  # Input signal
    b = [[3.1953125], [0.42386913756811406, -2.782525031028174], [-4.606777844852687, 0.023283687418655274]]
    a = [[1, -0.3466037644977315], [1, -0.6659138380831608, 0.1626025786310818], [1, -0.623802437419108, 0.45098689983844903]]
    n_int = 4  # Number of integer bits
    n_frac = 8  # Number of fractional bits
    y = parallel_lfilter_fxp(b, a, x, n_int, n_frac)  # Apply the parallel filter sections to x
    """

    # Initialize output signal
    y = np.zeros(len(x))
    
    # Number of filter sections
    num_sections = len(a)

    # Iterate over each filter section and apply the appropriate filter
    for i in range(len(a)):
        if len(a[i]) == 2:
            y += iir_direct_fxp(b[i], a[i], x, n_int, n_frac)
        elif len(a[i]) == 3:
            y += iir_transposed_fxp(b[i], a[i], x, n_int, n_frac)
        else:
            raise ValueError("Only pass first or second order sections!")
        # Print progress (same line)
        print(f"{i + 1} of {num_sections} sections filtered", end='\r')
        
    return y

def deltaSigma(x, n_word=5, n_frac=0, type="mid-rise"):
    # Coefficients of H0.
    b00 = Fxp(7.3765809, n_word=12, n_frac=8, overflow='saturate', rounding='around')
    a01 = Fxp(0.3466036, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    # Coefficients of H1.
    b10 = Fxp(0.424071040, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    b11 = Fxp(2.782608716, n_word=12, n_frac=9, overflow='saturate', rounding='around')
    a11 = Fxp(0.66591402, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    a12 = Fxp(0.16260264, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    # Coefficients of H2.
    b20 = Fxp(4.606822182, n_word=12, n_frac=8, overflow='saturate', rounding='around')
    b21 = Fxp(0.023331537, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    a21 = Fxp(0.62380242, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    a22 = Fxp(0.4509869, n_word=12, n_frac=11, overflow='saturate', rounding='around')
    
    y_iir = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    e = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    y_i = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    x0 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    x0d = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    x1 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w1 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w1d = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w1dd = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    x2 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w2 = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w2d = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    w2dd = Fxp(0, n_word=24, n_frac=19, overflow='saturate', rounding='around')
    v = Fxp(0, n_word=n_word, n_frac=n_frac, overflow='saturate', rounding='around')
    
    y = np.zeros(len(x))
    
    for i in range(len(x)):
        y_i( (x[i]+y_iir)() )
        v( y_i() )

        # if type=="mid-rise":
        #     if abs(v()) % 2 == 0:
        #         if v() > 0:
        #             v( (v+1)() )
        #         elif v() < 0:
        #             v( (v-1)() )
        y[i] = v()
        e( (y_i-v)() )
    
        x0( (b00*e+a01*x0d)() )
        w1( (e+a11*w1d-a12*w1dd)() )
        x1( (b10*w1-b11*w1d)() )
        w2( (e+a21*w2d-a22*w2dd)() )
        x2( (b21*w2d-b20*w2)() )
        y_iir( (x0+x1+x2)() )
    
        x0d( x0() )
        w1dd( w1d() )
        w1d( w1() )
        w2dd( w2d() )
        w2d( w2() )
    return y

def convert_1b(x, LUT):
    y = []
    for i in range(len(x)):
        pos = x[i] + 8
        y = np.concatenate((y, LUT[-1-pos]))
    return np.array(y)
    




    