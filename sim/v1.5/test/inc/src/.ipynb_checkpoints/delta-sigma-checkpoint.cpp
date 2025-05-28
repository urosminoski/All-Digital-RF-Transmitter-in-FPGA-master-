#include <iostream>
#include <vector>
#include <complex>
#include <unordered_map>
#include "fxpDsp.hpp"
#include "fileProcessing.hpp"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " x_real_in.txt x_imag_in.txt x_real_out.txt x_imag_out.txt\n";
        return 1;
    }

    // constexpr int W = 18;
    // constexpr int I = 1;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;

    std::vector<double>               x_real, x_imag;
    std::unordered_map<std::string, std::string>    metadataReal, metadataImag;
    readRealData(argv[1], metadataReal, x_real);
    readRealData(argv[2], metadataImag, x_imag);

    std::vector<std::vector<double>> iir_deltaSigma;
    std::unordered_map<std::string, std::string> iir_deltaSigma_metadata;
    readReal2dData(argv[5], iir_deltaSigma_metadata, iir_deltaSigma);

    deltaSigma_real<12, 4, 12, 4, 4, S, Q, O>(x_real, iir_deltaSigma, true);
    deltaSigma_real<12, 4, 12, 4, 4, S, Q, O>(x_imag, iir_deltaSigma, true);

    writeRealData(argv[3], metadataReal, x_real);
    writeRealData(argv[4], metadataImag, x_imag);
}