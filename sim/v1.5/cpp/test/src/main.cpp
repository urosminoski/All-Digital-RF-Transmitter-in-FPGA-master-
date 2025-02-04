#include <iostream>
#include <vector>
#include <complex>
#include <nlohmann/json.hpp> // JSON handling library
#include "fxpDsp.hpp"
#include "fileProcessing.hpp"

bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT);

int main() {

    constexpr int W = 18;
    constexpr int I = 1;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;

     // std::string fileName_deltaSigma_iirs    = "../../data/input/deltaSigma_iirs.txt"; 

    // Load FIR coeffitients for T/2 and T/4 delay
    std::string fileName_delayFirCofficients_5   = "./data/input/delayFirCoefficients_5_80dB.txt";     // T/2 delay
    std::string fileName_delayFirCofficients_25  = "./data/input/delayFirCoefficients_25_80dB.txt";    // T/4 delay
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

    std::string fileName_sinData            = "./data/input/sinData.txt";
    std::string fileName_sinDataQuant       = "./data/output/sinData_quant.txt";
    std::string fileName_sinDataFir         = "./data/output/sinData_fir.txt";
    std::string fileName_sinDataDelay_5     = "./data/output/sinData_delay_5.txt";
    std::string fileName_sinDataDelay_25    = "./data/output/sinData_delay_25.txt";
    std::string fileName_sinDataOSR_2       = "./data/output/sinData_OSR_2.txt";
    std::string fileName_sinDataOSR_8       = "./data/output/sinData_OSR_8.txt";

    // Load input real sin data
    std::vector<double>                             sinData;
    std::unordered_map<std::string, std::string>    sinMeatadata;
    readRealData(fileName_sinData, sinMeatadata, sinData);

    // // Quantize real sin data and write it to file
    // std::vector<double> sinDataQuant;
    // sinDataQuant.reserve(sinData.size());
    // quantizeReal<W, I, S, Q, O>(sinData, sinDataQuant);
    // writeRealData(fileName_sinDataQuant, sinMeatadata, sinDataQuant);

    // // Perform FIR filtering
    // std::vector<double> sinDataFir(sinData.size());
    // std::copy(sinData.begin(), sinData.end(), sinDataFir.begin());
    // firReal<W, I, S, Q, O>(sinDataFir, delayFirCofficients_5);
    // writeRealData(fileName_sinDataFir, sinMeatadata, sinDataFir);

    // Perform T/2 delay
    std::vector<double> sinDataDelay_5(sinData.size());
    std::copy(sinData.begin(), sinData.end(), sinDataDelay_5.begin());
    delayReal<W, I, S, Q, O>(sinDataDelay_5, delayFirCofficients_5, 2, 1);
    writeRealData(fileName_sinDataDelay_5, sinMeatadata, sinDataDelay_5);

    // Perform T/4 delay
    std::vector<double> sinDataDelay_25(sinData.size());
    std::copy(sinData.begin(), sinData.end(), sinDataDelay_25.begin());
    delayReal<W, I, S, Q, O>(sinDataDelay_25, delayFirCofficients_25, 4, 3);
    writeRealData(fileName_sinDataDelay_25, sinMeatadata, sinDataDelay_25);

    // interpolatorReal<W, I, S, Q, O>(sinData, polyFirCofficients[0], 2);
    // writeRealData(fileName_sinDataOSR_2,
    //                 sinMeatadata,
    //                 sinData);
    
    // interpolationReal<W, I, S, Q, O>(sinData, polyFirCofficients);
    // writeRealData(fileName_sinDataOSR_8,
    //                 sinMeatadata,
    //                 sinData);


    std::string fileName_sinDataComplex         = "./data/input/sinDataComplex.txt";
    std::string fileName_sinDataComplexQuant    = "./data/output/sinDataComplex_quant.txt";
    std::string fileName_sinDataComplexFir      = "./data/output/sinDataComplex_fir.txt";
    std::string fileName_sinDataComplexDelay_5  = "./data/output/sinDataComplex_delay_5.txt";
    std::string fileName_sinDataComplexDelay_25 = "./data/output/sinDataComplex_delay_25.txt";
    std::string fileName_sinDataComplexOSR_2    = "./data/output/sinDataComplex_OSR_2.txt";
    std::string fileName_sinDataComplexOSR_8    = "./data/output/sinDataComplex_OSR_8.txt";

    // Load input complex sin data
    std::vector<std::complex<double>>               sinDataComplex;
    std::unordered_map<std::string, std::string>    sinComplexMeatadata;
    readComplexData(fileName_sinDataComplex, sinComplexMeatadata, sinDataComplex);

    // // // Quantize complex sin data and write it to file
    // std::vector<std::complex<double>> sinDataComplexQuant;
    // sinDataComplexQuant.reserve(sinDataComplex.size());
    // quantizeComplex<W, I, S, Q, O>(sinDataComplex, sinDataComplexQuant);
    // writeComplexData(fileName_sinDataComplexQuant, sinComplexMeatadata, sinDataComplexQuant);

    // // Perform FIR filtering
    // std::vector<std::complex<double>> sinDataComplexFir(sinDataComplex.size());
    // std::copy(sinDataComplex.begin(), sinDataComplex.end(), sinDataComplexFir.begin());
    // firComplex<W, I, S, Q, O>(sinDataComplexFir, delayFirCofficients_5);
    // writeComplexData(fileName_sinDataComplexFir, sinComplexMeatadata, sinDataComplexFir);

    // Perform T/2 delay
    std::vector<std::complex<double>> sinDataComplexDelay_5(sinDataComplex.size());
    std::copy(sinDataComplex.begin(), sinDataComplex.end(), sinDataComplexDelay_5.begin());
    delayComplex<W, I, S, Q, O>(sinDataComplexDelay_5, delayFirCofficients_5, 2, 1);
    writeComplexData(fileName_sinDataComplexDelay_5, sinComplexMeatadata, sinDataComplexDelay_5);

    // Perform T/4 delay
    std::vector<std::complex<double>> sinDataComplexDelay_25(sinDataComplex.size());
    std::copy(sinDataComplex.begin(), sinDataComplex.end(), sinDataComplexDelay_25.begin());
    delayComplex<W, I, S, Q, O>(sinDataComplexDelay_25, delayFirCofficients_25, 4, 3);
    writeComplexData(fileName_sinDataComplexDelay_25, sinComplexMeatadata, sinDataComplexDelay_25);


    // interpolatorComplex<W, I, S, Q, O>(sinDataComplex, polyFirCofficients[0], 2);
    // writeComplexData(fileName_sinDataComplexOSR_2,
    //                 sinComplexMeatadata,
    //                 sinDataComplex);

    // interpolationComplex<W, I, S, Q, O>(sinDataComplex, polyFirCofficients);
    // writeComplexData(fileName_sinDataComplexOSR_8,
    //                 sinComplexMeatadata,
    //                 sinDataComplex);

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