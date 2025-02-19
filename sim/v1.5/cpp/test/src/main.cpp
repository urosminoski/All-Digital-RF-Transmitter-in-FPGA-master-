#include <iostream>
#include <vector>
#include <complex>
#include <unordered_map>
#include <nlohmann/json.hpp> // JSON handling library
#include "fxpDsp.hpp"
#include "fileProcessing.hpp"

bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT);
void splitComplexVector(const std::vector<std::complex<double>>& complexVec, 
                        std::vector<double>& realVec, 
                        std::vector<double>& imagVec);
std::vector<std::complex<double>> combineRealImagVectors(const std::vector<double>& realVec, 
                                                         const std::vector<double>& imagVec);
void scale_vector(std::vector<double>& vector, double scale);

int main() {

    constexpr int W = 18;
    constexpr int I = 1;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;



    // Load FIR coeffitients for T/2 and T/4 delay
    std::string fileName_delayFirCofficients_5   = "./data/input/delayFirCoefficients_5_20dB.txt";     // T/2 delay
    std::string fileName_delayFirCofficients_25  = "./data/input/delayFirCoefficients_25_20dB.txt";    // T/4 delay
    std::vector<double> delayFirCofficients_5;  
    std::vector<double> delayFirCofficients_25; 
    std::unordered_map<std::string, std::string>    delayFirMetadata_5;
    std::unordered_map<std::string, std::string>    delayFirMetadata_25;
    readRealData(fileName_delayFirCofficients_5, delayFirMetadata_5, delayFirCofficients_5);
    readRealData(fileName_delayFirCofficients_25, delayFirMetadata_25, delayFirCofficients_25);

    // Load FIR coeffitients for polyphase interpolation
    std::string fileName_polyFirCofficients   = "./data/input/polyFirCofficients_80dB.txt"; 
    std::vector<std::vector<double>> polyFirCofficients;
    std::unordered_map<std::string, std::string> polyFirMetadata;
    readReal2dData(fileName_polyFirCofficients, polyFirMetadata, polyFirCofficients);

    // Load IIR coeffitients for delta-sigma modulation
    std::string fileName_iir_deltaSigma   = "./data/input/deltaSigma_iirs.txt"; 
    std::vector<std::vector<double>> iir_deltaSigma;
    std::unordered_map<std::string, std::string> iir_deltaSigma_metadata;
    readReal2dData(fileName_iir_deltaSigma, iir_deltaSigma_metadata, iir_deltaSigma);

    // Load LUT data for parallel-to-serial conversion
    std::string fileName_LUT   = "./data/input/LUT3.txt";
    std::vector<std::vector<double>> LUT;
    std::unordered_map<std::string, std::string> LUT_metadata;
    readReal2dData(fileName_LUT, LUT_metadata, LUT);

    std::string fileName_sinData_complex        = "./data/input/sinDataComplex.txt";
    std::string fileName_sinDataR_delay_5       = "./data/output/sinDataR_delay_5.txt";
    std::string fileName_sinDataI_delay_25      = "./data/output/sinDataI_delay_25.txt";
    std::string fileName_sinDataR_OSR_8         = "./data/output/sinDataR_OSR_8.txt";
    std::string fileName_sinDataI_OSR_8         = "./data/output/sinDataI_OSR_8.txt";
    std::string fileName_sinDataR_deltaSigma    = "./data/output/sinDataR_deltaSigma.txt";
    std::string fileName_sinDataI_deltaSigma    = "./data/output/sinDataI_deltaSigma.txt";
    std::string fileName_sinData_deltaSigma_d   = "./data/output/sinData_deltaSigma_double.txt";
    std::string fileName_sinDataR_serial        = "./data/output/sinDataR_serial.txt";
    std::string fileName_sinDataI_serial        = "./data/output/sinDataI_serial.txt";
    std::string fileName_sinDataR_rfiq          = "./data/output/sinData_rfiq.txt";

    // Load input complex sin data
    std::vector<std::complex<double>>               inSignal;
    std::unordered_map<std::string, std::string>    metadata;
    readComplexData(fileName_sinData_complex, metadata, inSignal);

    // Vectors to hold real and imaginary parts
    std::vector<double> realPart, imagPart;
    realPart.reserve(inSignal.size());
    imagPart.reserve(inSignal.size());
    splitComplexVector(inSignal, realPart, imagPart);

    metadata["complex"] = "0";

    // Delay Real Part for T/2
    delay_real<W, I, S, Q, O>(realPart, delayFirCofficients_5, 2, 1);
    writeRealData(fileName_sinDataR_delay_5, metadata, realPart);

    // Delay Imaginary Part for T/4
    delay_real<W, I, S, Q, O>(imagPart, delayFirCofficients_25, 2, 1);
    writeRealData(fileName_sinDataI_delay_25, metadata, imagPart);

    metadata["OSR"] = std::to_string(static_cast<int>(std::pow(2, polyFirCofficients.size())));

    // Perform Interpolation by a factor of 8
    interpolation_real<W, I, S, Q, O>(realPart, polyFirCofficients);
    interpolation_real<W, I, S, Q, O>(imagPart, polyFirCofficients);
    writeRealData(fileName_sinDataR_OSR_8, metadata, realPart);
    writeRealData(fileName_sinDataI_OSR_8, metadata, imagPart);

    std::cout << "Before : " << realPart[10] << std::endl;  
    scale_vector(realPart, 4);
    scale_vector(imagPart, 4);
    std::cout << "After : " << realPart[10] << std::endl;
    
    // Perform delta-sigma modulation on real and imaginary parts
    deltaSigma_real<12, 4, 12, 4, 4, S, Q, O>(realPart, iir_deltaSigma, true);
    deltaSigma_real<12, 4, 12, 4, 4, S, Q, O>(imagPart, iir_deltaSigma, true);
    writeRealData(fileName_sinDataR_deltaSigma, metadata, realPart);
    writeRealData(fileName_sinDataI_deltaSigma, metadata, imagPart);

    // Perform parallel-to-serial conversion on real and imaginary parts
    metadata["lut_width"] = std::to_string(LUT[0].size());
    serialConverter<4>(realPart, LUT);
    serialConverter<4>(imagPart, LUT);
    writeRealData(fileName_sinDataR_serial, metadata, realPart);
    writeRealData(fileName_sinDataI_serial, metadata, imagPart);

    // Perform RFIQ reconstruction
    std::vector<double> recSignal;
    rfiq(realPart, imagPart, recSignal);
    writeRealData(fileName_sinDataR_rfiq, metadata, recSignal);



    // std::vector<double> a = {1.2, -2.5, 0.9, 2.1, 3.4, -3.7};
    // std::vector<double> b;
    // quantize_real<3, 3, true, AC_TRN, AC_SAT>(a, b);
    // writeRealData("./data/output/tmp.txt", metadata, b);




    // // // Quantize real sin data and write it to file
    // // std::vector<double> sinDataQuant;
    // // sinDataQuant.reserve(sinData.size());
    // // quantizeReal<W, I, S, Q, O>(sinData, sinDataQuant);
    // // writeRealData(fileName_sinDataQuant, sinMeatadata, sinDataQuant);

    // // // Perform FIR filtering
    // // std::vector<double> sinDataFir(sinData.size());
    // // std::copy(sinData.begin(), sinData.end(), sinDataFir.begin());
    // // firReal<W, I, S, Q, O>(sinDataFir, delayFirCofficients_5);
    // // writeRealData(fileName_sinDataFir, sinMeatadata, sinDataFir);

    // // Perform T/2 delay
    // std::vector<double> sinDataDelay_5(sinData.size());
    // std::copy(sinData.begin(), sinData.end(), sinDataDelay_5.begin());
    // delay_real<W, I, S, Q, O>(sinDataDelay_5, delayFirCofficients_5, 2, 1);
    // writeRealData(fileName_sinDataDelay_5, sinMeatadata, sinDataDelay_5);

    // // Perform T/4 delay
    // std::vector<double> sinDataDelay_25(sinData.size());
    // std::copy(sinData.begin(), sinData.end(), sinDataDelay_25.begin());
    // delay_real<W, I, S, Q, O>(sinDataDelay_25, delayFirCofficients_25, 4, 3);
    // writeRealData(fileName_sinDataDelay_25, sinMeatadata, sinDataDelay_25);

    // // Perform delta-sigma modulation
    // std::vector<double> sinData_deltaSigma(sinData.size());
    // std::copy(sinData.begin(), sinData.end(), sinData_deltaSigma.begin());
    // deltaSigma_real<12, 4, 12, 4, 4, S, Q, O>(sinData_deltaSigma, iir_deltaSigma, false ); 
    // writeRealData(fileName_sinData_deltaSigma, sinMeatadata, sinData_deltaSigma);

    // // interpolatorReal<W, I, S, Q, O>(sinData, polyFirCofficients[0], 2);
    // // writeRealData(fileName_sinDataOSR_2,
    // //                 sinMeatadata,
    // //                 sinData);
    
    // // interpolationReal<W, I, S, Q, O>(sinData, polyFirCofficients);
    // // writeRealData(fileName_sinDataOSR_8,
    // //                 sinMeatadata,
    // //                 sinData);


    // std::string fileName_sinDataComplex         = "./data/input/sinDataComplex.txt";
    // std::string fileName_sinDataComplexQuant    = "./data/output/sinDataComplex_quant.txt";
    // std::string fileName_sinDataComplexFir      = "./data/output/sinDataComplex_fir.txt";
    // std::string fileName_sinDataComplexDelay_5  = "./data/output/sinDataComplex_delay_5.txt";
    // std::string fileName_sinDataComplexDelay_25 = "./data/output/sinDataComplex_delay_25.txt";
    // std::string fileName_sinDataComplexOSR_2    = "./data/output/sinDataComplex_OSR_2.txt";
    // std::string fileName_sinDataComplexOSR_8    = "./data/output/sinDataComplex_OSR_8.txt";

    // // Load input complex sin data
    // std::vector<std::complex<double>>               sinDataComplex;
    // std::unordered_map<std::string, std::string>    sinComplexMeatadata;
    // readComplexData("./data/input/sinDataComplex.txt", sinComplexMeatadata, sinDataComplex);

    // // // // // Quantize complex sin data and write it to file
    // // // std::vector<std::complex<double>> sinDataComplexQuant;
    // // // sinDataComplexQuant.reserve(sinDataComplex.size());
    // // // quantizeComplex<W, I, S, Q, O>(sinDataComplex, sinDataComplexQuant);
    // // // writeComplexData(fileName_sinDataComplexQuant, sinComplexMeatadata, sinDataComplexQuant);

    // // Perform FIR filtering
    // std::vector<std::complex<double>> sinDataComplexFir(sinDataComplex.size());
    // std::copy(sinDataComplex.begin(), sinDataComplex.end(), sinDataComplexFir.begin());
    // interpolator_complex<W, I, S, Q, O>(sinDataComplexFir, polyFirCofficients[0], 2);
    // interpolator_complex<W, I, S, Q, O>(sinDataComplexFir, polyFirCofficients[1], 2);
    // writeComplexData("./data/output/tmp1.txt", sinComplexMeatadata, sinDataComplexFir);

    // // Perform T/2 delay
    // std::vector<std::complex<double>> sinDataComplexDelay_5(sinDataComplex.size());
    // std::copy(sinDataComplex.begin(), sinDataComplex.end(), sinDataComplexDelay_5.begin());
    // delay_complex<W, I, S, Q, O>(sinDataComplexDelay_5, delayFirCofficients_5, 2, 1);
    // writeComplexData(fileName_sinDataComplexDelay_5, sinComplexMeatadata, sinDataComplexDelay_5);

    // // Perform T/4 delay
    // std::vector<std::complex<double>> sinDataComplexDelay_25(sinDataComplex.size());
    // std::copy(sinDataComplex.begin(), sinDataComplex.end(), sinDataComplexDelay_25.begin());
    // delay_complex<W, I, S, Q, O>(sinDataComplexDelay_25, delayFirCofficients_25, 4, 3);
    // writeComplexData(fileName_sinDataComplexDelay_25, sinComplexMeatadata, sinDataComplexDelay_25);


    // // interpolatorComplex<W, I, S, Q, O>(sinDataComplex, polyFirCofficients[0], 2);
    // // writeComplexData(fileName_sinDataComplexOSR_2,
    // //                 sinComplexMeatadata,
    // //                 sinDataComplex);

    // // interpolationComplex<W, I, S, Q, O>(sinDataComplex, polyFirCofficients);
    // // writeComplexData(fileName_sinDataComplexOSR_8,
    // //                 sinComplexMeatadata,
    // //                 sinDataComplex);

    return 0;
}

bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT)
{
    // Clear the LUT vector to ensure it starts empty
    LUT.clear();

    // Open the file
    std::ifstream file(fileName);

    // Check if the file can be opened successfully
    if (!file.is_open())
    {
        // Throw an exception if the file cannot be opened
        throw std::runtime_error("Could not open file: " + fileName);
        return false; // Unnecessary but included for clarity
    }

    // Create a JSON object to hold the file content
    nlohmann::json j;

    // Parse the file content into the JSON object
    file >> j;

    // Close the file after reading
    file.close();

    // Convert the JSON object to a 2D vector of integers
    LUT = j.get<std::vector<std::vector<int>>>();

    // Return true to indicate successful parsing
    return true;
}

// Function to split complex vector into real and imaginary vectors
void splitComplexVector(const std::vector<std::complex<double>>& complexVec, 
                        std::vector<double>& realVec, 
                        std::vector<double>& imagVec) {
    realVec.resize(complexVec.size());
    imagVec.resize(complexVec.size());

    for (size_t i = 0; i < complexVec.size(); ++i) {
        realVec[i] = complexVec[i].real();
        imagVec[i] = complexVec[i].imag();
    }
}

// Function to combine real and imaginary vectors into a complex vector
std::vector<std::complex<double>> combineRealImagVectors(const std::vector<double>& realVec, 
                                                         const std::vector<double>& imagVec) {
    if (realVec.size() != imagVec.size()) {
        throw std::runtime_error("Error: Real and Imaginary vectors must have the same size.");
    }

    std::vector<std::complex<double>> complexVec(realVec.size());

    for (size_t i = 0; i < realVec.size(); ++i) {
        complexVec[i] = std::complex<double>(realVec[i], imagVec[i]);
    }

    return complexVec;
}

void scale_vector(std::vector<double>& vector, double scale) {
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