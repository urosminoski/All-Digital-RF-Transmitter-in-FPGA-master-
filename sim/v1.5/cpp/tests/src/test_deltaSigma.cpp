#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <variant>
#include <map>
#include <type_traits>
#include <ac_fixed.h>
#include <ac_complex.h>
#include "funcs.hpp"
#include <complex>

template<int W, int I, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FixedPoint {
private:
    std::variant<std::vector<ac_fixed<W, I, S, Q, O>>, std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>> vectorData;
    std::vector<std::vector<ac_fixed<W, I, S, Q, O>>> matrixData;

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

    // Accessors for template parameters
    constexpr int getW() const { return W; }
    constexpr int getI() const { return I; }
    constexpr bool isSigned() const { return S; }
    constexpr ac_q_mode getQMode() const { return Q; }
    constexpr ac_o_mode getOMode() const { return O; }

     // Expose the underlying variant
    const auto& getVariant() const {
        return vectorData;
    }

    auto& getVariant() {
        return vectorData;
    }

    // 
    FixedPoint(bool isComplex = false)
        : vectorData(isComplex 
            ? std::variant<std::vector<ac_fixed<W, I, S, Q, O>>, std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(
                std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>())
            :std::variant<std::vector<ac_fixed<W, I, S, Q, O>>, std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(
                std::vector<ac_fixed<W, I, S, Q, O>>())) {}

    FixedPoint(const std::vector<double>& inputVec) {
        std::vector<ac_fixed<W, I, S, Q, O>> fixedVec;
        for (const auto& val : inputVec) {
            fixedVec.emplace_back(val);
        }
        vectorData = std::move(fixedVec);
    }

    FixedPoint(const std::vector<std::complex<double>>& inputVec) {
        std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>> fixedVec;
        for (const auto& val : inputVec) {
            fixedVec.emplace_back(
                ac_fixed<W, I, S, Q, O>(val.real()),
                ac_fixed<W, I, S, Q, O>(val.imag())
            );
        }
        vectorData = std::move(fixedVec);
    }

    FixedPoint(const std::variant<std::vector<double>, std::vector<std::complex<double>>>& InputVec) {
        std::visit([&](const auto& vec) {
            using T = std::decay_t<decltype(vec)>;
            if constexpr (std::is_same_v<T, std::vector<double>>) {
                // Handle real data
                std::vector<ac_fixed<W, I, S, Q, O>> fixedVec;
                fixedVec.reserve(vec.size());
                for (const auto& realVal : vec) {
                    fixedVec.emplace_back(realVal); // Convert double to ac_fixed
                }
                vectorData = std::move(fixedVec);
            } else if constexpr (std::is_same_v<T, std::vector<std::complex<double>>>) {
                // Handle complex data
                std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>> fixedVec;
                fixedVec.reserve(vec.size());
                for (const auto& complexVal : vec) {
                    ac_fixed<W, I, S, Q, O> realPart(complexVal.real());
                    ac_fixed<W, I, S, Q, O> imagPart(complexVal.imag());
                    fixedVec.emplace_back(ac_complex<ac_fixed<W, I, S, Q, O>>(realPart, imagPart));
                }
                vectorData = std::move(fixedVec);
            } else {
                throw std::runtime_error("Unsupported data type in InputVec");
            }
        }, InputVec);
    }

    FixedPoint(const std::vector<std::vector<double>>& inputMatrix) {
        for (const auto& row : inputMatrix) {
            std::vector<ac_fixed<W, I, S, Q, O>> fixedRow;
            for (const auto& val : row) {
                fixedRow.emplace_back(val);
            }
            matrixData.emplace_back(std::move(fixedRow));
        }
    }

    void push_back(const double val) {
        auto* fixedVec = std::get_if<std::vector<ac_fixed<W, I, S, Q, O>>>(&vectorData);
        if (!fixedVec) {
            throw std::runtime_error("Vector is storing complex data; cannot add real data!");
        }
        fixedVec->emplace_back(ac_fixed<W, I, S, Q, O>(val));
    }

    void push_back(const std::complex<double> val) {
        auto* fixedComplexVec = std::get_if<std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(&vectorData);
        if (!fixedComplexVec) {
            throw std::runtime_error("Vector is storing real data; cannot add complex data!");
        }
        fixedComplexVec->emplace_back(ac_complex<ac_fixed<W, I, S, Q, O>>(
            ac_fixed<W, I, S, Q, O>(val.real()),
            ac_fixed<W, I, S, Q, O>(val.imag())
        ));
    }

    void print() const {
        std::visit([](const auto& vec) {
            using T = typename std::decay_t<decltype(vec)>;
            if constexpr (std::is_same_v<T, std::vector<ac_fixed<W, I, S, Q, O>>>) {
                std::cout << "Real Data:\n";
                for (const auto& val : vec) {
                    std::cout << val.to_double() << "\n";
                }
                std::cout << "\n";
            } else if constexpr (std::is_same_v<T, std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>) {
                std::cout << "Complex Data:\n";
                for (const auto& val : vec) {
                    std::cout << "(" << val.r().to_double() << ", " <<  val.i().to_double() << ")\n";
                }
                std::cout << "\n";
            }
        }, vectorData);
    }

    void print_matrix() const {
        std::cout << "Matrix Data:\n";
        for (const auto& row : matrixData) {
            for (const auto& val : row) {
                std::cout << val.to_double() << ", ";
            }
            std::cout << "\n";
        }
        std::cout << std::endl;
    }

    ac_fixed<W, I, S, Q, O> getMatrix(const size_t row, const size_t col) {
        // Check if the requested row index is valid
        if (row >= matrixData.size()) {
            throw std::out_of_range("Row index out of bounds!");
        }
        // Check if the requested column index is valid
        if (col >= matrixData[row].size()) {
            throw std::out_of_range("Column index out of bounds!");
        }
        // Return the value at the specified row and column
        return matrixData[row][col];
    }

    ac_fixed<W, I, S, Q, O> getMatrix(const size_t row, const size_t col) const {
        // Check if the requested row index is valid
        if (row >= matrixData.size()) {
            throw std::out_of_range("Row index out of bounds!");
        }
        // Check if the requested column index is valid
        if (col >= matrixData[row].size()) {
            throw std::out_of_range("Column index out of bounds!");
        }
        // Return the value at the specified row and column
        return matrixData[row][col];
    }

    ac_fixed<W, I, S, Q, O> getReal(const size_t index) {
        auto* fixedVec = std::get_if<std::vector<ac_fixed<W, I, S, Q, O>>>(&vectorData);
        if (!fixedVec) {
            throw std::runtime_error("Vector does not contain real values!");
        }
        if (index >= fixedVec->size()) {
            throw std::out_of_range("Index out of bounds!");
        }
        return (*fixedVec)[index];
    }

    ac_fixed<W, I, S, Q, O> getReal(const size_t index) const {
        auto* fixedVec = std::get_if<std::vector<ac_fixed<W, I, S, Q, O>>>(&vectorData);
        if (!fixedVec) {
            throw std::runtime_error("Vector does not contain real values!");
        }
        if (index >= fixedVec->size()) {
            throw std::out_of_range("Index out of bounds!");
        }
        return (*fixedVec)[index];
    }

    ac_complex<ac_fixed<W, I, S, Q, O>> getComplex(const size_t index) {
        auto* fixedVec = std::get_if<std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(&vectorData);
        if (!fixedVec) {
            throw std::runtime_error("Vector does not contain complex values!");
        }
        if (index >= fixedVec->size()) {
            throw std::out_of_range("Index out of bounds!");
        }
        return (*fixedVec)[index];
    }

    ac_complex<ac_fixed<W, I, S, Q, O>> getComplex(const size_t index) const {
        auto* fixedVec = std::get_if<std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(&vectorData);
        if (!fixedVec) {
            throw std::runtime_error("Vector does not contain complex values!");
        }
        if (index >= fixedVec->size()) {
            throw std::out_of_range("Index out of bounds!");
        }
        return (*fixedVec)[index];
    }

    size_t getVectorSize() const {
        return std::visit([](const auto& vec) -> size_t {
            return vec.size();
        }, vectorData);
    }

    size_t getMatrixRowCount() const {
        return matrixData.size(); // Number of rows
    }

    size_t getMatrixColumnCount(const size_t row = 0) const {
        if (row >= matrixData.size()) {
            throw std::out_of_range("Row index out of bounds!");
        }
        return matrixData[row].size(); // Number of columns in the specified row
    }



    void readFromFile(const std::string& fileName, std::map<std::string, double>& metadata) {
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
                } else if (count == 1) {
                    isComplex = false;
                } else {
                    throw std::runtime_error("Invalid data format in line: " + line);
                }
                firstDataLine = false;
            }

            parseDataLine(line, isComplex, realData, complexData);
        }

        if (isComplex) {
            std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>> fixedComplexData;
            for (const auto& c : complexData) {
                fixedComplexData.emplace_back(
                    ac_fixed<W, I, S, Q, O>(c.real()),
                    ac_fixed<W, I, S, Q, O>(c.imag()));
            }
            vectorData = std::move(fixedComplexData);
        } else {
            std::vector<ac_fixed<W, I, S, Q, O>> fixedRealData;
            for (const auto& r : realData) {
                fixedRealData.emplace_back(ac_fixed<W, I, S, Q, O>(r));
            }
            vectorData = std::move(fixedRealData);
        }

        inputFile.close();
    }

    void writeToFile(const std::string& fileName,
                     const std::map<std::string, double>& metadata) {
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
        std::visit([&outputFile](const auto& dataVec) {
            for (const auto& val : dataVec) {
                using T = typename std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, ac_fixed<W, I, S, Q, O>>) {
                    outputFile << val.to_double() << "\n";
                } else if constexpr (std::is_same_v<T, ac_complex<ac_fixed<W, I, S, Q, O>>>) {
                    outputFile << val.r().to_double() << " " << val.i().to_double() << "\n";
                } else {
                    throw std::runtime_error("Error: Unexpected data type during write operation.\n");
                }
            }
        }, vectorData);
    }

    template<typename SignalType>
    void IIR_parallel(SignalType& input, SignalType& output, 
                      const FixedPoint<W, I, S, Q, O>& iirCoefficients,
                      std::vector<std::vector<SignalType>>& delayLine) {
        output = 0;
        for (size_t i = 0; i < iirCoefficients.getMatrixRowCount(); i++) {
            delayLine[i][0] = input - iirCoefficients.getMatrix(i, 4)*delayLine[i][1] - 
                                        iirCoefficients.getMatrix(i, 5)*delayLine[i][2];
            output += iirCoefficients.getMatrix(i, 0)*delayLine[i][0] + 
                            iirCoefficients.getMatrix(i, 1)*delayLine[i][1] + 
                            iirCoefficients.getMatrix(i, 2)*delayLine[i][2];
            delayLine[i][2] = delayLine[i][1];
            delayLine[i][1] = delayLine[i][0];
        }
    }

    template<int intermediate_I, int out_I>
    void deltaSigma(const FixedPoint<W, I, S, Q, O>& iirCoefficients) {
        // Lambda to process both real and complex data
        auto process = [&](auto& vector, const auto& iirCoefficients) -> void {
            using OriginalSignalType = typename std::decay_t<decltype(vector)>::value_type;
            using SignalType = std::conditional_t<
                std::is_same_v<OriginalSignalType, ac_fixed<W, I, S, Q, O>>,
                ac_fixed<2*W, intermediate_I, S, Q, O>,
                ac_complex<ac_fixed<2*W, intermediate_I, S, Q, O>>
            >;
            using SignalTypeOut = std::conditional_t<
                std::is_same_v<OriginalSignalType, ac_fixed<W, I, S, Q, O>>,
                ac_fixed<out_I, out_I, S, Q, O>,
                ac_complex<ac_fixed<out_I, out_I, S, Q, O>>
            >;

            // Initialize variables for intermediate and feedback computations
            SignalType iirOutput = 0, error = 0, intermediateOutput = 0;
            SignalTypeOut outputSample = 0;
            // Initialize IIR states (y, w, wd, wdd)
            std::vector<std::vector<SignalType>> delayLine(
                iirCoefficients.getMatrixRowCount(),
                std::vector<SignalType>(3, SignalType())
            );

            for (auto& sample : vector) {
                intermediateOutput = sample + iirOutput;
                outputSample = intermediateOutput;
                // Append the processed sample to the output vector
                sample = outputSample;
                error = intermediateOutput - outputSample;

                IIR_parallel<SignalType>(error, iirOutput, iirCoefficients, delayLine);
            }
        };

        // Use std::visit ti handle variant
        auto& variantData = this->vectorData;
        std::visit([&](auto& vector) -> void {
            process(vector, iirCoefficients);
        }, variantData);
    }
};



