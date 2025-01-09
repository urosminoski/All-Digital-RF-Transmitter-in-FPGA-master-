#include <iostream>
#include <stdexcept>
#include <complex>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <type_traits>
#include <nlohmann/json.hpp>
#include <ac_fixed.h>
#include <ac_complex.h>

#include <fstream>
#include <sstream>
#include <string>

// Type aliases
using RealVector = std::vector<double>;
using ComplexVector = std::vector<std::complex<double>>;
using RealMatrix = std::vector<std::vector<double>>;
using ComplexMatrix = std::vector<std::vector<std::complex<double>>>;

using FxpVector = std::variant<RealVector, ComplexVector>;
using FxpMatrix = std::variant<RealMatrix, ComplexMatrix>;

// Function prototypes
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void discretizeVector(FxpVector& inputData);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void discretizeMatrix(FxpMatrix& inputData);

void printVector(const FxpVector& inputData);
void printMatrix(const FxpMatrix& inputData);
static void readMetadata(std::ifstream& inputFile, std::map<std::string, double>& metadata);
static void parseDataLine(const std::string& line, bool isComplex,
                            std::vector<double>& realData,
                            std::vector<std::complex<double>>& complexData);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void readVector(const std::string& inputFileName, 
                FxpVector& inputData,
                std::map<std::string, double>& metadata);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void readMatrix(const std::string& inputFileName, 
                FxpMatrix& inputData,
                std::map<std::string, double>& metadata);

