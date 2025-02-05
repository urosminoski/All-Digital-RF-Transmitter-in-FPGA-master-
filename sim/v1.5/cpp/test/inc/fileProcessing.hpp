#ifndef FILE_PROCESSING_HPP
#define FILE_PROCESSING_HPP

#include <fstream>
#include <string>
#include <vector>
#include <complex>
#include <unordered_map>
#include <stdexcept>
#include <sstream>

// Function declarations for reading

void readMetadata(std::ifstream& inputFile, std::unordered_map<std::string, std::string>& metadata);

void readRealData(const std::string& filename, 
                  std::unordered_map<std::string, std::string>& metadata, 
                  std::vector<double>& realData);

void readReal2dData(const std::string& filename, 
                    std::unordered_map<std::string, std::string>& metadata, 
                    std::vector<std::vector<double>>& real2DData);

void readComplexData(const std::string& filename, 
                     std::unordered_map<std::string, std::string>& metadata, 
                     std::vector<std::complex<double>>& complexData);

// Function declarations for writing
void writeMetadata(std::ofstream& outputFile, const std::unordered_map<std::string, std::string>& metadata);

void writeRealData(const std::string& filename, 
                   const std::unordered_map<std::string, std::string>& metadata, 
                   const std::vector<double>& realData);

void writeReal2dData(const std::string& filename, 
                     const std::unordered_map<std::string, std::string>& metadata, 
                     const std::vector<std::vector<double>>& real2DData);

void writeComplexData(const std::string& filename, 
                      const std::unordered_map<std::string, std::string>& metadata, 
                      const std::vector<std::complex<double>>& complexData);

#endif // FILE_PROCESSING_HPP
