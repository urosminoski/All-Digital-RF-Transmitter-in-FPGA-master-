#!/bin/bash

# Directories for input and output data files
INPUT_DIR="./data/input"
LUTS_DIR="./data/luts"
FIR_DIR="./data/firCoeff"
OUTPUT_DIR="./data/output"

# Paths for scripts and compiled binaries
MAKE_DIR="./cpp"
EXECUTABLE="./cpp/bin/bin"
GEN_DATA_SCRIPT="./python/genData.py"
READ_DATA_SCRIPT="./python/readData.py"

# Make sure output directory exists
mkdir -p "$OUTPUT_DIR"

# Function to generate input data using the Python script
generate_input_data() {
    read -rp "Do you want to generate input data? (y/n): " gen_choice
    if [[ "$gen_choice" == "y" || "$gen_choice" == "Y" ]]; then
        python3 "$GEN_DATA_SCRIPT"
        if [[ $? -ne 0 ]]; then
            echo "Input data generation failed. Exiting."
            exit 1
        fi
        echo "Input data generation complete!"
    fi
}

# Compile the C++ program using the Makefile
echo "Compiling the C++ program..."
make -C "$MAKE_DIR"
if [[ $? -ne 0 ]]; then
    echo "Compilation failed. Exiting."
    exit 1
fi
echo "Compilation successful!"

# Function to process a single pair of input file and LUT file
process_single() {
    local inputFile="$1"
    local lutFile="$2"

    # Run the C++ executable
    echo "Processing $inputFile with $lutFile"
    "$EXECUTABLE" "$inputFile" "$lutFile"
}

# Generate input data before starting
generate_input_data

# Menu to choose mode
echo "Choose an option:"
echo "1) Process all combinations of input and LUT files"
echo "2) Process a specific pair of files"
read -rp "Enter your choice (1 or 2): " choice

if [[ "$choice" == "1" ]]; then
    # Process all combinations of input files and LUT files
    for inputFile in "$INPUT_DIR"/*.txt; do
        for lutFile in "$LUTS_DIR"/*.json; do
            process_single "$inputFile" "$lutFile"
        done
    done
    # echo "Processing complete for all combinations!"

elif [[ "$choice" == "2" ]]; then
    # Show all available input files
    echo "Available input files:"
    ls "$INPUT_DIR"/*.txt | nl
    read -rp "Select an input file by number: " input_choice
    inputFile=$(ls "$INPUT_DIR"/*.txt | sed -n "${input_choice}p")

    # Show all available LUT files
    echo "Available LUT files:"
    ls "$LUTS_DIR"/*.json | nl
    read -rp "Select an LUT file by number: " lut_choice
    lutFile=$(ls "$LUTS_DIR"/*.json | sed -n "${lut_choice}p")

    # Validate selections
    if [[ -z "$inputFile" || -z "$lutFile" ]]; then
        echo "Invalid selection. Exiting."
        exit 1
    fi

    # Process the selected pair
    process_single "$inputFile" "$lutFile"
    # echo "Processing complete for selected pair!"
else
    echo "Invalid choice. Exiting."
    exit 1
fi

read_output_data() {
    read -rp "Do you want to read output data? (y/n): " read_choice
    if [[ "$read_choice" == "y" || "$read_choice" == "Y" ]]; then
        echo "Do you want to read:"
        echo "1) All files in $OUTPUT_DIR"
        echo "2) Specific files"
        read -rp "Enter your choice (1 or 2): " read_option

        if [[ "$read_option" == "1" ]]; then
            python3 "$READ_DATA_SCRIPT" "$OUTPUT_DIR"/*
        elif [[ "$read_option" == "2" ]]; then
            echo "Available output files:"
            ls "$OUTPUT_DIR" | nl
            read -rp "Select files by number (e.g., 1 3 5): " file_choices

            # Convert numbers to file paths
            files_to_read=$(echo "$file_choices" | tr ' ' '\n' | awk -v dir="$OUTPUT_DIR" 'NR==FNR{a[NR]=$0;next} {print dir "/" a[$1]}' <(ls "$OUTPUT_DIR") -)

            # Validate if any files were selected
            if [[ -z "$files_to_read" ]]; then
                echo "No valid files selected. Skipping reading output data."
            else
                python3 "$READ_DATA_SCRIPT" $files_to_read
            fi
        else
            echo "Invalid choice. Skipping reading output data."
        fi
    fi
}

# Ask if the user wants to read the output data
read_output_data