int main() {
    constexpr int W = 12;
    constexpr int I = 4;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;

    // // Initialize a real matrix
    // FxpMatrix realMatrix_1 = RealMatrix{
    //     {1.23124322, -2.33432143, 0.32423411},
    //     {3.14159265, 2.71828183, -1.61803399}
    // };
    // discretizeMatrix<W, I, S, Q, O>(realMatrix_1);
    // printMatrix(realMatrix_1);

    // // Initialize a complex matrix
    // FxpMatrix complexMatrix_1 = ComplexMatrix{
    //     {{1.2123123, -0.323414}, {2.312342313, 3.4653424}, {-1.1346521341, 2.2243522}},
    //     {{3.14123423, 0.921349}, {2.72134211, -3.312343}, {1.62342351, 0.72352343}}
    // };
    // discretizeMatrix<W, I, S, Q, O>(complexMatrix_1);
    // printMatrix(complexMatrix_1);

    // // Initialize a real vector
    // FxpVector realVector_1 = RealVector{1.23124322, -2.33432143, 0.32423411};
    // discretizeVector<W, I, S, Q, O>(realVector_1);
    // printVector(realVector_1);

    // // Initialize a complex vector
    // FxpVector complexVector_1 = ComplexVector{
    //     {1.2123123, -0.323414}, {2.312342313, 3.4653424}, {-1.1346521341, 2.2243522}
    // };
    // discretizeVector<W, I, S, Q, O>(complexVector_1);
    // printVector(complexVector_1);

    std::string file_path_in_real = "./data/input/sinData_2.txt";
    FxpVector realVector;
    std::map<std::string, double> metadata1;
    readVector<W, I, S, Q, O>(file_path_in_real, realVector, metadata1);
    printVector(realVector);
    std::string file_path_in_complex = "./data/input/sinDataComplex_2.txt";
    FxpVector complexVector;
    std::map<std::string, double> metadata2;
    readVector<W, I, S, Q, O>(file_path_in_complex, complexVector, metadata2);
    printVector(complexVector);

    std::string file_path_in_real_m = "./data/input/matrixData_2.txt";
    FxpMatrix realMatrix;
    std::map<std::string, double> metadata3;
    readMatrix<W, I, S, Q, O>(file_path_in_real_m, realMatrix, metadata3);
    printMatrix(realMatrix);
    std::string file_path_in_complex_m = "./data/input/matrixDataComplex_2.txt";
    FxpMatrix complexMatrix;
    std::map<std::string, double> metadata4;
    readMatrix<W, I, S, Q, O>(file_path_in_complex_m, complexMatrix, metadata4);
    printMatrix(complexMatrix);

    return 0;
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void discretizeVector(FxpVector& inputData) {
    std::visit([](auto& data) {
        for (auto& value : data) {
            if constexpr (std::is_same_v<std::decay_t<decltype(data)>, RealVector>) {
                value = ac_fixed<W, I, S, Q, O>(value).to_double();
            } else if constexpr (std::is_same_v<std::decay_t<decltype(data)>, ComplexVector>) {
                value = std::complex<double>(
                    ac_fixed<W, I, S, Q, O>(value.real()).to_double(),
                    ac_fixed<W, I, S, Q, O>(value.imag()).to_double()
                );
            } else {
                throw std::runtime_error("Invalid input data type for discretizeVector!");
            }
        }
    }, inputData);
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void discretizeMatrix(FxpMatrix& inputData) {
    std::visit([](auto& data) {
        for (auto& row : data) {
            for (auto& value : row) {
                if constexpr (std::is_same_v<std::decay_t<decltype(data)>, RealMatrix>) {
                    value = ac_fixed<W, I, S, Q, O>(value).to_double();
                } else if constexpr (std::is_same_v<std::decay_t<decltype(data)>, ComplexMatrix>) {
                    value = std::complex<double>(
                        ac_fixed<W, I, S, Q, O>(value.real()).to_double(),
                        ac_fixed<W, I, S, Q, O>(value.imag()).to_double()
                    );
                } else {
                    throw std::runtime_error("Invalid input data type for discretizeMatrix!");
                }
            }
        }
    }, inputData);
}

void printVector(const FxpVector& inputData) {
    std::visit([](const auto& data) {
        if constexpr (std::is_same_v<std::decay_t<decltype(data)>, RealVector>) {
            std::cout << "\nReal Vector:\n";
            for (const auto& value : data) {
                std::cout << value << " ";
            }
        } else if constexpr (std::is_same_v<std::decay_t<decltype(data)>, ComplexVector>) {
            std::cout << "\nComplex Vector:\n";
            for (const auto& value : data) {
                std::cout << "(" << value.real() << ", " << value.imag() << ") ";
            }
        } else {
            throw std::runtime_error("Invalid input data type for printVector!");
        }
        std::cout << "\n";
    }, inputData);
}

void printMatrix(const FxpMatrix& inputData) {
    std::visit([](const auto& data) {
        if constexpr (std::is_same_v<std::decay_t<decltype(data)>, RealMatrix>) {
            std::cout << "\nReal Matrix:\n";
            for (const auto& row : data) {
                for (const auto& value : row) {
                    std::cout << value << " ";
                }
                std::cout << "\n";
            }
        } else if constexpr (std::is_same_v<std::decay_t<decltype(data)>, ComplexMatrix>) {
            std::cout << "\nComplex Matrix:\n";
            for (const auto& row : data) {
                for (const auto& value : row) {
                    std::cout << "(" << value.real() << ", " << value.imag() << ") ";
                }
                std::cout << "\n";
            }
        } else {
            throw std::runtime_error("Invalid input data type for printMatrix!");
        }
    }, inputData);
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void readVector(const std::string& inputFileName, 
                FxpVector& inputData,
                std::map<std::string, double>& metadata) {
    // Open the input file
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Cannot open file " + inputFileName);
    }

    metadata.clear();
    std::string line;
    bool isComplex = false;
    bool firstDataLine = true;
    std::vector<double> realData;
    std::vector<std::complex<double>> complexData;

    // Read metadata
    readMetadata(inputFile, metadata);

    // Read data lines
    while (std::getline(inputFile, line)) {
        if (line.empty()) continue; // Skip empty lines

        // Detect if the data is complex or real
        if (firstDataLine) {
            std::istringstream iss(line);
            int count = 0;
            double tmp;
            while (iss >> tmp) count++;

            if (count == 2) {
                isComplex = true;
            } else if (count == 1) {
                isComplex = false;
            } else {
                throw std::runtime_error("Invalid data format in line: " + line);
            }
            firstDataLine = false;
        }

        // Parse the line into real or complex data
        parseDataLine(line, isComplex, realData, complexData);
    }

    // Process and store data in inputData
    if (isComplex) {
        ComplexVector processedData;
        for (const auto& c : complexData) {
            processedData.emplace_back(
                ac_fixed<W, I, S, Q, O>(c.real()).to_double(),
                ac_fixed<W, I, S, Q, O>(c.imag()).to_double()
            );
        }
        inputData = std::move(processedData);
    } else {
        RealVector processedData;
        for (const auto& r : realData) {
            processedData.emplace_back(ac_fixed<W, I, S, Q, O>(r).to_double());
        }
        inputData = std::move(processedData);
    }

    inputFile.close();
}

// Does not
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void readMatrix(const std::string& inputFileName, 
                FxpMatrix& inputData,
                std::map<std::string, double>& metadata) {
    // Open the input file
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Cannot open file " + inputFileName);
    }

    metadata.clear();
    std::string line;
    bool isComplex = false;
    bool firstDataLine = true;
    std::vector<std::vector<double>> realData;
    std::vector<std::vector<std::complex<double>>> complexData;

    // Read metadata
    readMetadata(inputFile, metadata);

    // Read data lines
    while (std::getline(inputFile, line)) {
        if (line.empty()) continue; // Skip empty lines

        std::vector<double> rowRealData;
        std::vector<std::complex<double>> rowComplexData;

        // Detect if the data is complex or real
        if (firstDataLine) {
            std::istringstream iss(line);
            int count = 0;
            double tmp;
            while (iss >> tmp) count++;

            if (count % 2 == 0) {
                isComplex = true;
            } else {
                isComplex = false;
            }
            firstDataLine = false;
        }

        // Parse the line into real or complex data
        parseDataLine(line, isComplex, rowRealData, rowComplexData);

        if (isComplex) {
            complexData.push_back(std::move(rowComplexData));
        } else {
            realData.push_back(std::move(rowRealData));
        }
    }

    // Process and store data in inputData
    if (isComplex) {
        ComplexMatrix processedData;
        for (const auto& row : complexData) {
            std::vector<std::complex<double>> processedRow;
            for (const auto& c : row) {
                processedRow.emplace_back(
                    ac_fixed<W, I, S, Q, O>(c.real()).to_double(),
                    ac_fixed<W, I, S, Q, O>(c.imag()).to_double()
                );
            }
            processedData.push_back(std::move(processedRow));
        }
        inputData = std::move(processedData);
    } else {
        RealMatrix processedData;
        for (const auto& row : realData) {
            std::vector<double> processedRow;
            for (const auto& r : row) {
                processedRow.emplace_back(ac_fixed<W, I, S, Q, O>(r).to_double());
            }
            processedData.push_back(std::move(processedRow));
        }
        inputData = std::move(processedData);
    }

    inputFile.close();
}


// Helper functions for file reading
static void readMetadata(std::ifstream& inputFile, std::map<std::string, double>& metadata) {
    metadata.clear();
    std::string line;

    while (std::getline(inputFile, line)) {
        if (line.empty() || line[0] != '#') {
            inputFile.seekg(-static_cast<int>(line.size()) - 1, std::ios::cur);
            break;
        }

        size_t delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(1, delimiter_pos - 1);
            std::string value_str = line.substr(delimiter_pos + 1);
            try {
                metadata[key] = std::stod(value_str);
            } catch (const std::invalid_argument&) {
                throw std::runtime_error("Invalid value in metadata: " + value_str);
            }
        } else {
            throw std::runtime_error("Malformed metadata line: " + line);
        }
    }
}

static void parseDataLine(const std::string& line, bool isComplex,
                            std::vector<double>& realData,
                            std::vector<std::complex<double>>& complexData) {
    const char* str = line.c_str();
    char* end;
    std::vector<double> values;

    while (*str != '\0') {
        double value = std::strtod(str, &end);

        if (str == end) {
            throw std::runtime_error("Failed to parse value in line: " + line);
        }

        values.push_back(value);
        str = end;
        while (*str == ' ') ++str;
    }

    if (isComplex && values.size() == 2) {
        complexData.emplace_back(values[0], values[1]);
    } else if (!isComplex && values.size() == 1) {
        realData.push_back(values[0]);
    } else {
        throw std::runtime_error("Inconsistent data format in line: " + line);
    }
}

template<int W, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class WrapperFixed {
    int I;
    ac_fixed<W, 1, S, Q, O> fxpValue;
}

void deltaSigma(FxpVector& inputData,
                FxpVector& outputData,
                FxpMatrix& ) {

}