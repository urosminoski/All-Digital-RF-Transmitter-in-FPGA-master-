#include "fxpDsp.hpp"

// Default constructor
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
FxpDsp() 
    : fxpVector(RealVector{}), metadata{}, qFormat(false) {}

// Empty constructor with qFormat
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
FxpDsp(bool qFormat) 
    : fxpVector(RealVector{}), metadata{}, qFormat(qFormat) {}

// Real data constructor
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
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
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
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

// Make data into Q format
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
template<size_t integerWidth = 0>
void makeQ() {
    qFormat = true;

    // Calculate maximum range for Q format
    double scaleFactor = std::pow(2, integerWidth);

    // Process the variant to normalize and scale the data
    std::visit([&](auto& vector) {
        // Handle both real and complex data
        double maxVal = 0.0;

        // Find the maximum magnitude (works for real and complex)
        for (const auto& val : vector) {
            maxVal = std::max(maxVal, static_cast<double>(std::abs(val))); // For real and complex numbers
        }

        std::cout << "Max value: " << maxVal << std::endl;

        if (maxVal == 0.0) {
            throw std::runtime_error("Input data contains only zeros, normalization is not possible!");
        }

        // Normalize and scale each value
        for (auto& val : vector) {
            val = (val / maxVal) * scaleFactor; // Scaling works for both real and complex
        }
    }, fxpVector);
}


