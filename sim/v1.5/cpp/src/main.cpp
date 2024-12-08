#include <iostream>
#include <string>
#include <vector>
#include "funcs.hpp"

int main(int argc, char* argv[])
{
    // Check if file name is passed as argument
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::string inputFileName = argv[1];
    std::string outputFileName = argv[2];

    std::vector<double> x;
    std::vector<double> y;

    // Read data form file
    if (!readFromFile(inputFileName, x))
    {
        return 1;   // Exit if reading fails
    }

    // Perform delta-sigma modulation
    deltaSigma(x, y);

    // Write to the file
    if (!writeToFile(outputFileName, y))
    {
        return 1; // Exit if writing fails
    }

    return 0;
}