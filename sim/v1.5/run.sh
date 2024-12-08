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
INPUT_FILE_2="./data/LUT3.json"
OUTPUT_FILE_1="./data/sinData_deltaSigma.txt"
OUTPUT_FILE_2="./data/sinData_serialData.txt"

# Function to display usage
usage() {
    echo "Usage: $0 [compile|run|compile-run] [--open-figs]"
    echo "  compile            Compile the C++ program using the Makefile."
    echo "  run                Run the compiled binary and the Python script."
    echo "  compile-run        Compile the program and then run it."
    echo "Options:"
    echo "  --open-figs        Open all PNG figures from the ./figs directory after running."
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

# Run the compiled binary
run_cpp() {
    if [[ -x $BIN_FILE ]]; then
        echo "Running the C++ binary with:"
        echo "  Input File 1: $INPUT_FILE_1"
        echo "  Output File 1: $OUTPUT_FILE_1"
        echo "  Input File 2: $INPUT_FILE_2"
        echo "  Output File 2: $OUTPUT_FILE_2"
        $BIN_FILE "$INPUT_FILE_1" "$OUTPUT_FILE_1" "$INPUT_FILE_2" "$OUTPUT_FILE_2"
    else
        echo "Error: Binary file $BIN_FILE does not exist or is not executable."
        exit 1
    fi

    echo "C++ program completed. Running Python script..."
    if [[ -f $PYTHON_SCRIPT ]]; then
        python3 "$PYTHON_SCRIPT" "$OUTPUT_FILE"
        if [[ $? -ne 0 ]]; then
            echo "Python script execution failed."
            exit 1
        fi
    else
        echo "Error: Python script $PYTHON_SCRIPT not found."
        exit 1
    fi
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
        run_cpp
        if [[ $OPEN_FIGS == true ]]; then
            open_figures
        fi
        ;;
    compile-run)
        compile_cpp
        run_cpp
        if [[ $OPEN_FIGS == true ]]; then
            open_figures
        fi
        ;;
    *)
        usage
        ;;
esac

# Bash completion function
_run_sh_completion() {
    local cur prev options
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    options="compile run compile-run --open-figs"

    COMPREPLY=( $(compgen -W "${options}" -- "$cur") )
}

# Register the completion function
complete -F _run_sh_completion ./run.sh
