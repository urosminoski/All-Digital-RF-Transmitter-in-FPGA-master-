#include <iostream>
#include <vector>
#include <complex>
#include <unordered_map>
#include "fxpDsp.hpp"
#include "fileProcessing.hpp"

static void scale_vector(std::vector<double>& vector, double scale);

int main(int argc, char* argv[]) {
    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " x_real_in.txt x_imag_in.txt x_real_out.txt x_imag_out.txt\n";
        return 1;
    }

    // constexpr int W = 12;
    // constexpr int I = 5;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;

    // Load FIR coeffitients for polyphase interpolation
    std::vector<std::vector<double>> polyFirCofficients;
    std::unordered_map<std::string, std::string> polyFirMetadata;
    readReal2dData(argv[4], polyFirMetadata, polyFirCofficients);

    // Load IIR filter coeffitients for Delta-Sigma modulation
    std::vector<std::vector<double>> iir_deltaSigma;
    std::unordered_map<std::string, std::string> iir_deltaSigma_metadata;
    readReal2dData(argv[5], iir_deltaSigma_metadata, iir_deltaSigma);

    // Load LUT data for parallel-to-serial conversion
    std::vector<std::vector<double>> LUT;
    std::unordered_map<std::string, std::string> LUT_metadata;
    readReal2dData(argv[6], LUT_metadata, LUT);

    // Load input data
    std::vector<double> xi, xq;
    std::unordered_map<std::string, std::string>    metadata_i, metadata_q;
    readRealData(argv[1], metadata_i, xi);
    readRealData(argv[2], metadata_q, xq);

    // Perform Interpolation by a factor of 8
    // interpolation_real<12, 1, S, Q, O>(xi, polyFirCofficients);
    // interpolation_real<12, 1, S, Q, O>(xq, polyFirCofficients);

    // Perform Delta-Sigma Modulation
    scale_vector(xi, 4);
    scale_vector(xq, 4);
    deltaSigma_real<12, 4, 12, 4, 4, S, Q, O>(xi, iir_deltaSigma, argv[7], true);
    deltaSigma_real<12, 4, 12, 4, 4, S, Q, O>(xq, iir_deltaSigma, argv[7], true);

    // Perform Serializtion with LUT
    serialConverter<4>(xi, LUT);
    serialConverter<4>(xq, LUT);

    // Perform RFIQ reconstruction
    std::vector<double> x_rec;
    rfiq(xi, xq, x_rec);

    writeRealData(argv[3], metadata_i, x_rec);

}

static void scale_vector(std::vector<double>& vector, double scale) {
    // Ensure vector is not empty
    if (vector.empty()) {
        throw std::runtime_error("Input vector is empty!");
    }
    // Ensure scale is not 0
    if (scale == 0) {
        throw std::runtime_error("Scale cannot be 0!");
    }

    for (auto& val : vector) {
        val *= scale;
    }
}