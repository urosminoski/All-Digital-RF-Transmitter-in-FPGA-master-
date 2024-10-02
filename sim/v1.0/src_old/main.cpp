#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "../inc/FixedPoint.hpp"
#include "../inc/IIR_filter.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

int main() {
    // Coefficients as double
    const double bD[] = {3.19382974, -8.02002256, 8.73762976, -4.61756997, 0.97458298};
    const double aD[] = {1, -1.63632004, 1.47600867, -0.75840147, 0.2125798, -0.02541702};

    // Convert coefficients to FixedPoint with given precision
    uint8_t totalBits = 12;
    uint8_t fracBits = 8;
    fxp::OverflowMode mode = fxp::OverflowMode::Saturate;

    std::vector<fxp::FixedPoint> b, a;
    for (double coef : bD) {
        b.emplace_back(coef, totalBits, fracBits, mode);
    }
    for (double coef : aD) {
        a.emplace_back(coef, totalBits, fracBits, mode);
    }

    // Load input data
    std::ifstream inputFile("data/input_signal.txt");    // Open the file
    if(!inputFile) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    std::vector<fxp::FixedPoint> data;   // Vector to store the data
    std::string line;

    while(std::getline(inputFile, line)) {
        try {
            double value = std::stod(line);     // Convert string to double
            data.push_back(fxp::FixedPoint(value, totalBits, fracBits, mode));              // Store vale to the vector
        } 
        catch (const std::invalid_argument& e) {
            std::cerr << "Invalid number in file: " << line << std::endl;
            return 1;
        }
    }

    inputFile.close();


    // Simulate delta-sigma modultaion


    return 0;
}
