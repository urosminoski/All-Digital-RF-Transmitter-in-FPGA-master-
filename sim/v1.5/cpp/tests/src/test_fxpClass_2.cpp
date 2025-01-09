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

template<int W, int I, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FxpDsp {
private:
    using RealVector    = std::vector<double>;
    using ComplexVector = std::vector<std::complex<double>>;

    std::variant<RealVector, ComplexVector>     fxpVector;
    std::map<std::string, double>               metadata;
    bool                                        qFormat;

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

public:
    // Default constructor
    FxpDsp() 
        : fxpVector(RealVector{}), metadata{}, qFormat(false) {}
    
    // Real data constructor
    FxpDsp(const RealVector& inVector,
           const std::map<std::string, double>& meta = {},
           bool qFmt = false)
        : fxpVector(RealVector{}), metadata(meta), qFormat(qFmt) {
        auto& tmpVector = std::get<RealVector>(fxpVector);
        tmpVector.reserve(inVector.size());
        for (const auto& value : inVector) {
            tmpVector.emplace_back(
                ac_fixed<W, I, S, Q, O>(value).to_double()
            );
        }
    }

    // Complex data constructor
    FxpDsp(const ComplexVector& inVector,
           const std::map<std::string, double>& meta = {},
           bool qFmt = false)
        : fxpVector(ComplexVector{}), metadata(meta), qFormat(qFmt) {
        auto& tmpVector = std::get<ComplexVector>(fxpVector);
        tmpVector.reserve(inVector.size());
        for (const auto& value : inVector) {
            tmpVector.emplace_back(
                ac_fixed<W, I, S, Q, O>(value.real()).to_double(),
                ac_fixed<W, I, S, Q, O>(value.imag()).to_double()
            );
        }
    }

    // Getter for real data
    double getRealValue(const size_t index) {
        auto* vectorPtr = std::get_if<RealVector>(&fxpVector);
        if (!vectorPtr) {
            throw std::runtime_error("Vector does not contain real data!");
        }
        if (index >= vectorPtr->size()) {
            throw std::runtime_error("Index out of bounds!");
        }
        return (*vectorPtr)[index];
    }

    // Getter for complex data
    std::complex<double> getComplexValue(const size_t index) {
        auto* vectorPtr = std::get_if<ComplexVector>(&fxpVector);
        if (!vectorPtr) {
            throw std::runtime_error("Vector does not contain complex data!");
        }
        if (index >= vectorPtr->size()) {
            throw std::runtime_error("Index out of bounds!");
        }
        return (*vectorPtr)[index];
    }

    size_t getSize() {
        return std::visit([](const auto& vector){
            return vector.size();
        }, fxpVector);
    }

    void print() const {
        std::visit([](const auto& vector) {
            std::cout << "\n";
            for (const auto& value : vector) {
                using T = typename std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, double>) {
                    std::cout << value << "\n";
                } else if constexpr (std::is_same_v<T, std::complex<double>>) {
                    std::cout << "(" << value.real() << ", " << value.imag() << ")\n";
                } else {
                    throw std::runtime_error("Invalid data type for ptint()!");
                }
            }
        }, fxpVector);
    }

    void printMetadata() const {
        std::cout << "\n";
        for (const auto& [key, value] : metadata) {
            std::cout << "#" << key << "=" << value << "\n";
        }
    }

    void readFromFile(const std::string& fileName) {
        std::ifstream inputFile(fileName);
        if (!inputFile.is_open()) {
            throw std::runtime_error("Cannot open file " + fileName);
        }

        // inputFile.precision(std::numeric_limits<double>::digits10 + 1);
        metadata.clear();

        bool isComplex = false;
        bool firstDataLine = true;
        std::vector<double> realData;
        std::vector<std::complex<double>> complexData;
        std::string line;

        readMetadata(inputFile, metadata);

        while (std::getline(inputFile, line)) {
            if (firstDataLine) {
                std::istringstream iss(line);
                int count = 0;
                double tmp;
                while (iss >> tmp) count++;

                if (count == 2) {
                    isComplex = true;
                    fxpVector = ComplexVector();
                } else if (count == 1) {
                    isComplex = false;
                    fxpVector = RealVector();
                } else {
                    throw std::runtime_error("Invalid data format in line: " + line);
                }
                firstDataLine = false;
            }

            parseDataLine(line, isComplex, realData, complexData);
        }

        if (isComplex) {
            auto& tmpVector = std::get<ComplexVector>(fxpVector);
            tmpVector.reserve(complexData.size());
            for (const auto& c : complexData) {
                tmpVector.emplace_back(
                    ac_fixed<W, I, S, Q, O>(c.real()).to_double(),
                    ac_fixed<W, I, S, Q, O>(c.imag()).to_double());
            }
        } else {
            auto& tmpVector = std::get<RealVector>(fxpVector);
            tmpVector.reserve(complexData.size());
            for (const auto& r : realData) {
                tmpVector.emplace_back(ac_fixed<W, I, S, Q, O>(r).to_double());
            }
        }

        inputFile.close();
    }

    void writeToFile(const std::string& fileName) {
        // Open file for writing
        std::ofstream outputFile(fileName);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Error: cannot open file " + fileName + "for writing!");

        }
        // Write metadata
        for (const auto& [key, val] : metadata) {
            outputFile << "#" << key << "=" << val << "\n";
        }
        // Write data
        std::visit([&outputFile](const auto& vector) {
            for (const auto& value : vector) {
                using T = typename std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, double>) {
                    outputFile << value << "\n";
                } else if constexpr (std::is_same_v<T, std::complex<double>>) {
                    outputFile << value.real() << " " << value.imag() << "\n";
                } else {
                    throw std::runtime_error("Error: Unexpected data type during write operation.\n");
                }
            }
        }, fxpVector);
    }

};

