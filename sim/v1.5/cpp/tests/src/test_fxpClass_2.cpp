#include <iostream>
#include <stdexcept>
#include <complex>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <ac_fixed.h>
#include <ac_complex.h>

#include <fstream>
#include <sstream>
#include <string>

template <typename T>
inline constexpr bool always_false_v = false;

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

    template<typename SignalType, typename CoeffType>
    void iirParallel(SignalType& input, SignalType& output,
                     const std::vector<std::vector<double>>& iirCoeff,
                     std::vector<std::vector<SignalType>>& delayLine) {
        output = 0;
        for (size_t i = 0; i < iirCoeff.size(); i++) {
            delayLine[i][0] = input - CoeffType(iirCoeff[i][4])*delayLine[i][1] -
                                      CoeffType(iirCoeff[i][5])*delayLine[i][2];
            output += CoeffType(iirCoeff[i][0])*delayLine[i][0] +
                      CoeffType(iirCoeff[i][1])*delayLine[i][1] +
                      CoeffType(iirCoeff[i][2])*delayLine[i][2];
            delayLine[i][2] = delayLine[i][1];
            delayLine[i][1] = delayLine[i][0];
        }
    }

    template<int iW = 2*W, int iI, int oW, 
             int oI = oW, int iirW = W, int iirI = I>
    void deltaSigma(const std::vector<std::vector<double>>& iirCoeff) {
        // Ensure iirCoeff is not empty
        if (iirCoeff.empty()) {
            throw std::runtime_error("IIR coeffitients are empty!");
        }

        auto process = [&](auto& vector) -> void {
            using OriginalSignalType    = typename std::decay_t<decltype(vector)>::value_type;
            using SignalType            = std::conditional_t<
                                            std::is_same_v<OriginalSignalType, double>,
                                            ac_fixed<iW, iI, S, Q, O>,
                                            ac_complex<ac_fixed<iW, iI, S, Q, O>>
                                            >;
            using SignalTypeIn         = std::conditional_t<
                                            std::is_same_v<OriginalSignalType, double>,
                                            ac_fixed<W, I, S, Q, O>,
                                            ac_complex<ac_fixed<W, I, S, Q, O>>
                                            >;
            using SignalTypeOut         = std::conditional_t<
                                            std::is_same_v<OriginalSignalType, double>,
                                            ac_fixed<oW, oI, S, Q, O>,
                                            ac_complex<ac_fixed<oW, oI, S, Q, O>>
                                            >;
            using CoeffType             = ac_fixed<iirW, iirI, S, Q, O>;
            // Initialize variables for intermediate and feedback computations
            SignalType iirOutput = 0, error = 0, intermediateOutput = 0;
            SignalTypeOut outputSample = 0;
            // Initialize IIR delay lines
            std::vector<std::vector<SignalType>> delayLine(
                iirCoeff.size(),
                std::vector<SignalType>(3, SignalType())
            );

            for (auto& sample : vector) {
                if constexpr (std::is_same_v<OriginalSignalType, double>) {
                    SignalTypeIn fxpSample = sample;
                    intermediateOutput  = fxpSample + iirOutput;
                    outputSample        = intermediateOutput;
                    sample              = outputSample.to_double();     // In-place processing
                    error = intermediateOutput - outputSample;
                    iirParallel<SignalType, CoeffType>(error, iirOutput, iirCoeff, delayLine);
                } else if constexpr (std::is_same_v<OriginalSignalType, std::complex<double>>) {
                    SignalTypeIn fxpSample(
                        sample.real(),
                        sample.imag()
                    );
                    intermediateOutput  = fxpSample + iirOutput;
                    outputSample        = intermediateOutput;
                    sample              = std::complex<double>(      // In-place processing
                                            outputSample.r().to_double(),
                                            outputSample.i().to_double()
                                        );
                    error = intermediateOutput - outputSample;
                    iirParallel<SignalType, CoeffType>(error, iirOutput, iirCoeff, delayLine);
                } else {
                    static_assert(always_false_v<SignalTypeOut>, "Unsupported SignalTypeOut!");
                }
            }
        };

        // Use std::visit to handle variant
        std::visit([&](auto& vector){
            using T = typename std::decay_t<decltype(vector)>;
            if constexpr (std::is_same_v<T, RealVector> ||
                          std::is_same_v<T, ComplexVector>) {
                process(vector);
            } else {
                throw std::runtime_error("Incompatible type for deltaSigma!");
            }
        }, fxpVector);
    }

    template<size_t bitW>
    void serialConverter(std::vector<std::vector<int>>& LUT) {
        // Validate LUT: Check that LUT is not empty
        if (LUT.empty()) {
            throw std::invalid_argument("LUT cannot be empty!");
        }

        // Validate LUT: Check that all rows have the same number of columns
        size_t columSize = LUT[0].size();
        for (const auto& row : LUT) {
            if (row.empty()) {
                throw std::invalid_argument("LUT rows cannot be empty!");
            }
            if (row.size() != columSize) {
                throw std::invalid_argument("All rows in LUT must have the same size!");
            }
        }

        // Cast LUT to std::vector<std::vector<double>> to be compatible with fxpVector
        std::vector<std::vector<double>> doubleLUT;
        doubleLUT.reserve(LUT.size());
        for (const auto& row : LUT) {
            std::vector<double> doubleRow;
            doubleRow.reserve(row.size());
            std::transform(row.begin(), row.end(), std::back_inserter(doubleRow),
                        [](int value) { return static_cast<double>(value); });
            doubleLUT.emplace_back(std::move(doubleRow));
        }

        // Update metadata
        metadata["lut_width"] = static_cast<double>(columSize);

        // Lambda function to process both real and complex data
        std::visit([&](auto& vector) {
            int correction = 1 << (bitW - 1);

            using T = typename std::decay_t<decltype(vector)>;
            // Process each value
            if constexpr (std::is_same_v<T, RealVector>) {
                // Real data processing
                RealVector result;
                result.reserve(vector.size() * columSize);
                for (const auto& value : vector) {
                    auto correctedValue = value + correction;
                    if (correctedValue < 0 || static_cast<size_t>(correctedValue) >= doubleLUT.size()) {
                        throw std::out_of_range("Input value results in out-of-range LUT index!");
                    }
                    size_t position = static_cast<size_t>(correctedValue);
                    const auto& lutRow = doubleLUT[doubleLUT.size() - 1 - position];
                    result.insert(result.end(), lutRow.begin(), lutRow.end());
                }
                fxpVector = std::move(result);

            } else if constexpr (std::is_same_v<T, ComplexVector>) {
                // Complex data processing
                ComplexVector result;
                result.reserve(vector.size() * columSize * 2);
                for (const auto& value : vector) {
                    auto correctedReal = value.real() + correction;
                    auto correctedImag = value.imag() + correction;
                    if (correctedReal < 0 || correctedImag < 0 ||
                        static_cast<size_t>(correctedReal) >= doubleLUT.size() ||
                        static_cast<size_t>(correctedImag) >= doubleLUT.size()) {
                        throw std::out_of_range("Input value results in out-of-range LUT index!");
                    }
                    size_t positionReal = static_cast<size_t>(correctedReal);
                    size_t positionImag = static_cast<size_t>(correctedImag);
                    const auto& realLutRow = doubleLUT[doubleLUT.size() - 1 - positionReal];
                    const auto& imagLutRow = doubleLUT[doubleLUT.size() - 1 - positionImag];
                    for (size_t i = 0; i < realLutRow.size(); i++) {
                        result.emplace_back(realLutRow[i], imagLutRow[i]);
                    }
                }
                fxpVector = std::move(result);

            } else {
                throw std::runtime_error("Incompatible type for serialConverter!");
            }
        }, fxpVector);
    }
};

bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT);

#define IIR_FILTERS { \
  {7.3765809    , 0             , 0 , 1 , -0.3466036    , 0             }, \
  {0.424071040  , -2.782608716  , 0 , 1 , -0.66591402   , 0.16260264    }, \
  {-4.606822182 , 0.023331537   , 0 , 1 , -0.62380242   , 0.4509869     }  \
}

int main() {
    using MyFxpDsp = FxpDsp<12, 4, true, AC_RND, AC_SAT>;

    std::vector<std::vector<double>> iirCoeff = IIR_FILTERS;

    std::string file_path_lut = "../../data/luts/LUT4.json";
    std::vector<std::vector<int>> LUT;
    readLUT(file_path_lut, LUT);

    std::string file_path_in_1          = "./data/input/sinData.txt";
    std::string file_path_deltaSigma_1  = "./data/output/sinData_deltaSigma.txt";
    std::string file_path_serialized_1  = "./data/output/sinData_serial.txt";

    std::string file_path_in_2          = "./data/input/sinDataComplex.txt";
    std::string file_path_deltaSigma_2  = "./data/output/sinDataComplex_deltaSigma.txt";
    std::string file_path_serialized_2  = "./data/output/sinDataComplex_serial.txt";

    MyFxpDsp realSignal;
    realSignal.readFromFile(file_path_in_1);
    realSignal.deltaSigma<24, 4, 4, 4, 12, 4>(iirCoeff);
    realSignal.writeToFile(file_path_deltaSigma_1);
    realSignal.serialConverter<4>(LUT);
    realSignal.writeToFile(file_path_serialized_1);

    MyFxpDsp complexSignal;
    complexSignal.readFromFile(file_path_in_2);
    complexSignal.deltaSigma<24, 4, 4, 4, 12, 4>(iirCoeff);
    complexSignal.writeToFile(file_path_deltaSigma_2);
    complexSignal.serialConverter<4>(LUT);
    complexSignal.writeToFile(file_path_serialized_2);

    // Example usage: Default constructor
    // MyFxpDsp fxpVector1;
    // fxpVector1.readFromFile(file_path_in_real);
    // fxpVector1.writeToFile(file_path_out_real);
    // fxpVector1.printMetadata();
    // fxpVector1.print();
    // MyFxpDsp fxpVector2;
    // fxpVector2.readFromFile(file_path_in_complex);
    // fxpVector2.writeToFile(file_path_out_complex);
    // fxpVector2.printMetadata();
    // fxpVector2.print();

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

bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT)
{
    // Clear the LUT vector to ensure it starts empty
    LUT.clear();

    // Open the file
    std::ifstream file(fileName);

    // Check if the file can be opened successfully
    if (!file.is_open())
    {
        // Throw an exception if the file cannot be opened
        throw std::runtime_error("Could not open file: " + fileName);
        return false; // Unnecessary but included for clarity
    }

    // Create a JSON object to hold the file content
    nlohmann::json j;

    // Parse the file content into the JSON object
    file >> j;

    // Close the file after reading
    file.close();

    // Convert the JSON object to a 2D vector of integers
    LUT = j.get<std::vector<std::vector<int>>>();

    // Return true to indicate successful parsing
    return true;
}