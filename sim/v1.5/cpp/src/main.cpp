#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "funcs.hpp"

int main(int argc, char* argv[])
{
    // Check if file name is passed as argument
    if (argc != 5)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::string inputFileName_sinData = argv[1];
    std::string outputFileName_deltaSigma = argv[2];
    std::string inputFileName_LUTdata = argv[3];
    std::string outputFileName_serialData = argv[4];

    std::vector<std::complex<double>> x;
    std::vector<std::complex<int>> y;
    std::map<std::string, double> metadata;

    // Read data form file
    if (!readFromFile(inputFileName_sinData, x, metadata))
    {
        return 1;   // Exit if reading fails
    }

    // Perform delta-sigma modulation
    deltaSigmaComplex(x, y);

    // Write to the file
    if (!writeToFile(outputFileName_deltaSigma, y, metadata))
    {
        return 1; // Exit if writing fails
    }

    // Read LUT
    std::vector<std::vector<int>> LUT;
    readLUT(inputFileName_LUTdata, LUT);

    // Extract the file name (after the last slash)
    size_t lastSlashPos = inputFileName_LUTdata.find_last_of("/\\");
    std::string fileName = inputFileName_LUTdata.substr(lastSlashPos + 1);
    // Extract the LUT name (before the last dot)
    size_t dotPos = fileName.find_last_of('.');
    std::string lutName = fileName.substr(0, dotPos);
    // Extract the number from the LUT name
    std::string number;
    for (char c : lutName) {
        if (std::isdigit(c)) {
            number += c;
        }
    }
    // Convert the extracted number to a double and store it in metadata
    if (!number.empty()) {
        metadata["lut_name"] = std::stod(number); // Correct conversion
    } else {
        std::cerr << "Warning: No numeric part found in LUT name." << std::endl;
    }

    // Add LUT size to metadata
    if (!LUT.empty())
    {
        metadata["lut_size"] = static_cast<double>(LUT[0].size());
    }
    else
    {
        std::cerr << "Warning: LUT is empty. Cannot add lut_size to metadata." << std::endl;
    }

    // Perfomr paralel to serial convertion
    std::vector<std::complex<int>> y_serial;
    parallelToSerialConverterComplex(y, LUT, y_serial);

    // Write to the file
    if (!writeToFile(outputFileName_serialData, y_serial, metadata))
    {
        return 1; // Exit if writing fails
    }

    return 0;
}