int main() {
    using MyFxpDsp = FxpDsp<12, 4, true, AC_RND, AC_SAT>;

    std::string file_path_in_real = "./data/input/sinData_2.txt";
    std::string file_path_out_real = "./data/output/sinData_2.txt";
    std::string file_path_in_complex = "./data/input/sinDataComplex_2.txt";
    std::string file_path_out_complex = "./data/output/sinDataComplex_2.txt";

    // Example usage: Default constructor
    MyFxpDsp fxpVector1;
    fxpVector1.readFromFile(file_path_in_real);
    fxpVector1.writeToFile(file_path_out_real);
    fxpVector1.printMetadata();
    fxpVector1.print();
    MyFxpDsp fxpVector2;
    fxpVector2.readFromFile(file_path_in_complex);
    fxpVector2.writeToFile(file_path_out_complex);
    fxpVector2.printMetadata();
    fxpVector2.print();

    // // Example usage: Real data constructor
    // std::vector<double> realData = {1.78853123, -2.9231232, 3.235463542353};
    // MyFxpDsp realFxp(realData, {{"scale", 2.0}}, true);
    // std::cout << "Real value at index 2 is " << realFxp.getRealValue(2) << std::endl;
    // std::cout << "Size of real vector is " << realFxp.getSize() << std::endl;
    // realFxp.print();

    // // Example usage: Complex data constructor
    // std::vector<std::complex<double>> complexData = {{1.112312, -1.9358421}, {2.823462362, -2.3233492}, {3.3, -3.3}};
    // MyFxpDsp complexFxp(complexData, {{"precision", 16}}, false);
    // std::cout << "Complex value at index 2 is (" << complexFxp.getComplexValue(2).real() << ", " << complexFxp.getComplexValue(2).imag() << ")" << std::endl;
    // std::cout << "Size of complex vector is " << complexFxp.getSize() << std::endl;
    // complexFxp.print();

    return 0;
}