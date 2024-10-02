#include "../inc/IIR_filter.hpp"

void fixedPointDirectOne_IIR(const std::vector<fxp::FixedPoint> &inSignal,
                            const std::vector<fxp::FixedPoint> &b,
                            const std::vector<fxp::FixedPoint> &a,
                            std::vector<fxp::FixedPoint> &outSignal){
                            

    size_t nb = b.size();   // Number of b coefficients
    size_t na = a.size();   // Number of a coefficients

    auto totalBits = inSignal[0].GetTotalBits();
    auto fracBits = inSignal[0].GetFracBits(); 
    auto mode = inSignal[0].GetOverflowMode();

    // Delay lines initialized to zero
    std::vector<fxp::FixedPoint> xDelay(nb, fxp::FixedPoint(totalBits, fracBits, mode));
    std::vector<fxp::FixedPoint> yDelay(na, fxp::FixedPoint(totalBits, fracBits, mode));
    

    for(size_t i = 0; i < inSignal.size(); i++){

        // Update the x[] delay line
        for(size_t j = nb - 1; j > 0; j--){
            xDelay[j] = xDelay[j - 1];
        }
        xDelay[0] = inSignal[i];

        // Filter x[] with coeffitients b[]
        fxp::FixedPoint z1 = b[0] * xDelay[0];  // z1 has double precision
        z1.PrintConfig();
        for(size_t j = 1; j < nb; j++){
            z1 = z1 + (b[j] * xDelay[j]);   // z1 maintains double precision
        }

        // Update the y[] delay line
        for(size_t j = na - 1; j > 0; j--){
            yDelay[j] = yDelay[j - 1];
        }

        // Filter y[] with coefficients a[]
        fxp::FixedPoint z2 = fxp::FixedPoint(totalBits << 1, fracBits << 1, mode);  // Double width zero initialization
        z2.PrintConfig();
        for(size_t j = 1; j < na; j++){
            z2 = z2 + (a[j] * yDelay[j]);   // z2 accumulates with double precision
        }

        z1 = z1 - z2;   // Still in double precision
        z1.Resize(totalBits, fracBits); // Resize to original precision
        outSignal[i] = z1;  // Store result

        yDelay[0] = z1; // Update yDelay with the resized result

    }
}