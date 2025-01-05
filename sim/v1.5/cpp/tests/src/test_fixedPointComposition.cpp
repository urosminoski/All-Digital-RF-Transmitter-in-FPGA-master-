#include <iostream>
#include <stdexcept>
#include <complex>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <type_traits>
#include <ac_fixed.h>
#include <ac_complex.h>

#include <fstream>
#include <sstream>
#include <string>

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
class FxpVectorHandler;
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
class FxpMatrixHandler;
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
class FixedPoint;


template<int W, int I, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FixedPoint {
private:
    std::unique_ptr<FxpVectorHandler<W, I, S, Q, O>> vectorHandler;
    std::unique_ptr<FxpMatrixHandler<W, I, S, Q, O>> matrixHandler;

public:
    // Constructor initializes handlers based on needs
    FixedPoint(bool useVector = true, bool useMatrix = false) {
        if (useVector) {
            vectorHandler = std::make_unique<FxpVectorHandler<W, I, S, Q, O>>();
        }
        if (useMatrix) {
            matrixHandler = std::make_unique<FxpMatrixHandler<W, I, S, Q, O>>();
        }
    }

    // Initialize vector data
    void initVector(const std::vector<double>& inVector) {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->init(inVector);
    }
    // Initialize vector data
    void initVector(const std::vector<std::complex<double>>& inVector) {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->init(inVector);
    }
    // Initialize matrix data
    void initMatrix(const std::vector<std::vector<double>>& inMatrix) {
        if (!matrixHandler) {
            throw std::runtime_error("Matrix handler not initialized!");
        }
        matrixHandler->init(inMatrix);
    }

    //
    void readVector(const std::string& fileName) {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->readFromFile(fileName);
    }
    void writeVector(const std::string& fileName) {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->writeToFile(fileName);
    }

    ac_fixed<W, I, S, Q, O> getFxpReal(const size_t index) {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->getFxpReal(index);
    }
    ac_complex<ac_fixed<W, I, S, Q, O>> getFxpComplex(const size_t index) {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->getFxpComplex(index);
    }
    double getReal(const size_t index) {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->getReal(index);
    }
    std::complex<double> getComplex(const size_t index) {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->getComplex(index);
    }

    // // Perform delta-sigma modulation
    // void deltaSigma() {
    //     if (!vectorHandler || !matrixHandler) {
    //         throw std::runtime_error("Bot vector and matrix handlers must be initialized!");
    //     }
    //     vectorHandler->deltaSigma(*matrixHandler);
    // }

    // Printing methods for debugging
    void printVector() const {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->print();
    }
    void printVectorMetadata() const {
        if (!vectorHandler) {
            throw std::runtime_error("Vector handler not initialized!");
        }
        vectorHandler->printMetadata();
    }
    void printMatrix() const {
        if (!matrixHandler) {
            throw std::runtime_error("Matrix handler not initialized!");
        }
        matrixHandler->print();
    }
};

template<int W, int I, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FxpMatrixHandler {
private:
    using FxpRealVector = std::vector<ac_fixed<W, I, S, Q, O>>;
    using FxpRealMatrix = std::vector<std::vector<ac_fixed<W, I, S, Q, O>>>;
    
    FxpRealMatrix matrixData;

public:
    FxpMatrixHandler()
        : matrixData(FxpRealMatrix{}) {}

    FxpMatrixHandler(const std::vector<std::vector<double>>& inMatrix) {
        for (const auto& row : inMatrix) {
            FxpRealVector fxpVector;
            for (const auto& val : row) {
                fxpVector.emplace_back(val);
            }
            matrixData.emplace_back(std::move(fxpVector));
        }
    }

    void init(const std::vector<std::vector<double>>& inMatrix) {
        for (const auto& row : inMatrix) {
            FxpRealVector fxpVector;
            for (const auto& val : row) {
                fxpVector.emplace_back(val);
            }
            matrixData.emplace_back(std::move(fxpVector));
        }
    }

    void print() const {
        std::cout << "\nMatrix Data:\n";
        for (const auto& row : matrixData) {
            for (const auto& val : row) {
                std::cout << val.to_double() << ", ";
            }
            std::cout << "\n";
        }
    }
};

