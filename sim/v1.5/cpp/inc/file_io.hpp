#ifndef FILE_IO_HPP
#define FILE_IO_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Reads data from a file and stores it in the provided vector.
// Returns true if successful, false otherwise.
bool readFromFile(const std::string& fileName, std::vector<double>& data);

// Writes data from a vector to a file.
// Returns true if successful, false otherwise.
bool writeToFile(const std::string& fileName, const std::vector<double>& data);

#endif // FILE_IO_HPP