// Getter for real data
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
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
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
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

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
size_t getSize() {
    return std::visit([](const auto& vector){
        return vector.size();
    }, fxpVector);
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
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

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void printMetadata() const {
    std::cout << "\n";
    for (const auto& [key, value] : metadata) {
        std::cout << "#" << key << "=" << value << "\n";
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
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

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void writeToFile(const std::string& fileName) {
    // Open file for writing
    std::ofstream outputFile(fileName);
    if (!outputFile.is_open()) {
        throw std::runtime_error("Error: cannot open file " + fileName + " for writing!");

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

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
template<int firW, int firI>
void fir(const std::vector<double>& firCoeff) {
    // Ensure firCoeff is not empty
    if (firCoeff.empty()) {
        throw std::runtime_error("FIR coeffitients must not be empty!");
    }
    // Discretize FIR coeffitients
    using CoeffType     = ac_fixed<firW, firI, S, Q, O>;
    std::vector<CoeffType> fxpFirCoeff;
    fxpFirCoeff.reserve(firCoeff.size());
    for (auto& coeff : firCoeff) {
        fxpFirCoeff.emplace_back(coeff);
    }

    std::visit([&](auto& vector) {
        using InputType     = typename std::decay_t<decltype(vector)>::value_type;
        using SignalType    = std::conditional_t<
                                std::is_same_v<InputType, double>,
                                ac_fixed<W+firW, I+firI, S, Q, O>,
                                ac_complex<ac_fixed<W+firW, I+firI, S, Q, O>>
                            >;
        std::vector<SignalType> delayLine(firCoeff.size(), SignalType(0));
        size_t k = 0;

        for (size_t n = 0; n < vector.size(); n++) {
            vector[n] = firConvolution<InputType, SignalType, CoeffType>(vector[n], k, delayLine, fxpFirCoeff);
        }

    }, fxpVector);
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
template<typename CoeffType, size_t ratio>
void makePolyFir(const std::vector<double>& firCoeff,
                    std::vector<std::vector<CoeffType>>& firCoeff_poly) {
    // Validate firCoeff: Check that firCoeff is not empty
    if (firCoeff.empty()) {
        throw std::invalid_argument("FIR coeffitients for interpolation cannot be empty!");
    }

    // Copy firCoeff to avoid modifying original
    std::vector<double> firCoeff_padded = firCoeff;

    // Calculate the next power of two
    size_t targetSize = std::pow(2, std::ceil(std::log2(firCoeff_padded.size())));

    // Resize padded vector and pad it with zeros
    firCoeff_padded.resize(targetSize, 0.0);

    // Ensure firCoeff_poly has 'ratio' rows
    firCoeff_poly.resize(ratio);
    // Divide coefficients into ratio parts
    for (size_t i = 0; i < firCoeff_padded.size(); i++) {
        CoeffType fxpCoeff = firCoeff_padded[i];
        firCoeff_poly[i % ratio].emplace_back(fxpCoeff);
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
template<int intW, int intI, size_t ratio>
void interpolator(const std::vector<double>& firCoeff) {
    // Validate firCoeff: Check that firCoeff is not empty
    if (firCoeff.empty()) {
        throw std::invalid_argument("FIR coeffitients for interpolation cannot be empty!");
    }

    using CoeffType = ac_fixed<intW, intI, S, Q, O>;

    // Make polyphase filters out of firCoeff
    std::vector<std::vector<CoeffType>> firCoeff_poly;
    makePolyFir<CoeffType, ratio>(firCoeff, firCoeff_poly);

    std::visit([&](auto& vector) {
        using InputType     = typename std::decay_t<decltype(vector)>::value_type;
        using SignalType    = std::conditional_t<
                                std::is_same_v<InputType, double>,
                                ac_fixed<W+intW, I+intI, S, Q, O>,
                                ac_complex<ac_fixed<W+intW, I+intI, S, Q, O>>
                            >;
        using OutputType    = std::conditional_t<
                                std::is_same_v<InputType, double>,
                                RealVector,
                                ComplexVector
                            >;

        std::vector<std::vector<SignalType>> delayLine(ratio, std::vector<SignalType>(firCoeff_poly[0].size(), SignalType(0.0)));
        std::vector<size_t> k(ratio, 0);
        OutputType outputVector;
        InputType output;

        outputVector.reserve(vector.size() * ratio);

        for (size_t n = 0; n < vector.size(); n++) {
            for (size_t m = 0; m < ratio; m++) {
                output = firConvolution<InputType, SignalType, CoeffType>(vector[n], k[m], delayLine[m], firCoeff_poly[m]);
                outputVector.emplace_back(output);
            }
        }
        vector = std::move(outputVector);
    }, fxpVector);
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
template<int intW, int intI>
void interpolation(const std::vector<std::vector<double>>& firCoeffs) {
    size_t firNum = firCoeffs.size();

    for (size_t i = 0; i < firNum; i++) {
        interpolator<intW, intI, 2>(firCoeffs[i]);
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
template<typename SignalType>
void iirParallel(SignalType& input, SignalType& output,
                    const std::vector<std::vector<double>>& iirCoeff,
                    std::vector<std::vector<SignalType>>& delayLine) {
    using CoeffType = ac_fixed<32, 16, S, Q, O>;
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

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
template<int iirW, int iirI, int iI, 
            int oW, bool highPrecision = false>
void deltaSigma(const std::vector<std::vector<double>>& iirCoeff) {
    // Ensure iirCoeff is not empty
    if (iirCoeff.empty()) {
        throw std::runtime_error("IIR coeffitients are empty!");
    }

    // Discretize iirCoeff
    using CoeffType = ac_fixed<iirW, iirI, S, Q, O>;

    // Define iirCoeff_fxp outside the conditional blocks
    std::vector<std::vector<double>> iirCoeff_fxp;

    if constexpr (!highPrecision) {
        iirCoeff_fxp = iirCoeff;
        for (size_t i = 0; i < iirCoeff.size(); i++) {
            for (size_t j = 0; j < iirCoeff[i].size(); j++) {
                iirCoeff_fxp[i][j] = CoeffType(iirCoeff[i][j]).to_double();
            }
        }
    } else if constexpr (highPrecision) {
        FxpFactory<W, S, Q, O> factory;
        iirCoeff_fxp = factory.convertMatrix(iirCoeff);
    }

    auto process = [&](auto& vector) -> void {
        using OriginalSignalType    = typename std::decay_t<decltype(vector)>::value_type;
        using SignalType            = std::conditional_t<
                                        std::is_same_v<OriginalSignalType, double>,
                                        ac_fixed<W+iirW, iI, S, Q, O>,
                                        ac_complex<ac_fixed<W+iirW, iI, S, Q, O>>
                                        >;
        using SignalTypeIn         = std::conditional_t<
                                        std::is_same_v<OriginalSignalType, double>,
                                        ac_fixed<W, I, S, Q, O>,
                                        ac_complex<ac_fixed<W, I, S, Q, O>>
                                        >;
        using SignalTypeOut         = std::conditional_t<
                                        std::is_same_v<OriginalSignalType, double>,
                                        ac_fixed<oW, oW, S, Q, O>,
                                        ac_complex<ac_fixed<oW, oW, S, Q, O>>
                                        >;
        // using CoeffType             = ac_fixed<iirW, iirI, S, Q, O>;
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
                iirParallel<SignalType>(error, iirOutput, iirCoeff_fxp, delayLine);
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
                iirParallel<SignalType>(error, iirOutput, iirCoeff_fxp, delayLine);
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

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
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