template<int W, int I, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FxpVectorHandler {
private:
    using FxpRealVector = std::vector<ac_fixed<W, I, S, Q, O>>;
    using FxpComplexVector = std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>;
    
    std::variant<FxpRealVector,
                 FxpComplexVector,
                 std::vector<int>> vectorData;
    std::map<std::string, double> metadata;

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
    FxpVectorHandler()
        : vectorData(FxpRealVector{})  // Initialize with an empty real vector by default
    {
        // Optionally initialize metadata or other state here
        metadata.clear();
    }

    // Initialize with real data
    FxpVectorHandler(const std::vector<double>& inVector) {
        FxpRealVector fxpVector;
        for (const auto& val : inVector) {
            fxpVector.emplace_back(val);
        }
        vectorData = std::move(fxpVector);
    }
    // Initialize with complex data
    FxpVectorHandler(const std::vector<std::complex<double>>& inVector) {
        FxpComplexVector fxpVector;
        for (const auto& val : inVector) {
            fxpVector.emplace_back(ac_fixed<W, I, S, Q, O>(val.real()),
                                   ac_fixed<W, I, S, Q, O>(val.imag()));
        }
        vectorData = std::move(fxpVector);
    }

    // Initialize with real data
    void init(const std::vector<double>& inVector) {
        FxpRealVector fxpVector;
        for (const auto& val : inVector) {
            fxpVector.emplace_back(val);
        }
        vectorData = std::move(fxpVector);
    }
    // Initialize with complex data
    void init(const std::vector<std::complex<double>>& inVector) {
        FxpComplexVector fxpVector;
        for (const auto& val : inVector) {
            fxpVector.emplace_back(ac_fixed<W, I, S, Q, O>(val.real()),
                                   ac_fixed<W, I, S, Q, O>(val.imag()));
        }
        vectorData = std::move(fxpVector);
    }

    ac_fixed<W, I, S, Q, O> getFxpReal(const size_t index) {
        std::visit([](const auto& vec) {
            using T = std::decay_t<decltype(vec)>;
            if constexpr (std::is_same_v<T, FxpRealVector>) {
                std::cout << "vectorData contains FxpRealVector.\n";
            } else if constexpr (std::is_same_v<T, FxpComplexVector>) {
                std::cout << "vectorData contains FxpComplexVector.\n";
            } else {
                std::cout << "vectorData contains an unexpected type.\n";
            }
        }, vectorData);

        auto* vec = std::get_if<FxpRealVector>(&vectorData);
        if (!vec) {
            throw std::runtime_error("Vector does not contain real values!");
        }
        if (index >= vec->size()) {
            throw std::runtime_error("Index is out of bound!");
        }
        std::cout << "Hello" << (*vec)[index] << std::endl;
        return (*vec)[index];
    }

    ac_complex<ac_fixed<W, I, S, Q, O>> getFxpComplex(const size_t index) {
        auto* vec = std::get_if<FxpComplexVector>(&vectorData);
        if (!vec) {
            throw std::runtime_error("Vector does not contain real values!");
        }
        if (index >= vec->size()) {
            throw std::runtime_error("Index is out of bound!");
        }
        return (*vec)[index];
    }

    double getReal(const size_t index) {
        return getFxpReal(index).to_double();
    }

    std::complex<double> getComplex(const size_t index) {
        ac_complex<ac_fixed<W, I, S, Q, O>> complexFxp = getFxpComplex(index);
        return std::complex<double>(complexFxp.r().to_double(), complexFxp.i().to_double());
    }

    // Print stored data for debugging
    void print() const {
        std::visit([](const auto& vec) {
            using T = typename std::decay_t<decltype(vec)>;
            if constexpr (std::is_same_v<T, FxpRealVector>) {
                std::cout << "Real Data:\n";
                for (const auto& val : vec) {
                    std::cout << val.to_double() << "\n";
                }
                std::cout << std::endl;
            } else if constexpr (std::is_same_v<T, FxpComplexVector>) {
                std::cout << "Complex Data:\n";
                for (const auto& val : vec) {
                    std::cout << "(" << val.r().to_double() << ", " << val.i().to_double() << ")\n";
                }
                std::cout << std::endl;
            }
            
        }, vectorData);
    }
    void printMetadata() const {
        std::cout << "Metadata:\n";
        for (const auto& [key, val] : metadata) {
            std::cout << key << "=" << val << "\n";
        }
        std::cout << std::endl;
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
        std::visit([&outputFile](const auto& dataVec) {
            for (const auto& val : dataVec) {
                using T = typename std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, ac_fixed<W, I, S, Q, O>>) {
                    outputFile << val.to_double() << "\n";
                } else if constexpr (std::is_same_v<T, ac_complex<ac_fixed<W, I, S, Q, O>>>) {
                    outputFile << val.r().to_double() << " " << val.i().to_double() << "\n";
                } else if constexpr (std::is_same_v<T, int>) {
                    outputFile << val << "\n";
                } else {
                    throw std::runtime_error("Error: Unexpected data type during write operation.\n");
                }
            }
        }, vectorData);
    }

    // template<typename SignalType>
    // void IIR_parallel(SignalType& input, SignalType& output, 
    //                   const FixedPoint<W, I, S, Q, O>& iirCoefficients,
    //                   std::vector<std::vector<SignalType>>& delayLine) {
    //     output = 0;
    //     for (size_t i = 0; i < iirCoefficients.getMatrixRowCount(); i++) {
    //         delayLine[i][0] = input - iirCoefficients.getMatrix(i, 4)*delayLine[i][1] - 
    //                                     iirCoefficients.getMatrix(i, 5)*delayLine[i][2];
    //         output += iirCoefficients.getMatrix(i, 0)*delayLine[i][0] + 
    //                         iirCoefficients.getMatrix(i, 1)*delayLine[i][1] + 
    //                         iirCoefficients.getMatrix(i, 2)*delayLine[i][2];
    //         delayLine[i][2] = delayLine[i][1];
    //         delayLine[i][1] = delayLine[i][0];
    //     }
    // }

    // template<int intermediate_I, int out_I>
    // void deltaSigma(const FixedPoint<W, I, S, Q, O>& iirCoefficients) {
    //     // Lambda to process both real and complex data
    //     auto process = [&](auto& vector, const auto& iirCoefficients) -> void {
    //         using OriginalSignalType = typename std::decay_t<decltype(vector)>::value_type;
    //         using SignalType = std::conditional_t<
    //             std::is_same_v<OriginalSignalType, ac_fixed<W, I, S, Q, O>>,
    //             ac_fixed<2*W, intermediate_I, S, Q, O>,
    //             ac_complex<ac_fixed<2*W, intermediate_I, S, Q, O>>
    //         >;
    //         using SignalTypeOut = std::conditional_t<
    //             std::is_same_v<OriginalSignalType, ac_fixed<W, I, S, Q, O>>,
    //             ac_fixed<out_I, out_I, S, Q, O>,
    //             ac_complex<ac_fixed<out_I, out_I, S, Q, O>>
    //         >;

    //         // Initialize variables for intermediate and feedback computations
    //         SignalType iirOutput = 0, error = 0, intermediateOutput = 0;
    //         SignalTypeOut outputSample = 0;
    //         // Initialize IIR states (y, w, wd, wdd)
    //         std::vector<std::vector<SignalType>> delayLine(
    //             iirCoefficients.getMatrixRowCount(),
    //             std::vector<SignalType>(3, SignalType())
    //         );

    //         for (auto& sample : vector) {
    //             intermediateOutput = sample + iirOutput;
    //             outputSample = intermediateOutput;
    //             // Append the processed sample to the output vector
    //             sample = outputSample;
    //             error = intermediateOutput - outputSample;

    //             IIR_parallel<SignalType>(error, iirOutput, iirCoefficients, delayLine);
    //         }
    //     };

    //     // Use std::visit ti handle variant
    //     auto& variantData = this->vectorData;
    //     std::visit([&](auto& vector) -> void {
    //         process(vector, iirCoefficients);
    //     }, variantData);
    // }
};

