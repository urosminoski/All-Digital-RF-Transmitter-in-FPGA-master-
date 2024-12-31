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
class FixedPointVector {
private:
    std::variant<std::vector<ac_fixed<W, I, S, Q, O>>, std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>> data;

public:

    // Accessors for template parameters
    constexpr int getW() const { return W; }
    constexpr int getI() const { return I; }
    constexpr bool isSigned() const { return S; }
    constexpr ac_q_mode getQMode() const { return Q; }
    constexpr ac_o_mode getOMode() const { return O; }

    FixedPointVector(bool isComplex = false) {
        if (isComplex) {
            data = std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>();
        } else {
            data = std::vector<ac_fixed<W, I, S, Q, O>>();
        }
    }

    // FixedPointVector(const std::vector<double>& InputVec) {
    //     std::vector<ac_fixed<W, I, S, Q, O>> fixedVec;
    //     fixedVec.reserve(InputVec.size());
    //     for (const auto& realVal : InputVec) {
    //         fixedVec.emplace_back(realVal);
    //     }
    //     data = std::move(fixedVec);
    // }

    // FixedPointVector(const std::vector<std::complex<double>>& InputVec) {
    //     std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>> fixedVec;
    //     fixedVec.reserve(InputVec.size());
    //     for (const auto& complexVal : InputVec) {
    //         ac_fixed<W, I, S, Q, O> realPart(complexVal.real());
    //         ac_fixed<W, I, S, Q, O> imagPart(complexVal.imag());
    //         fixedVec.emplace_back(ac_complex<ac_fixed<W, I, S, Q, O>>(realPart, imagPart));
    //     }
    //     data = std::move(fixedVec);
    // }

    FixedPointVector(const std::variant<std::vector<double>, std::vector<std::complex<double>>>& InputVec) {
        std::visit([&](const auto& vec) {
            using T = std::decay_t<decltype(vec)>;
            if constexpr (std::is_same_v<T, std::vector<double>>) {
                // Handle real data
                std::vector<ac_fixed<W, I, S, Q, O>> fixedVec;
                fixedVec.reserve(vec.size());
                for (const auto& realVal : vec) {
                    fixedVec.emplace_back(realVal); // Convert double to ac_fixed
                }
                data = std::move(fixedVec);
            } else if constexpr (std::is_same_v<T, std::vector<std::complex<double>>>) {
                // Handle complex data
                std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>> fixedVec;
                fixedVec.reserve(vec.size());
                for (const auto& complexVal : vec) {
                    ac_fixed<W, I, S, Q, O> realPart(complexVal.real());
                    ac_fixed<W, I, S, Q, O> imagPart(complexVal.imag());
                    fixedVec.emplace_back(ac_complex<ac_fixed<W, I, S, Q, O>>(realPart, imagPart));
                }
                data = std::move(fixedVec);
            } else {
                throw std::runtime_error("Unsupported data type in InputVec");
            }
        }, InputVec);
    }

    void push_back(double value) {
        auto& fixedVec = std::get_if<std::vector<ac_fixed<W, I, S, Q, O>>>(&data);
        if (!fixedVec) {
            throw std::runtime_error("Vector is storing complex data; cannot add real data");
        }
        fixedVec->emplace_back(value);
    }

    void push_back(std::complex<double> value) {
        auto& fixedVec = std::get_if<std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(&data);
        if (!fixedVec) {
            throw std::runtime_error("Vector is storing real data; cannot add complex data");
        }
        fixedVec->emplace_back(value);
    }

    void print() const {
        std::visit([](const auto& vec) {
            using T = typename std::decay_t<decltype(vec)>;
            if constexpr (std::is_same_v<T, std::vector<ac_fixed<W, I, S, Q, O>>>) {
                for (const auto& val : vec) {
                    std::cout << val.to_double() << ", ";
                }
                std::cout << std::endl;
            } else if constexpr (std::is_same_v<T, std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>) {
                for (const auto& val : vec) {
                    std::cout << "(" << val.r().to_double() << ", " << val.i().to_double() << "), ";
                }
                std::cout << std::endl;
            }
        }, data);
    }

    auto getVal(size_t index) const {
        return std::visit([index](const auto& vec) -> std::variant<ac_fixed<W, I, S, Q, O>, ac_complex<ac_fixed<W, I, S, Q, O>>> {
            if (index >= vec.size()) {
                throw std::out_of_range("Index out of bounds!");
            }
            return vec[index];
        }, data);
    }

    ac_fixed<W, I, S, Q, O> getReal(size_t index) {
        if (std::holds_alternative<std::vector<ac_fixed<W, I, S, Q, O>>>(data)) {
            const auto& vec = std::get<std::vector<ac_fixed<W, I, S, Q, O>>>(data);
            if (index >= vec.size()) {
                throw std::out_of_range("Index out of bounds!");
            } else {
                return vec[index];
            }
        } else {
            throw std::runtime_error("Data does not contain complex values.");
        }
    }

    ac_complex<ac_fixed<W, I, S, Q, O>> getComplex(size_t index) {
        if (std::holds_alternative<std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(data)) {
            const auto& vec = std::get<std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(data);
            if (index >= vec.size()) {
                throw std::out_of_range("Index out of bounds!");
            } else {
                return vec[index];
            }
        } else {
            throw std::runtime_error("Data does not contain real values.");
        }
    }

    bool isComplex() const {
        return std::holds_alternative<std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>>(data);
    }

    bool isReal() const {
        return std::holds_alternative<std::vector<ac_fixed<W, I, S, Q, O>>>(data);
    }
};


#define IIR_FILTERS { \
  {7.3765809, 0, 0, 1, 0.3466036, 0}, \
  {0.424071040, 2.782608716, 0, 1, 0.66591402, 0.16260264}, \
  {4.606822182, 0.023331537, 0, 1, 0.62380242, 0.4509869} \
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        // Expecting two parameters, file names
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_deltaSigma_file> <input_LUT_file> <output_serial_file>" << std::endl;
        return 1;
    }

    std::string inputDataFileName = argv[1];
    std::string inputLutFileName = argv[2];

    // Read input file
    std::variant<std::vector<double>, std::vector<std::complex<double>>> inputData;
    std::map<std::string, double> metadata;
    if (!readFromFile(inputDataFileName, inputData, metadata)) {
        std::cerr << "Error: Reading form file " << inputDataFileName << " was unsuccessful!" << std::endl;
        return 1;
    }

    constexpr int W = 12; 
    constexpr int I = 4;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;

    // Discretize IIR coeffitients
    std::vector<std::vector<double>> iirCoeff = IIR_FILTERS;
    std::vector<FixedPointVector<W, I, S, Q, O>> iirCoeffFixed;
    for (const auto& vec : iirCoeff) {
        FixedPointVector<W, I, S, Q, O> fixedVec(vec);
        iirCoeffFixed.emplace_back(fixedVec);
    }

    FixedPointVector<W, I, S, Q, O> inputDataFixed(inputData);

    // deltaSigma(inputDataFixed, iirCoeffFixed);

    // inputDataFixed.print();

    // for (const auto& vec : iirCoeffFixed) {
    //     vec.print();
    // }

    return 0;
}