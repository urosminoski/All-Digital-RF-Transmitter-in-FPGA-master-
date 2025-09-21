#!/bin/bash

# Source configuration file to load the variables
source config.cfg

# Extract the dB value from the delay FIR coefficient file name.
# This assumes the file name contains an underscore followed by digits and "dB"
delay_value=$(echo "$DELAY_FIR_COEFF_5" | sed -n 's/.*_\([0-9]\+\)dB.*/\1/p')

# Extract the dB value from the poly FIR coefficient file name.
poly_value=$(echo "$POLY_FIR_COEFF" | sed -n 's/.*_\([0-9]\+\)dB.*/\1/p')

# Extract the number from the LUT file name (e.g., "LUT3.txt" gives "3")
lut_value=$(echo "$LUT" | sed -n 's/.*LUT\([0-9]\+\).*/\1/p')

# Construct the output file name using the extracted values.
SIGNAL_OUTPUT="./data/output/signal_rfiq_${delay_value}_${poly_value}_${lut_value}.txt"

echo "Generated output file name: $SIGNAL_OUTPUT"

# Now you can compile and run your program with the proper arguments:
make
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

./build/fxpdsp_test "$SIGNAL_INPUT" "$SIGNAL_OUTPUT" "$DELAY_FIR_COEFF_5" "$DELAY_FIR_COEFF_25" "$POLY_FIR_COEFF" "$DELTA_SIGMA_IIR" "$LUT"