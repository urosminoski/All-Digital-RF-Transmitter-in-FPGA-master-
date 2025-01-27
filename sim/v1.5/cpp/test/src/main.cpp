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
    // std::string fileName_interpolate_firs   = "../../data/input/polyFirCofficients_60dB.txt"; 

    // Load FIR coeffitients for polyphase-interpolation
    std::string fileName_polyFirCofficients   = "./data/input/polyFirCofficients_80dB.txt"; 
    std::vector<std::vector<double>>                polyFirCofficients;
    std::unordered_map<std::string, std::string>    polyFirMetadata;
    readReal2dData(fileName_polyFirCofficients,
                   polyFirMetadata,
                   polyFirCofficients);

    // Load input real sin data
    std::string fileName_sinData   = "./data/input/sinData.txt";
    std::vector<double>                             sinData;
    std::unordered_map<std::string, std::string>    sinMeatadata;
    readRealData(fileName_sinData,
                 sinMeatadata,
                 sinData);

    // Quantize real sin data and write it to file
    std::vector<double> sinDataQuant;
    sinDataQuant.reserve(sinData.size());
    quantizeReal<W, I, S, Q, O>(sinData, sinDataQuant);
    std::string fileName_sinDataQuant   = "./data/output/sinData_quant.txt";
    writeRealData(fileName_sinDataQuant,
                  sinMeatadata,
                  sinDataQuant);
    // Perform FIR filtering
    std::vector<double> sinDataFir;
    firReal<W, I, S, Q, O>(sinData, sinDataFir, polyFirCofficients[0]);
    std::string fileName_sinDataFir  = "./data/output/sinData_fir.txt";
    writeRealData(fileName_sinDataFir,
                    sinMeatadata,
                    sinDataFir);

    interpolatorReal<W, I, S, Q, O>(sinData, polyFirCofficients[0], 2);
    std::string fileName_sinDataOSR_2  = "./data/output/sinData_OSR_2.txt";
    writeRealData(fileName_sinDataOSR_2,
                    sinMeatadata,
                    sinData);
    
    interpolationReal<W, I, S, Q, O>(sinData, polyFirCofficients);
    std::string fileName_sinDataOSR_8  = "./data/output/sinData_OSR_8.txt";
    writeRealData(fileName_sinDataOSR_8,
                    sinMeatadata,
                    sinData);




    // Load input complex sin data
    std::string fileName_sinDataComplex   = "./data/input/sinDataComplex.txt";
    std::vector<std::complex<double>>                             sinDataComplex;
    std::unordered_map<std::string, std::string>    sinComplexMeatadata;
    readComplexData(fileName_sinDataComplex,
                    sinComplexMeatadata,
                    sinDataComplex);

    // // Quantize complex sin data and write it to file
    std::vector<std::complex<double>> sinDataComplexQuant;
    sinDataComplexQuant.reserve(sinDataComplex.size());
    quantizeComplex<W, I, S, Q, O>(sinDataComplex, sinDataComplexQuant);
    std::string fileName_sinDataComplexQuant   = "./data/output/sinDataComplex_quant.txt";
    writeComplexData(fileName_sinDataComplexQuant,
                    sinComplexMeatadata,
                    sinDataComplexQuant);

    // // Perform FIR filtering
    std::vector<std::complex<double>> sinDataComplexFir;
    firComplex<W, I, S, Q, O>(sinDataComplex, sinDataComplexFir, polyFirCofficients[0]);
    std::string fileName_sinDataComplexFir  = "./data/output/sinDataComplex_fir.txt";
    writeComplexData(fileName_sinDataComplexFir,
                    sinComplexMeatadata,
                    sinDataComplexFir);


    interpolatorComplex<W, I, S, Q, O>(sinDataComplex, polyFirCofficients[0], 2);
    std::string fileName_sinDataComplexOSR_2  = "./data/output/sinDataComplex_OSR_2.txt";
    writeComplexData(fileName_sinDataComplexOSR_2,
                    sinComplexMeatadata,
                    sinDataComplex);

    interpolationComplex<W, I, S, Q, O>(sinDataComplex, polyFirCofficients);
    std::string fileName_sinDataComplexOSR_8 = "./data/output/sinDataComplex_OSR_8.txt";
    writeComplexData(fileName_sinDataComplexOSR_8,
                    sinComplexMeatadata,
                    sinDataComplex);

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