#define IIR_FILTERS { \
  {7.3765809    , 0             , 0 , 1 , -0.3466036    , 0             }, \
  {0.424071040  , -2.782608716  , 0 , 1 , -0.66591402   , 0.16260264    }, \
  {-4.606822182 , 0.023331537   , 0 , 1 , -0.62380242   , 0.4509869     }  \
}

int main() {
    constexpr int W = 12; 
    constexpr int I = 4;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;

    // FixedPoint<W, I, S, Q, O> fxpData;
    // // Init with real data
    // std::vector<double> realData = {1.12312312, -2.12312444212, 3.094875621};
    // fxpData.initVector(realData);
    // fxpData.printVector();
    // // Initialize with complex data
    // std::vector<std::complex<double>> complexData = {{1.123124342, -2.123123443}, {-3.4652324, 4.89324882374878}};
    // fxpData.initVector(complexData);
    // fxpData.printVector();
    // fxpData.printVectorMetadata();

    // std::vector<std::vector<double>> iirCoeff = IIR_FILTERS;

    // std::string file_path_in = "./data/input/sinData.txt";
    // std::string file_path_out = "./data/output/sinData_out.txt";

    // FixedPoint<W, I, S, Q, O> fxpData(true, true);
    // fxpData.readVector(file_path_in);
    // fxpData.writeVector(file_path_out);
    // fxpData.initMatrix(iirCoeff);
    // fxpData.printMatrix();

    std::vector<double> realData = {1.0, 2.0, 3.0};

    FixedPoint<W, I, S, Q, O> fxpData(true, false);
    fxpData.initVector(realData);
    double RealVal = fxpData.getReal(0);


    // ac_fixed<W, I, S, Q, O> fxpRealVal = fxpData.getFxpReal(0);
    // // ac_complex<ac_fixed<W, I, S, Q, O>> fxpComplexVal = fxpData.getFxpComplex(0);
    // double RealVal = fxpData.getReal(0);
    // std::complex<double> ComplexVal = fxpData.getComplex(0);

    return 0;
}

