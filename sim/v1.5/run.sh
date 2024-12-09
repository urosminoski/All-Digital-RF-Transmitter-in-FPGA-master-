#!/bin/bash

# Paths
CPP_DIR="./cpp"
BIN_DIR="$CPP_DIR/bin"
BIN_FILE="$BIN_DIR/bin"
SRC_DIR="$CPP_DIR/src"
PYTHON_SCRIPT="./python/readData.py"  # Path to the Python script
FIGS_DIR="./figs"  # Directory containing PNG figures

# Default input and output files
INPUT_FILE_1="./data/sinData.txt"
OUTPUT_FILE_1="./data/sinData_deltaSigma.txt"
OUTPUT_FILE_2="./data/sinData_serialData.txt"

# Array to store LUT file paths
LUT_FILES=(./data/LUT1.json ./data/LUT2.json ./data/LUT3.json ./data/LUT4.json ./data/LUT5.json)

# Function to display usage
usage() {
    echo "Usage: $0 [compile|run|compile-run] [--open-figs] [--all-luts]"
    echo "  compile            Compile the C++ program using the Makefile."
    echo "  run                Run the compiled binary and the Python script."
    echo "  compile-run        Compile the program and then run it."
    echo "Options:"
    echo "  --open-figs        Open all PNG figures from the ./figs directory after running."
    echo "  --all-luts         Process all LUT files defined in the script."
    exit 1
}

# Compile the C++ program
compile_cpp() {
    echo "Cleaning previous build files..."
    make -C $CPP_DIR clean
    if [[ $? -ne 0 ]]; then
        echo "Cleaning failed."
        exit 1
    fi
    echo "Clean successful."

    echo "Compiling the C++ program..."
    make -C $CPP_DIR
    if [[ $? -ne 0 ]]; then
        echo "Compilation failed."
        exit 1
    fi
    echo "Compilation successful."
}

# Run the compiled binary for a single LUT
run_cpp() {
    local input_file_2="$1"
    if [[ -x $BIN_FILE ]]; then
        echo "Running the C++ binary with:"
        echo "  Input File 1: $INPUT_FILE_1"
        echo "  Output File 1: $OUTPUT_FILE_1"
        echo "  Input File 2: $input_file_2"
        echo "  Output File 2: $OUTPUT_FILE_2"
        $BIN_FILE "$INPUT_FILE_1" "$OUTPUT_FILE_1" "$input_file_2" "$OUTPUT_FILE_2"
    else
        echo "Error: Binary file $BIN_FILE does not exist or is not executable."
        exit 1
    fi

    echo "C++ program completed. Running Python script..."
    if [[ -f $PYTHON_SCRIPT ]]; then
        python3 "$PYTHON_SCRIPT"
        if [[ $? -ne 0 ]]; then
            echo "Python script execution failed."
            exit 1
        fi
    else
        echo "Error: Python script $PYTHON_SCRIPT not found."
        exit 1
    fi
}

# Run for all LUT files
run_all_luts() {
    for lut_file in "${LUT_FILES[@]}"; do
        echo "Processing LUT file: $lut_file"
        run_cpp "$lut_file"
    done
}

# Open all PNG files in the ./figs directory
open_figures() {
    if [[ -d $FIGS_DIR ]]; then
        echo "Opening all PNG figures from $FIGS_DIR..."
        for png_file in "$FIGS_DIR"/*.png; do
            if [[ -f $png_file ]]; then
                xdg-open "$png_file" &
            else
                echo "No PNG files found in $FIGS_DIR."
                break
            fi
        done
    else
        echo "Error: Figures directory $FIGS_DIR does not exist."
        exit 1
    fi
}

# Parse arguments
COMMAND=""
OPEN_FIGS=false
PROCESS_ALL_LUTS=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        compile)
            COMMAND="compile"
            shift
            ;;
        run)
            COMMAND="run"
            shift
            ;;
        compile-run)
            COMMAND="compile-run"
            shift
            ;;
        --open-figs)
            OPEN_FIGS=true
            shift
            ;;
        --all-luts)
            PROCESS_ALL_LUTS=true
            shift
            ;;
        *)
            usage
            ;;
    esac
done

# Execute commands based on arguments
case $COMMAND in
    compile)
        compile_cpp
        ;;
    run)
        if [[ $PROCESS_ALL_LUTS == true ]]; then
            run_all_luts
        else
            run_cpp "${LUT_FILES[0]}"  # Default to the first LUT file
        fi
        if [[ $OPEN_FIGS == true ]]; then
            open_figures
        fi
        ;;
    compile-run)
        compile_cpp
        if [[ $PROCESS_ALL_LUTS == true ]]; then
            run_all_luts
        else
            run_cpp "${LUT_FILES[0]}"  # Default to the first LUT file
        fi
        if [[ $OPEN_FIGS == true ]]; then
            open_figures
        fi
        ;;
    *)
        usage
        ;;
esac
