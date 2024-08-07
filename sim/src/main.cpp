#include <iostream>
#include <fstream>
#include "../inc/ap_fixed.h"

// Coefficients of IIR as double
const double bD[] = {3.19382974, -8.02002256, 8.73762976, -4.61756997, 0.97458298};
const double aD[] = {1, -1.63632004, 1.47600867, -0.75840147, 0.2125798, -0.02541702};

int main() {
    // Coefficients of H0
    const ap_fixed<12, 4, AP_RND, AP_SAT, true> b00 = 7.3765809;
    const ap_fixed<12, 1, AP_RND, AP_SAT, true> a01 = -0.3466036;

    // Coefficients of H1
    const ap_fixed<12, 1, AP_RND, AP_SAT, true> b10 = 0.424071040;
    const ap_fixed<12, 3, AP_RND, AP_SAT, true> b11 = -2.782608716;
    const ap_fixed<12, 1, AP_RND, AP_SAT, true> a11 = -0.66591402;
    const ap_fixed<12, 1, AP_RND, AP_SAT, true> a12 = 0.16260264;

    // Coefficients of H2
    const ap_fixed<12, 4, AP_RND, AP_SAT, true> b20 = -4.606822182;
    const ap_fixed<12, 1, AP_RND, AP_SAT, true> b21 = 0.023331537;
    const ap_fixed<12, 1, AP_RND, AP_SAT, true> a21 = -0.62380242;
    const ap_fixed<12, 1, AP_RND, AP_SAT, true> a22 = 0.4509869;

    ap_fixed<32, 5, AP_RND, AP_SAT, true> iir_out;      // IIR's output
    ap_fixed<32, 5, AP_RND, AP_SAT, true> e;            // Quantization error
    ap_fixed<32, 5, AP_RND, AP_SAT, true> q_in;         // Quantizator's input
    // H0
    ap_fixed<32, 5, AP_RND, AP_SAT, true> x0;       // Output of the filter
    ap_fixed<32, 5, AP_RND, AP_SAT, true> x0d;      // Delay element
    // H1
    ap_fixed<32, 5, AP_RND, AP_SAT, true> x1;       // Output of the filter
    ap_fixed<32, 5, AP_RND, AP_SAT, true> w1;       // Intermediate value
    ap_fixed<32, 5, AP_RND, AP_SAT, true> w1d;      // Delay element
    ap_fixed<32, 5, AP_RND, AP_SAT, true> w1dd;     // Delay element
    // H2
    ap_fixed<32, 5, AP_RND, AP_SAT, true> x2;       // Output of the filter
    ap_fixed<32, 5, AP_RND, AP_SAT, true> w2;       // Intermediate value
    ap_fixed<32, 5, AP_RND, AP_SAT, true> w2d;      // Delay element
    ap_fixed<32, 5, AP_RND, AP_SAT, true> w2dd;     // Delay element

    ap_fixed<32, 5, AP_RND, AP_SAT, true> x;     // Input
    ap_fixed<5, 5, AP_RND, AP_SAT, true> y;     // Output

    // Open the input file
    std::ifstream inputFile("data/input_signal.txt");    // Open the file
    if(!inputFile) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    // Open the output file
    std::ofstream outputFile("data/output_signal.txt");  // Open the output file
    if(!outputFile) {
        std::cerr << "Error opening output file!" << std::endl;
        return 1;
    }

    std::string line;

    while(std::getline(inputFile, line)) {
        try {
            double value =std::stod(line);     // Convert string to double

            x = value;

            std::cout << "value = " << value << std::endl;

            q_in = x + iir_out;
            y = q_in;
            // if (y.range(y.width-1, y.width-1) == 0) {
            //     y = y + ap_fixed<12, 4, AP_RND, AP_SAT, true>(1);
            // }
            e = q_in - y;
            // std::cout << "e = " << e << std::endl;

            x0 = b00*e - a01*x0d;
            w1 = e - a11*w1d - a12*w1dd;
            x1 = b10*w1 + b11*w1d;
            w2 = e - a21*w2d - a22*w2dd;
            x2 = b20*w2 + b21*w2d;
            iir_out = x0 + x1 + x2;

            // std::cout << "iir_out = " << iir_out << std::endl;

            x0d = x0;
            w1dd = w1d;
            w1d = w1;
            w2dd = w2d;
            w2d = w2;

            outputFile << y << std::endl;  // Write the result to the file
        } 
        catch (const std::invalid_argument& e) {
            std::cerr << "Invalid number in file: " << line << std::endl;
            return 1;
        }
    }

    inputFile.close();
    outputFile.close();
    
    return 0;
}