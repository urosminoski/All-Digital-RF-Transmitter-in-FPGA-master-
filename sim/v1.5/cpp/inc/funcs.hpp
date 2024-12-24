#ifndef PARALLEL_SERIAL_UTILS_HPP
#define PARALLEL_SERIAL_UTILS_HPP

#include <vector>
#include <variant>
#include <map>
#include <string>
#include <fstream>
#include <complex>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ac_fixed.h>

using json = nlohmann::json;

// FIR Coefficients (Global Definition)
extern std::vector<double> firCoeff;

// Function Declarations

/**
 * @brief Reads data and metadata from a file.
 * 
 * @param fileName The name of the input file to read.
 * @param data A variant that stores either:
 *             - `std::vector<double>`: Real data.
 *             - `std::vector<std::complex<double>>`: Complex data.
 * @param metadata A map to store metadata key-value pairs extracted from the file.
 * 
 * @return True if the file is successfully read and parsed; false otherwise.
 */
bool readFromFile(const std::string& fileName,
                  std::variant<std::vector<double>, std::vector<std::complex<double>>>& data,
                  std::map<std::string, double>& metadata);

/**
 * @brief Writes data and metadata to a file.
 * 
 * @param fileName The name of the output file to write.
 * @param data A variant containing either:
 *             - `std::vector<double>`: Real data.
 *             - `std::vector<std::complex<double>>`: Complex data.
 * @param metadata A map containing metadata key-value pairs to be written.
 * 
 * @return True if the file is successfully written; false otherwise.
 */
bool writeToFile(const std::string& fileName,
                 const std::variant<std::vector<double>, std::vector<std::complex<double>>>& data,
                 const std::map<std::string, double>& metadata);

/**
 * @brief Reads a lookup table (LUT) from a JSON file.
 * 
 * @param fileName The name of the JSON file to read.
 * @param LUT A reference to a 2D vector of integers to store the LUT.
 * 
 * @return True if the LUT is successfully read; false otherwise.
 */
bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT);

/**
 * @brief Applies an FIR filter to real or complex data.
 * 
 * @param input A variant containing either:
 *              - `std::vector<double>`: Real data to be filtered.
 *              - `std::vector<std::complex<double>>`: Complex data to be filtered.
 * @param firCoeff A vector of FIR filter coefficients.
 * @param output A variant that stores the filtered output. It will contain:
 *               - `std::vector<double>`: Filtered real data.
 *               - `std::vector<std::complex<double>>`: Filtered complex data.
 */
void firWrapper(const std::variant<std::vector<double>, std::vector<std::complex<double>>>& input,
                const std::vector<double>& firCoeff,
                std::variant<std::vector<double>, std::vector<std::complex<double>>>& output);


/**
 * @brief Wrapper function for Delta-Sigma modulation.
 * 
 * @param input A variant containing either:
 *              - `std::vector<double>`: Real data to be processed.
 *              - `std::vector<std::complex<double>>`: Complex data to be processed.
 * @param output A variant that stores the processed result. It will contain:
 *               - `std::vector<double>`: Processed real data.
 *               - `std::vector<std::complex<double>>`: Processed complex data.
 */
void deltaSigmaWrapper(const std::variant<std::vector<double>, std::vector<std::complex<double>>>& input,
                       std::variant<std::vector<double>, std::vector<std::complex<double>>>& output);

/**
 * @brief Wrapper function for parallel-to-serial conversion of real or complex data.
 * 
 * @param input A variant containing either:
 *              - `std::vector<double>`: Real data to be processed.
 *              - `std::vector<std::complex<double>>`: Complex data to be processed.
 * @param LUT A lookup table for parallel-to-serial conversion.
 * @param output A variant that stores the converted output. It will contain:
 *               - `std::vector<double>`: Converted real data.
 *               - `std::vector<std::complex<double>>`: Converted complex data.
 */
void parallelToSerialConverterWrapper(const std::variant<std::vector<double>, std::vector<std::complex<double>>>& input,
                                      const std::vector<std::vector<int>>& LUT,
                                      std::variant<std::vector<double>, std::vector<std::complex<double>>>& output);


#endif // PARALLEL_SERIAL_UTILS_HPP
