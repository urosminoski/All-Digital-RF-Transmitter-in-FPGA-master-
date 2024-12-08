#include <iostream>
#include <string>
#include <vector>
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

    std::vector<double> x;
    std::vector<int> y;

    // Read data form file
    if (!readFromFile(inputFileName_sinData, x))
    {
        return 1;   // Exit if reading fails
    }

    // Perform delta-sigma modulation
    deltaSigma(x, y);

    // Write to the file
    if (!writeToFile(outputFileName_deltaSigma, y))
    {
        return 1; // Exit if writing fails
    }

    // Read LUT
    std::vector<std::vector<int>> LUT;
    readLUT(inputFileName_LUTdata, LUT);

    // Perfomr paralel to serial convertion
    std::vector<int> y_serial;
    parallelToSerialConverter(y, LUT, y_serial);

    // Write to the file
    if (!writeToFile(outputFileName_serialData, y_serial))
    {
        return 1; // Exit if writing fails
    }

    return 0;
}