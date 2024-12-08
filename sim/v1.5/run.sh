#!/bin/bash

# Paths
CPP_DIR="./cpp"
BIN_DIR="$CPP_DIR/bin"
BIN_FILE="$BIN_DIR/bin"
SRC_DIR="$CPP_DIR/src"

# Default input and output files
INPUT_FILE="./data/sinData.txt"
OUTPUT_FILE="./data/sinData_deltaSigma.txt"

# Function to display usage
usage() {
    echo "Usage: $0 [compile|run]"
    echo "  compile   Compile the C++ program using the Makefile."
    echo "  run       Run the compiled binary (checks if it exists and is executable)."
    exit 1
}

# Compile the C++ program
compile_cpp() {
    echo "Compiling the C++ program..."
    make -C $CPP_DIR
    if [[ $? -ne 0 ]]; then
        echo "Compilation failed."
        exit 1
    fi
    echo "Compilation successful."
}

# Run the compiled binarybu
run_cpp() {
    if [[ -x $BIN_FILE ]]; then
        echo "Running the C++ binary with:"
        echo "  Input File: $INPUT_FILE"
        echo "  Output File: $OUTPUT_FILE"
        $BIN_FILE "$INPUT_FILE" "$OUTPUT_FILE"
    else
        echo "Error: Binary file $BIN_FILE does not exist or is not executable."
        exit 1
    fi
}

# Main script logic
if [[ $# -ne 1 ]]; then
    usage
fi

case $1 in
    compile)
        compile_cpp
        ;;
    run)
        run_cpp
        ;;
    *)
        usage
        ;;
esac
