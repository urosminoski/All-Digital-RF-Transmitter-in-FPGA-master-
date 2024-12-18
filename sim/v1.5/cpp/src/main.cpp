#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <complex>
#include "funcs.hpp"

std::string extractNumberFromFileName(const std::string& fileName);
std::string modifyFileName(const std::string& fileName, const std::string& suffix);
void logWarning(const std::string& message);
bool processFiles(const std::string& inputFileName_sinData,
                  const std::string& outputFileName_deltaSigma,
                  const std::string& inputFileName_LUTdata,
                  std::string& outputFileName_serialData);

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_deltaSigma_file> <input_LUT_file> <output_serial_file>" << std::endl;
        return 1;
    }

    std::string inputFileName_sinData = argv[1];
    std::string outputFileName_deltaSigma = argv[2];
    std::string inputFileName_LUTdata = argv[3];
    std::string outputFileName_serialData = argv[4];

    if (!processFiles(inputFileName_sinData, outputFileName_deltaSigma, inputFileName_LUTdata, outputFileName_serialData)) {
        return 1; // Exit with failure
    }

    std::cout << "Processing completed successfully!" << std::endl;
    return 0;
}

// Helper function to extract a number from a file name
std::string extractNumberFromFileName(const std::string& fileName) {
    std::string number;
    for (char c : fileName) {
        if (std::isdigit(c)) {
            number += c;
        }
    }
    return number;
}

// Helper function to modify a file name
std::string modifyFileName(const std::string& fileName, const std::string& suffix) {
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos) {
        return fileName.substr(0, dotPos) + suffix + fileName.substr(dotPos);
    }
    return fileName + suffix;
}

// Helper function to log warnings
void logWarning(const std::string& message) {
    std::cerr << "Warning: " << message << std::endl;
}

// Main processing function
bool processFiles(const std::string& inputFileName_sinData,
                  const std::string& outputFileName_deltaSigma,
                  const std::string& inputFileName_LUTdata,
                  std::string& outputFileName_serialData) {
    std::vector<std::complex<double>> x;
    std::vector<std::complex<int>> y;
    std::map<std::string, double> metadata;

    // Step 1: Read data from input file
    if (!readFromFile(inputFileName_sinData, x, metadata)) {
        logWarning("Failed to read sinData file.");
        return false;
    }

    // Step 2: Perform delta-sigma modulation
    deltaSigmaComplex(x, y);

    // Step 3: Write delta-sigma output to file
    if (!writeToFile(outputFileName_deltaSigma, y, metadata)) {
        logWarning("Failed to write deltaSigma file.");
        return false;
    }

    // Step 4: Read LUT data
    std::vector<std::vector<int>> LUT;
    if (!readLUT(inputFileName_LUTdata, LUT)) {
        logWarning("Failed to read LUT file.");
        return false;
    }

    // Step 5: Extract number from LUT file name and add metadata
    size_t lastSlashPos = inputFileName_LUTdata.find_last_of("/\\");
    std::string fileName = inputFileName_LUTdata.substr(lastSlashPos + 1);
    std::string lutName = fileName.substr(0, fileName.find_last_of('.'));
    std::string number = extractNumberFromFileName(lutName);

    if (!number.empty()) {
        metadata["lut_name"] = std::stod(number);
        // outputFileName_serialData = modifyFileName(outputFileName_serialData, "_LUT" + number);
    } else {
        logWarning("No numeric part found in LUT name. File name will not be modified.");
    }

    // Step 6: Add LUT size to metadata
    if (!LUT.empty()) {
        metadata["lut_size"] = static_cast<double>(LUT[0].size());
    } else {
        logWarning("LUT is empty. Cannot add lut_size to metadata.");
    }

    // Step 7: Perform parallel-to-serial conversion
    std::vector<std::complex<int>> y_serial;
    parallelToSerialConverterComplex(y, LUT, y_serial);

    // Step 8: Write serial data to file
    if (!writeToFile(outputFileName_serialData, y_serial, metadata)) {
        logWarning("Failed to write serial data file.");
        return false;
    }

    return true;
}