#define IIR_FILTERS { \
  {7.3765809    , 0             , 0 , 1 , -0.3466036    , 0             }, \
  {0.424071040  , -2.782608716  , 0 , 1 , -0.66591402   , 0.16260264    }, \
  {-4.606822182 , 0.023331537   , 0 , 1 , -0.62380242   , 0.4509869     }  \
}

int main(int argc, char* argv[]) {
    constexpr int W = 12; 
    constexpr int I = 4;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;

    std::vector<std::vector<double>> iirCoeff = IIR_FILTERS;
    FixedPoint<W, I, S, Q, O> iirCoeffFixed(iirCoeff);

    std::string file_path_in = "./data/input/sinData.txt";
    std::string file_path_out = "./data/output/sinData_deltaSigma.txt";

    std::map<std::string, double> metadata;
    FixedPoint<W, I, S, Q, O> signal;
    signal.readFromFile(file_path_in, metadata);
    signal.deltaSigma<4, 4>(iirCoeffFixed);
    signal.writeToFile(file_path_out, metadata);

    std::string file_path_in2 = "./data/input/sinDataComplex.txt";
    std::string file_path_out2 = "./data/output/sinDataComplex_deltaSigma.txt";

    std::map<std::string, double> metadata2;
    FixedPoint<W, I, S, Q, O> signal2;
    signal2.readFromFile(file_path_in2, metadata2);
    signal2.deltaSigma<4, 4>(iirCoeffFixed);
    signal2.writeToFile(file_path_out2, metadata2);

    return 0;
}