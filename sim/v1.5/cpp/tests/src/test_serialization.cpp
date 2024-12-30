#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <complex>
#include "funcs.hpp"

std::string modifyFileName(const std::string&, const std::string&, const std::string&); 

int main(int argc, char* argv[]) {
    if (argc != 3) {
        // Expecting two parameters, file names
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_deltaSigma_file> <input_LUT_file> <output_serial_file>" << std::endl;
        return 1;
    }

    std::string inputDataFileName = argv[1];
    std::string inputLutFileName = argv[2];

    // Read input file
    std::variant<std::vector<double>, std::vector<std::complex<double>>> inputData;
    std::map<std::string, double> metadata;
    if (!readFromFile(inputDataFileName, inputData, metadata)) {
        std::cerr << "Error: Reading form file " << inputDataFileName << " was unsuccessful!" << std::endl;
        return 1;
    }

    // Read LUT file
    std::vector<std::vector<int>> LUT;
    if (!readLUT(inputLutFileName, LUT)) {
        std::cerr << "Error: Reading form file " << inputLutFileName << " was unsuccessful!" << std::endl;
        return 1;
    }
    // Check if LUT is not empty and update metadata with LUT size
    if (!LUT.empty()) {
        metadata["LUTwidth"] = static_cast<double>(LUT[0].size());
        // std::cout << "LUT size added to metadata: " << metadata["LUTwidth"] << std::endl;
    } else {
        std::cerr << "Error: LUT is empty. Cannot add LUT size to metadata." << std::endl;
    }

    // Perform parallel-to-serial convertion processing
    std::variant<std::vector<double>, std::vector<std::complex<double>>> serialData;    
    parallelToSerialConverterWrapper(inputData, LUT, serialData);

    // Write processed data into file
    std::string outputFileName_deltaSigma = modifyFileName(inputDataFileName, "_serialized", inputLutFileName);
    if (!writeToFile(outputFileName_deltaSigma, serialData, metadata)) {
        std::cerr << "Error: Writing to file " << outputFileName_deltaSigma << " was unsuccessful!" << std::endl;
        return 1;
    }

    std::cout << "Processing completed successfully!" << std::endl;
    return 0;
}

std::string modifyFileName(const std::string& fileName, const std::string& suffix, const std::string& lutFileName = "") {
    const std::string inputDir = "./data/input/";
    const std::string outputDir = "./data/output/";

    // Extract the LUT identifier from lutFileName if provided
    std::string lutIdentifier;
    if (!lutFileName.empty()) {
        size_t lutPos = lutFileName.find("LUT");
        if (lutPos != std::string::npos) {
            size_t endPos = lutFileName.find_first_not_of("0123456789", lutPos + 3);
            lutIdentifier = lutFileName.substr(lutPos, endPos - lutPos);
        }
    }

    // Replace input directory with output directory
    size_t pos = fileName.find(inputDir);
    std::string newFileName = fileName;
    if (pos != std::string::npos) {
        newFileName.replace(pos, inputDir.size(), outputDir);
    }

    // Find the position of the last dot in the file name
    size_t dotPos = newFileName.find_last_of('.');

    // Build the new file name
    if (dotPos != std::string::npos) {
        return newFileName.substr(0, dotPos) + suffix +
               (lutIdentifier.empty() ? "" : "_" + lutIdentifier) +
               newFileName.substr(dotPos);
    }

    // If there's no dot, append the suffix and LUT identifier
    return newFileName + suffix + (lutIdentifier.empty() ? "" : "_" + lutIdentifier);
}