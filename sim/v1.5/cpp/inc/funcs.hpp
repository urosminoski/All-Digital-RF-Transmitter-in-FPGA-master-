#ifndef FUNCS_HPP
#define FUNCS_HPP

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <variant>
#include <complex>
#include <regex>
#include <nlohmann/json.hpp>
#include <ac_fixed.h>

#define BITS_NUM    ( 4 )

typedef ac_fixed<32, 8, true, AC_RND, AC_SAT> fxFIR;

extern std::vector<double> firCoeff;

/**
 * @brief Reads a LUT from a JSON file.
 *
 * @param fileName The path to the JSON file.
 * @param lut The 2D vector to store the LUT data.
 */
bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& lut);

/**
 * @brief Reads data from a file into a vector.
 *
 * @param fileName The path to the file to read.
 * @param data The vector to store the read data.
 * @return True if the file is read successfully, otherwise false.
 */
bool readFromFileComplex(const std::string& fileName,
                         std::vector<std::complex<double>>& data,
                         std::map<std::string, double>& metadata);

/**
 * @brief Writes data from a vector to a file.
 *
 * @param fileName The path to the file to write.
 * @param data The vector containing data to write.
 * @return True if the file is written successfully, otherwise false.
 */
bool writeToFileComplex(const std::string& fileName, 
                        const std::vector<std::complex<double>>& data,
                        const std::map<std::string, double>& metadata);

bool readFromFile(const std::string& fileName,
                  std::variant<std::vector<double>, std::vector<std::complex<double>>>& data,
                  std::map<std::string, double>& metadata);

bool writeToFile(const std::string& fileName,
                 const std::variant<std::vector<double>, std::vector<std::complex<double>>>& data,
                 const std::map<std::string, double>& metadata);

void firComplex(std::vector<std::complex<double>>& input, std::vector<double>& firCoeff, std::vector<std::complex<double>>& output);
void fir(std::vector<double>& x, std::vector<double>& firCoeff, std::vector<double>& y);

void deltaSigmaComplex(const std::vector<std::complex<double>>& input, 
                       std::vector<std::complex<int>>& output);

/**
 * @brief Processes a vector of double values using a delta-sigma modulation algorithm.
 *
 * @param x The input vector of doubles to be processed.
 * @param y The output vector to store the processed values.
 */
void deltaSigma(std::vector<double>& x, std::vector<int>& y);

void parallelToSerialConverterComplex(const std::vector<std::complex<int>>& inputSignal,
                                      const std::vector<std::vector<int>>& LUT,
                                      std::vector<std::complex<int>>& outputSignal);

/**
 * @brief Converts parallel data to serial data using a LUT.
 *
 * @param inputSignal The input vector of signal values.
 * @param LUT The lookup table as a 2D vector.
 * @param outputSignal The output vector to store the converted serial data.
 */
void parallelToSerialConverter(const std::vector<int>& inputSignal,
                               const std::vector<std::vector<int>>& LUT,
                               std::vector<int>& outputSignal);

#endif // FUNCS_HPP
