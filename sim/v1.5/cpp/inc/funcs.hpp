#ifndef FUNCS_HPP
#define FUNCS_HPP

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <ac_fixed.h>

/**
 * @brief Reads a LUT from a JSON file.
 *
 * @param fileName The path to the JSON file.
 * @return A 2D vector of integers representing the LUT.
 */
std::vector<std::vector<int>> readLUT(const std::string& fileName);

/**
 * @brief Reads data from a file into a vector.
 *
 * @param fileName The path to the file to read.
 * @param data The vector to store the read data.
 * @return True if the file is read successfully, otherwise false.
 */
bool readFromFile(const std::string& fileName, std::vector<double>& data);

/**
 * @brief Writes data from a vector to a file.
 *
 * @param fileName The path to the file to write.
 * @param data The vector containing data to write.
 * @return True if the file is written successfully, otherwise false.
 */
bool writeToFile(const std::string& fileName, const std::vector<double>& data);

/**
 * @brief Processes a vector of double values using a delta-sigma modulation algorithm.
 *
 * @param x The input vector of doubles to be processed.
 * @param y The output vector to store the processed values.
 */
void deltaSigma(std::vector<double>& x, std::vector<double>& y);

/**
 * @brief Converts parallel data to serial data.
 *
 * Placeholder for the actual implementation.
 */
void parallelToSerialConverter();

#endif // FUNCS_HPP
