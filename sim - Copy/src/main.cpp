#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include "../inc/FixedPoint.hpp"
#include "../inc/IIR_filter.hpp"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Coefficients as double
const double bD[] = {3.19382974, -8.02002256, 8.73762976, -4.61756997, 0.97458298};
const double aD[] = {1, -1.63632004, 1.47600867, -0.75840147, 0.2125798, -0.02541702};

// Convert coefficients to FixedPoint with given precision
const uint8_t totalBits = 12;
const uint8_t fracBits = 9;
const fxp::OverflowMode mode = fxp::OverflowMode::Saturate;

fxp::FixedPoint simulateDeltaSigma(double value);

int main() {

    // Coefficients of H0
    const fxp::FixedPoint b00(7.3765809, 12, 8, mode);
    const fxp::FixedPoint a01(-0.3466036, 12, 11, mode);

    std::cout << std::fixed << std::setprecision(9) << "b00 = " << b00.ToDouble() << "(" << b00.ToInt()<< << std::endl;
    std::cout << std::fixed << std::setprecision(12) << "a01 = " << a01.ToDouble() << std::endl;
    std::cout << " " << std::endl;

    // Coefficients of H1
    const fxp::FixedPoint b10(0.424071040, 12, 11, mode);
    const fxp::FixedPoint b11(-2.782608716, 12, 9, mode);
    const fxp::FixedPoint a11(-0.66591402, 12, 11, mode);
    const fxp::FixedPoint a12(0.16260264, 12, 11, mode);

    std::cout << std::fixed << std::setprecision(11) << "b10 = " << b10.ToDouble() << std::endl;
    std::cout << std::fixed << std::setprecision(9) << "b11 = " << b11.ToDouble() << std::endl;
    std::cout << std::fixed << std::setprecision(11) << "a11 = " << a11.ToDouble() << std::endl;
    std::cout << std::fixed << std::setprecision(11) << "a12 = " << a12.ToDouble() << std::endl;
    std::cout << " " << std::endl;

    // Coefficients of H2
    const fxp::FixedPoint b20(-4.606822182, 12, 8, mode);
    const fxp::FixedPoint b21(0.023331537, 12, 11, mode);
    const fxp::FixedPoint a21(-0.62380242, 12, 11, mode);
    const fxp::FixedPoint a22(0.4509869, 12, 11, mode);

    std::cout << std::fixed << std::setprecision(11) << "b20 = " << b20.ToDouble() << std::endl;
    std::cout << std::fixed << std::setprecision(9) << "b21 = " << b21.ToDouble() << std::endl;
    std::cout << std::fixed << std::setprecision(11) << "a21 = " << a21.ToDouble() << std::endl;
    std::cout << std::fixed << std::setprecision(11) << "a22 = " << a22.ToDouble() << std::endl;
    std::cout << " " << std::endl;

    fxp::FixedPoint iir_out(12, 8, mode);  // IIR's output
    fxp::FixedPoint e(12, 8, mode);        // Quantization error
    fxp::FixedPoint q_in(12, 8, mode);     // Quantizator's input
    // H0
    fxp::FixedPoint x0(0, 24, 19, mode);   // Output of the filter
    fxp::FixedPoint x0d(0, 24, 19, mode);  // Delay element
    // H1
    fxp::FixedPoint x1(24, 19, mode);   // Output of the filter
    fxp::FixedPoint w1(24, 19, mode);   // Intermediate value
    fxp::FixedPoint w1d(24, 19, mode);  // Delay element
    fxp::FixedPoint w1dd(24, 19, mode); // Delay element
    // H2
    fxp::FixedPoint x2(24, 19, mode);   // Output of the filter
    fxp::FixedPoint w2(24, 19, mode);   // Intermediate value
    fxp::FixedPoint w2d(24, 19, mode);  // Delay element
    fxp::FixedPoint w2dd(24, 19, mode); // Delay element

    fxp::FixedPoint x(12, 8, mode);     // Input
    fxp::FixedPoint y(5, 0, mode);      // Output

    // fxp::FixedPoint x(0.88177766, 12, 8, mode);

    // q_in = iir_out + x;
    // y = q_in;
    // e = q_in - y;

    // x0 = b00*e - a01*x0d;
    // w1 = e - a11*w1d -a12*w1dd;
    // x1 = b10*w1 + b11*w1d;
    // w2 = e - a21*w2d -a22*w2dd;
    // x2 = b20*w2 + b21*w2d;
    // iir_out = x0 + x1 + x2;

    // x0d = x0;
    // w1dd = w1d;
    // w1d = w1;
    // w2dd = w2d;
    // w2d = w2;

    // std::cout << std::fixed << std::setprecision(9) << "q_in = " << q_in.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(1) << "y = " << y.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(9) << "e = " << e.ToDouble() << std::endl;

    // std::cout << std::fixed << std::setprecision(20) << "x0d = " << x0d.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "x0 = " << x0.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w1dd = " << w1dd.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w1d = " << w1d.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w1 = " << w1.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "x1 = " << x1.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w2dd = " << w2dd.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w2d = " << w2d.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w2 = " << w2.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "x2 = " << x2.ToDouble() << std::endl;

    // std::cout << std::fixed << std::setprecision(20) << "iir_out = " << iir_out.ToDouble() << std::endl;
    // std::cout << " " << std::endl;

    // x = fxp::FixedPoint(1.75280992, 12, 8, mode);

    // q_in = iir_out + x;
    // y = q_in;
    // e = q_in - y;

    // x0 = b00*e - a01*x0d;
    // w1 = e - a11*w1d -a12*w1dd;
    // x1 = b10*w1 + b11*w1d;
    // w2 = e - a21*w2d -a22*w2dd;
    // x2 = b20*w2 + b21*w2d;
    // iir_out = x0 + x1 + x2;

    // x0d = x0;
    // w1dd = w1d;
    // w1d = w1;
    // w2dd = w2d;
    // w2d = w2;

    // std::cout << std::fixed << std::setprecision(9) << "q_in = " << q_in.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(1) << "y = " << y.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(9) << "e = " << e.ToDouble() << std::endl;

    // std::cout << std::fixed << std::setprecision(20) << "x0d = " << x0d.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "x0 = " << x0.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w1dd = " << w1dd.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w1d = " << w1d.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w1 = " << w1.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "x1 = " << x1.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w2dd = " << w2dd.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w2d = " << w2d.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "w2 = " << w2.ToDouble() << std::endl;
    // std::cout << std::fixed << std::setprecision(20) << "x2 = " << x2.ToDouble() << std::endl;

    // std::cout << std::fixed << std::setprecision(20) << "iir_out = " << iir_out.ToDouble() << std::endl;
    // std::cout << " " << std::endl;

    // Load input data
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

    int k = 0;

    while(std::getline(inputFile, line)) {
        try {
            double value = std::stod(line);     // Convert string to double
            // fxp::FixedPoint outValue = simulateDeltaSigma(value);
            // outValue.PrintConfig();
            // outputFile << outValue.ToDouble() << std::endl;  // Write the result to the file

            x = value;

            q_in = x + iir_out;
            y = q_in;
            e = q_in - y;

            x0 = b00*e - a01*x0d;
            w1 = e - a11*w1d - a12*w1dd;
            x1 = b10*w1 + b11*w1d;
            w2 = e - a21*w2d - a22*w2dd;
            x2 = b20*w2 + b21*w2d;

            iir_out = x0 + x1 + x2;

            std::cout << std::fixed << std::setprecision(20) << "iir_out = " << x0.ToDouble() << std::endl;
            std::cout << " " << std::endl;

            std::cout << std::fixed << std::setprecision(20) << "x0d = " << x0d.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "x0 = " << x0.ToDouble() << std::endl;
            std::cout << " " << std::endl;

            std::cout << std::fixed << std::setprecision(20) << "w1dd = " << w1dd.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "w1d = " << w1d.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "w1 = " << w1.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "x1 = " << x1.ToDouble() << std::endl;
            std::cout << " " << std::endl;

            std::cout << std::fixed << std::setprecision(20) << "w2dd = " << w2dd.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "w2d = " << w2d.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "w2 = " << w2.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "x2 = " << x2.ToDouble() << std::endl;
            std::cout << " " << std::endl;

            x0d = x0;
            w1dd = w1d;
            w1d = w1;
            w2dd = w2d;
            w2d = w2;

            std::cout << std::fixed << std::setprecision(20) << "x0d = " << x0d.ToDouble() << std::endl;
            std::cout << " " << std::endl;

            std::cout << std::fixed << std::setprecision(20) << "w1dd = " << w1dd.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "w1d = " << w1d.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "w1 = " << w1.ToDouble() << std::endl;
            std::cout << " " << std::endl;

            std::cout << std::fixed << std::setprecision(20) << "w2dd = " << w2dd.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "w2d = " << w2d.ToDouble() << std::endl;
            std::cout << std::fixed << std::setprecision(20) << "w2 = " << w2.ToDouble() << std::endl;
            std::cout << "\n\n" << std::endl;

            outputFile << y.ToDouble() << std::endl;  // Write the result to the file

        } 
        catch (const std::invalid_argument& e) {
            std::cerr << "Invalid number in file: " << line << std::endl;
            return 1;
        }
        if(k++ >= 3) {
            break;
        }
    }

    inputFile.close();
    outputFile.close();

    // // Create some FixedPoint objects with different fractional and total bits
    // fxp::FixedPoint a(1.125, 8, 4, fxp::OverflowMode::Saturate); 
    // fxp::FixedPoint b(2.75, 10, 6, fxp::OverflowMode::Saturate); 
    // fxp::FixedPoint c(3.25, 12, 8, fxp::OverflowMode::Saturate); 
    // fxp::FixedPoint d(-5.325, 15, 10, fxp::OverflowMode::Saturate); 
    // fxp::FixedPoint f( -10.5938, 15, 5, fxp::OverflowMode::Saturate); 

    // fxp::FixedPoint h(3, 2, fxp::OverflowMode::Saturate);
    // h = 2;
    // std::cout << std::fixed << std::setprecision(10) << "h = " << h.ToDouble() << std::endl;
    // h.PrintConfig();

    // std::cout << std::fixed << std::setprecision(10) << "a*b = " << (a*b).ToDouble() << std::endl;
    // (a*b).PrintConfig();

    // std::cout << std::fixed << std::setprecision(10) << "a*b = " << (c*d).ToDouble() << std::endl;
    // (c*d).PrintConfig();

    // std::cout << std::fixed << std::setprecision(10) << "a*c = " << (a*c).ToDouble() << std::endl;
    // (a*c).PrintConfig();

    // std::cout << std::fixed << std::setprecision(10) << "a*b + c*d + a*c = " << (a*b + c*d + a*c).ToDouble() << std::endl;
    // (a*b + c*d + a*c).PrintConfig();

    

    // // f.ToDouble();
    // std::cout << std::fixed << std::setprecision(10) << "f = " << f.ToDouble() << std::endl;
    // f.PrintConfig();

    return 0;
}

