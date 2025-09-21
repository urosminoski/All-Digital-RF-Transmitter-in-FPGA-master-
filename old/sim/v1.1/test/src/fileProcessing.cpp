#include "fileProcessing.hpp"

// Helper function to trim whitespace from a string
static void trim(std::string& str) {
    str.erase(0, str.find_first_not_of(" \t"));
    str.erase(str.find_last_not_of(" \t") + 1);
}

// Function to read metadata
void readMetadata(std::ifstream& inputFile, std::unordered_map<std::string, std::string>& metadata) {
    std::string line;
    while (std::getline(inputFile, line)) {
        trim(line);

        if (line.empty()) {
            continue;
        }

        // Process metadata (lines starting with #)
        if (line[0] == '#') {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(1, delimiterPos - 1); // Extract key (skip #)
                std::string value = line.substr(delimiterPos + 1);  // Extract value
                trim(key);
                trim(value);
                metadata[key] = value;
            } else {
                throw std::runtime_error("Malformed metadata line: " + line);
            }
        } else {
            // Reached data, stop reading metadata
            inputFile.clear();
            inputFile.seekg(-static_cast<std::streamoff>(line.size() + 1), std::ios_base::cur);
            break;
        }
    }
}

// Function to read real data
void readRealData(const std::string& filename, 
                  std::unordered_map<std::string, std::string>& metadata, 
                  std::vector<double>& realData) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    readMetadata(inputFile, metadata);

    std::string line;
    while (std::getline(inputFile, line)) {
        trim(line);
        if (line.empty()) {
            continue;
        }

        try {
            realData.push_back(std::stod(line));
        } catch (const std::exception&) {
            throw std::runtime_error("Malformed real data line: " + line);
        }
    }

    inputFile.close();
}

// Function to read real 2D data
void readReal2dData(const std::string& filename, 
                    std::unordered_map<std::string, std::string>& metadata, 
                    std::vector<std::vector<double>>& real2DData) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    readMetadata(inputFile, metadata);

    std::string line;
    while (std::getline(inputFile, line)) {
        trim(line);
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::vector<double> row;
        double value;
        while (iss >> value) {
            row.push_back(value);
        }

        if (!row.empty()) {
            real2DData.push_back(row);
        } else {
            throw std::runtime_error("Malformed 2D real data line: " + line);
        }
    }

    inputFile.close();
}

// Function to read complex data
void readComplexData(const std::string& filename, 
                     std::unordered_map<std::string, std::string>& metadata, 
                     std::vector<std::complex<double>>& complexData) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    readMetadata(inputFile, metadata);

    std::string line;
    while (std::getline(inputFile, line)) {
        trim(line);
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        double realPart, imagPart;
        if (iss >> realPart >> imagPart) {
            complexData.emplace_back(realPart, imagPart);
        } else {
            throw std::runtime_error("Malformed complex data line: " + line);
        }
    }

    inputFile.close();
}


// Function to write metadata to a file
void writeMetadata(std::ofstream& outputFile, const std::unordered_map<std::string, std::string>& metadata) {
    for (const auto& [key, value] : metadata) {
        outputFile << "#" << key << "=" << value << "\n";
    }
}

// Function to write real data to a file
void writeRealData(const std::string& filename, 
                   const std::unordered_map<std::string, std::string>& metadata, 
                   const std::vector<double>& realData) {
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Step 1: Write metadata
    writeMetadata(outputFile, metadata);

    // Step 2: Write real data
    for (const double& value : realData) {
        outputFile << value << "\n";
    }

    // std::cout << "File written to " << filename << "\n";
    outputFile.close();
}

// Function to write real 2D data to a file
void writeReal2dData(const std::string& filename, 
                     const std::unordered_map<std::string, std::string>& metadata, 
                     const std::vector<std::vector<double>>& real2DData) {
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Step 1: Write metadata
    writeMetadata(outputFile, metadata);

    // Step 2: Write 2D real data
    for (const auto& row : real2DData) {
        for (size_t i = 0; i < row.size(); ++i) {
            outputFile << row[i];
            if (i < row.size() - 1) {
                outputFile << " "; // Separate values in the same row with a space
            }
        }
        outputFile << "\n";
    }

    outputFile.close();
}

// Function to write complex data to a file
void writeComplexData(const std::string& filename, 
                      const std::unordered_map<std::string, std::string>& metadata, 
                      const std::vector<std::complex<double>>& complexData) {
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Step 1: Write metadata
    writeMetadata(outputFile, metadata);

    // Step 2: Write complex data
    for (const auto& value : complexData) {
        outputFile << value.real() << " " << value.imag() << "\n"; // Write real and imaginary parts
    }

    outputFile.close();
}
