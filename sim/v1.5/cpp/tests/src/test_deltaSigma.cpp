#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <variant>
#include <type_traits>
#include <ac_fixed.h>
#include <ac_complex.h>
#include <complex>


// Abstract Base Class for Fixed-Point Numbers
class FixedPointBase {
public:
    virtual ~FixedPointBase() = default;    // Default destructor
    virtual void print() const = 0;         // Pure virtual print function 
};

// Template Derived Class for Different ac_fixed Types
template<int W, int I, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FixedPoint : public FixedPointBase {
private:
    ac_fixed<W, I, S, Q, O> ac_type;
    int i_width;

public:
    FixedPoint(double val) : ac_type(val), i_width(I) {}

    void print() const override {
        std::cout << "ac_fixed<" << W << ", " << I << ">: " << ac_type.to_double() << std::endl;
    }

    // Getter for underlying ac_fixed value
    const ac_fixed<W, I, S, Q, O>& getValue() const {
        return ac_type;
    }

    // Getter fot integer width
    int getIntegerWidth() const {
        return i_width;
    }
};

// Template Class for Managing a Fixed-Point Matrix
template<int W, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FixedPointMatrix {
private:
    std::vector<std::vector<std::shared_ptr<FixedPointBase>>> fixedPointMatrix;

    // Function to deduce the integer part `I` based on the coefficient value and total width `W`
    int deduceIntegerPart(double coeff) const {
        if (coeff < 0) {
            coeff = -coeff; // Consider absolute value for range determination
        }

        int requiredBits = 0;
        if (coeff > 0) {
            requiredBits = static_cast<int>(std::ceil(std::log2(static_cast<int>(coeff) + 1)));
        }

        // Check if calculated integer width is valid
        if (requiredBits >= W) {
            std::cerr << "Error: The number " << coeff
                      << " requires " << requiredBits
                      << " integer bits, but only " << W
                      << " total bits are available.\n";
            return -1;
        }

        // Deduce I: Total width minus bits required for the integer part
        return requiredBits + 1;
    }

    // Factory Function for Creating FixedPoint Objects
    std::shared_ptr<FixedPointBase> createFixedPoint(double value, int I) const {
        auto create = [&]<int... Is>(std::integer_sequence<int, Is...>) -> std::shared_ptr<FixedPointBase> {
            std::shared_ptr<FixedPointBase> ptr = nullptr;
            ((I == Is ? ptr = std::make_shared<FixedPoint<W, Is, S, Q, O>>(value) : nullptr), ...);
            return ptr;
        };
        return create(std::make_integer_sequence<int, W>{});
    }

public:
    // Constructor for Creating Fixed-Point Matrix
    FixedPointMatrix(const std::vector<std::vector<double>>& inputMatrix) {
        // Iterate over each row
        for (const auto& row : inputMatrix) {
            std::vector<std::shared_ptr<FixedPointBase>> rowBuffer;
            // Iterate over each element in the row
            for (const auto& value : row) {
                int I = deduceIntegerPart(value); // Deduce integer width
                if (I < 0) {
                    throw std::runtime_error("Invalid integer width deduced for coefficient.");
                }
                rowBuffer.emplace_back(createFixedPoint(value, I)); // Create and store FixedPoint object
            }
            fixedPointMatrix.emplace_back(std::move(rowBuffer));
        }
    }

    // Print function
    void print() const {
        for (const auto& row : fixedPointMatrix) {
            for (const auto& value : row) {
                value->print();
            }
            std::cout << std::endl;
        }
    }

    // // Method to get the value for a specific element as a ac_fixed
    // ac_fixed<W, I, S, Q, O> getValue(size_t row, size_t col) const {
    //     if (row >= fixedPointMatrix.size() || row < 0 ||
    //         col >= fixedPointMatrix[row].size() || col < 0) {
    //             throw std::out_of_range("Index out of range!");
    //     }

    //     // Dynamic cast to the spoecific position in matrix
    //     // I = fixedPointMatrix[row][col]::i_width;
    //     auto fixedPoint = dynamic_cast<FixedPoint<W, I, S, Q, O>*>(fixedPointMatrix[row][col].get());
    //     if (!fixedPoint) {
    //         throw std::runtime_error("Invalid FixedPointBase cast to FixedPoint");
    //     }
        
    //     // Return the ac_fixed value
    //     return fixedPoint->getValue();
    // }
};

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

    FixedPointVector(const std::vector<double>& InputVec) {
        std::vector<ac_fixed<W, I, S, Q, O>> fixedVec;
        fixedVec.reserve(InputVec.size());
        for (const auto& realVal : InputVec) {
            fixedVec.emplace_back(realVal);
        }
        data = std::move(fixedVec);
    }

    FixedPointVector(const std::vector<std::complex<double>>& InputVec) {
        std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>> fixedVec;
        fixedVec.reserve(InputVec.size());
        for (const auto& complexVal : InputVec) {
            ac_fixed<W, I, S, Q, O> realPart(complexVal.real());
            ac_fixed<W, I, S, Q, O> imagPart(complexVal.imag());
            fixedVec.emplace_back(ac_complex<ac_fixed<W, I, S, Q, O>>(realPart, imagPart));
        }
        data = std::move(fixedVec);
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

int main() {

    std::vector<double> realVec = {2.12034, 0.2345, -2.234523, -0.2334521};
    std::vector<std::complex<double>> complexVec = {
        {2.1123123, 3.21231231}, 
        {-1.5, 4.123124}, 
        {0.0, -2.21313243123}, 
        {-1.1, 0.5}
    };

    constexpr int W = 12;
    constexpr int I = 4;
    constexpr bool S = true;
    constexpr ac_q_mode Q = AC_RND;
    constexpr ac_o_mode O = AC_SAT;

    FixedPointVector<W, I, S, Q, O> fixedRealVec(realVec);
    FixedPointVector<W, I, S, Q, O> fixedComplexVec(complexVec);

    fixedRealVec.print();
    fixedComplexVec.print();

    if (fixedRealVec.isReal()) {
        std::cout << fixedRealVec.getReal(0).to_double() << std::endl;   
    } else {
        std::cout << "(" << fixedRealVec.getComplex(0).r().to_double() << ", " << fixedRealVec.getComplex(0).i().to_double() << ")" << std::endl;
    }

    if (fixedComplexVec.isReal()) {
        std::cout << fixedComplexVec.getReal(0).to_double() << std::endl;   
    } else {
        std::cout << "(" << fixedComplexVec.getComplex(0).r().to_double() << ", " << fixedComplexVec.getComplex(0).i().to_double() << ")" << std::endl;
    }

    return 0;
}