// Simulate delta-sigma modultaion
fxp::FixedPoint simulateDeltaSigma(double value) {
    fxp::FixedPoint x(value, totalBits, fracBits, mode);
    static fxp::FixedPoint dx1(0, totalBits, fracBits, mode);
    static fxp::FixedPoint dx2(0, totalBits, fracBits, mode);
    static fxp::FixedPoint dx3(0, totalBits, fracBits, mode);
    static fxp::FixedPoint dx4(0, totalBits, fracBits, mode);

    fxp::FixedPoint y(5, 0, mode);
    static fxp::FixedPoint dy1(0, totalBits, fracBits, mode);
    static fxp::FixedPoint dy2(0, totalBits, fracBits, mode);
    static fxp::FixedPoint dy3(0, totalBits, fracBits, mode);
    static fxp::FixedPoint dy4(0, totalBits, fracBits, mode);
    static fxp::FixedPoint dy5(0, totalBits, fracBits, mode);

    fxp::FixedPoint b0(bD[0], totalBits, fracBits, mode);
    fxp::FixedPoint b1(bD[1], totalBits, fracBits, mode);
    fxp::FixedPoint b2(bD[2], totalBits, fracBits, mode);
    fxp::FixedPoint b3(bD[3], totalBits, fracBits, mode);
    fxp::FixedPoint b4(bD[4], totalBits, fracBits, mode);

    fxp::FixedPoint a0(aD[0], totalBits, fracBits, mode);
    fxp::FixedPoint a1(aD[1], totalBits, fracBits, mode);
    fxp::FixedPoint a2(aD[2], totalBits, fracBits, mode);
    fxp::FixedPoint a3(aD[3], totalBits, fracBits, mode);
    fxp::FixedPoint a4(aD[4], totalBits, fracBits, mode);
    fxp::FixedPoint a5(aD[5], totalBits, fracBits, mode);

    fxp::FixedPoint z1(totalBits << 1, fracBits << 1, mode);
    fxp::FixedPoint z2(totalBits << 1, fracBits << 1, mode);

    static fxp::FixedPoint IIR_out(0, totalBits, fracBits, mode);
    static fxp::FixedPoint IIR_in(0, totalBits, fracBits, mode);
    static fxp::FixedPoint trunc_in(0, totalBits, fracBits, mode);

    x = fxp::FixedPoint(value, totalBits, fracBits, mode);

    trunc_in = x + IIR_out;
    y = trunc_in;
    IIR_in = trunc_in - y;

    z1 = b0*IIR_in + b1*dx1 + b2*dx2 + b3*dx3 + b4*dx4;
    z2 = a1*dy1 + a2*dy2 + a3*dy3 + a4*dy4 + a5*dy5;

    IIR_out = z2 - z1;

    return y;
}
