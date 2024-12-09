#ifndef FUNCS_HPP
#define FUNCS_HPP

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include <ac_fixed.h>

#define BITS_NUM    ( 4 )

/**
 * @brief Reads a LUT from a JSON file.
 *
 * @param fileName The path to the JSON file.
 * @param lut The 2D vector to store the LUT data.
 */
void readLUT(const std::string& fileName, std::vector<std::vector<int>>& lut);

/**
 * @brief Reads data from a file into a vector.
 *
 * @param fileName The path to the file to read.
 * @param data The vector to store the read data.
 * @return True if the file is read successfully, otherwise false.
 */
bool readFromFile(const std::string& fileName, 
                  std::vector<double>& data,
                  std::map<std::string, double>& metadata);

/**
 * @brief Writes data from a vector to a file.
 *
 * @param fileName The path to the file to write.
 * @param data The vector containing data to write.
 * @return True if the file is written successfully, otherwise false.
 */
bool writeToFile(const std::string& fileName, 
                 const std::vector<int>& data,
                 const std::map<std::string, double>& metadata);

/**
 * @brief Processes a vector of double values using a delta-sigma modulation algorithm.
 *
 * @param x The input vector of doubles to be processed.
 * @param y The output vector to store the processed values.
 */
void deltaSigma(std::vector<double>& x, std::vector<int>& y);

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
