#ifndef FXP_DSP_H
#define FXP_DSP_H

#include <iostream>
#include <stdexcept>
#include <complex>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <cmath> // For log2
#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp> // JSON handling library
#include <ac_fixed.h>        // Arbitrary-precision fixed-point arithmetic
#include <ac_complex.h>      // Complex type for ac_fixed

// Main DSP class template
template<int W, int I, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FxpDsp {
private:
    using RealVector    = std::vector<double>;
    using ComplexVector = std::vector<std::complex<double>>;

    std::variant<RealVector, ComplexVector> fxpVector; // Variant to hold real or complex data
    std::map<std::string, double> metadata;            // Metadata for the DSP object
    bool qFormat;                                      // Flag indicating Q-format representation

    // Internal base class for ac_fixed types
    class FxpBase {
    public:
        virtual ~FxpBase() = default;

        // Method to return the stored value as a double
        virtual double getDouble() const = 0;
    };

    // Derived template class for specific ac_fixed types
    template<int LocalW, int LocalI, bool LocalS, ac_q_mode LocalQ, ac_o_mode LocalO>
    class FxpDerived : public FxpBase {
    private:
        ac_fixed<LocalW, LocalI, LocalS, LocalQ, LocalO> value;

    public:
        explicit FxpDerived(double v) : value(v) {}
        double getDouble() const override { return value.to_double(); }
    };

    // Factory class for creating ac_fixed instances dynamically
    class FxpFactory {
    private:
        constexpr static int IStart = 1;

        static int calculateI(double value) {
            if (value < 0) { value = -value; }
            return static_cast<int>(std::ceil(std::log2(value + 1)) + 1);
        }

        template<int... IValues>
        std::unique_ptr<FxpBase> createFixedImpl(double value, int targetI, std::integer_sequence<int, IValues...>) const {
            std::unique_ptr<FxpBase> result = nullptr;

            ([&]() {
                if (targetI == IValues) {
                    result = std::make_unique<FxpDerived<W, IValues, S, Q, O>>(value);
                }
            }(), ...);

            if (!result) {
                throw std::runtime_error("Unsupported I value");
            }
            return result;
        }

        double convertValue(double value) const {
            constexpr int IEnd = W;
            int targetI = calculateI(value);

            if (targetI < IStart || targetI > IEnd) {
                throw std::runtime_error("Value out of range for I");
            }

            auto fixedObj = createFixedImpl(value, targetI, std::make_integer_sequence<int, IEnd + 1 - IStart>());
            return fixedObj->getDouble();
        }

    public:
        std::vector<std::vector<double>> convertMatrix(const std::vector<std::vector<double>>& matrix) const {
            std::vector<std::vector<double>> result = matrix;
            for (size_t i = 0; i < matrix.size(); ++i) {
                for (size_t j = 0; j < matrix[i].size(); ++j) {
                    result[i][j] = convertValue(matrix[i][j]);
                }
            }
            return result;
        }
    };

    // Helper functions for file operations
    static void readMetadata(std::ifstream& inputFile, std::map<std::string, double>& metadata);
    static void parseDataLine(const std::string& line, bool isComplex,
                              std::vector<double>& realData,
                              std::vector<std::complex<double>>& complexData);

    template<typename InputType, typename SignalType, typename CoeffType>
    InputType firConvolution(const InputType& input, size_t& k,
                             std::vector<SignalType>& delayLine,
                             const std::vector<CoeffType>& fxpFirCoeff);

public:
    // Constructors
    FxpDsp();
    FxpDsp(bool qFormat);
    FxpDsp(const RealVector& inVector, const std::map<std::string, double>& meta = {}, bool qFmt = false);
    FxpDsp(const ComplexVector& inVector, const std::map<std::string, double>& meta = {}, bool qFmt = false);

    // Methods for processing
    template<size_t integerWidth = 0>
    void makeQ();

    void scale(double scaleFactor);
    double getRealValue(size_t index);
    std::complex<double> getComplexValue(size_t index);
    size_t getSize() const;
    void print() const;
    void printMetadata() const;
    void readFromFile(const std::string& fileName);
    void writeToFile(const std::string& fileName);

    template<int firW, int firI>
    void fir(const std::vector<double>& firCoeff);

    template<typename CoeffType, size_t ratio>
    void makePolyFir(const std::vector<double>& firCoeff,
                     std::vector<std::vector<CoeffType>>& firCoeff_poly);

    template<int intW, int intI, size_t ratio>
    void interpolator(const std::vector<double>& firCoeff);

    template<int intW, int intI>
    void interpolation(const std::vector<std::vector<double>>& firCoeffs);

    template<typename SignalType>
    void iirParallel(SignalType& input, SignalType& output,
                     const std::vector<std::vector<double>>& iirCoeff,
                     std::vector<std::vector<SignalType>>& delayLine);

    template<int iirW, int iirI, int iI, int oW, bool highPrecision = false>
    void deltaSigma(const std::vector<std::vector<double>>& iirCoeff);

    template<size_t bitW>
    void serialConverter(std::vector<std::vector<int>>& LUT);
};

// Function declarations for file I/O operations
bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT);
void readArray(const std::string& fileName, std::vector<double>& vector);
void readArray_2D(const std::string& fileName, std::vector<std::vector<double>>& matrix);

// Include the template definitions
#include "../src/fxpDsp.tpp" 

#endif // FXP_DSP_H
