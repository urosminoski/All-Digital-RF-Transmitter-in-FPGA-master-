#include <iostream>
#include <cstdint>
#include <type_traits>
#include <cmath>

enum class OverflowMode {
    Saturate,
    Overflow
};

class FixedPoint {
public:
    // Constructors
    FixedPoint() : value(0), nFrac(0), totalBits(0), mode(OverflowMode::Overflow) {}
    FixedPoint(double d, uint32_t fb, uint32_t tb, OverflowMode om = OverflowMode::Overflow)
        : totalBits(tb), nFrac(fb), mode(om) {
        value = DoubleToFixed(d, fb);
        applyOverflowOrSaturation();
    }

    // Constructor with no initial value (useful for target variable)
    FixedPoint(uint32_t fb, uint32_t tb, OverflowMode om = OverflowMode::Overflow)
        : value(0), nFrac(fb), totalBits(tb), mode(om) {}

    // Get double value
    double GetDoubleValue() const {
        return FixedToDouble(value, nFrac);
    }

    // Get int value
    int32_t GetIntValue() const {
        return value;
    }

    // Print configuration of FixedPoint object
    void printConfig() const {
        bool isSigned = std::is_signed<decltype(value)>::value;
        std::cout << (isSigned ? "s" : "u") << totalBits << "." << nFrac << std::endl;
    }

    // Arithmetic operations
    FixedPoint operator+(const FixedPoint& other) const {
        if (nFrac != other.nFrac) {
            throw std::invalid_argument("Mismatched fractional bits!");
        }
        int32_t result = value + other.value;
        return FixedPoint::fromRawValue(result, nFrac, totalBits, mode);
    }

    FixedPoint operator*(const FixedPoint& other) const {
        // Perform multiplication in double precision to avoid loss of information
        int64_t result = int64_t(value) * int64_t(other.value);
        result += (1 << (nFrac - 1)); // Add rounding term
        // Shift result to the correct number of fractional bits
        return FixedPoint::fromRawValue(static_cast<int32_t>(result >> nFrac), nFrac, totalBits, mode);
    }

    // Assignment operator to handle resizing and conversion after multiplication
    FixedPoint& operator=(const FixedPoint& other) {
        // Adjust the result to fit the current object's fractional bit configuration
        int64_t adjustedValue = int64_t(other.value) << (nFrac - other.nFrac); // Adjust precision
        applyOverflowOrSaturation(adjustedValue); // Apply overflow or saturation based on the mode
        value = static_cast<int32_t>(adjustedValue);
        return *this;
    }

private:
    int32_t value;
    uint32_t nFrac;
    uint32_t totalBits;
    OverflowMode mode;

    // Convert from double to fixed-point
    static int32_t DoubleToFixed(double d, uint32_t nFrac) {
        return int32_t(d * double(1 << nFrac) + (d >= 0 ? 0.5 : -0.5));
    }

    // Convert from fixed-point to double
    static double FixedToDouble(int32_t d, uint32_t nFrac) {
        return double(d) / double(1 << nFrac);
    }

    // Apply overflow or saturation based on the mode
    void applyOverflowOrSaturation() {
        if (mode == OverflowMode::Saturate) {
            saturate();
        } else {
            maskOverflow();
        }
    }

    // Apply overflow or saturation based on the mode for the adjusted value
    void applyOverflowOrSaturation(int64_t& adjustedValue) {
        if (mode == OverflowMode::Saturate) {
            saturate(adjustedValue);
        } else {
            maskOverflow(adjustedValue);
        }
    }

    // Saturate the value to the given total number of bits
    void saturate() {
        int32_t maxVal = (1 << (totalBits - 1)) - 1;
        int32_t minVal = -(1 << (totalBits - 1));
        if (value > maxVal) value = maxVal;
        if (value < minVal) value = minVal;
    }

    void saturate(int64_t& extendedValue) {
        int32_t maxVal = (1 << (totalBits - 1)) - 1;
        int32_t minVal = -(1 << (totalBits - 1));
        if (extendedValue > maxVal) extendedValue = maxVal;
        if (extendedValue < minVal) extendedValue = minVal;
    }

    // Mask the value to simulate overflow
    void maskOverflow() {
        int32_t mask = (1 << totalBits) - 1;  // Mask to keep only `totalBits` bits
        int32_t signMask = 1 << (totalBits - 1); // Mask to extract the sign bit

        value &= mask;  // Apply the mask

        // Sign extend the value if necessary
        if (value & signMask) {
            value |= ~mask;  // If the sign bit is set, extend the sign
        }
    }

    void maskOverflow(int64_t& extendedValue) {
        int64_t mask = (1 << totalBits) - 1;  // Mask to keep only `totalBits` bits
        int64_t signMask = 1 << (totalBits - 1); // Mask to extract the sign bit

        extendedValue &= mask;  // Apply the mask

        // Sign extend the value if necessary
        if (extendedValue & signMask) {
            extendedValue |= ~mask;  // If the sign bit is set, extend the sign
        }
    }

    // Private constructor to create from raw value
    static FixedPoint fromRawValue(int32_t rawValue, uint32_t fb, uint32_t tb, OverflowMode om) {
        FixedPoint fp;
        fp.value = rawValue;
        fp.nFrac = fb;
        fp.totalBits = tb;
        fp.mode = om;
        fp.applyOverflowOrSaturation();  // Ensure that the value fits within the total bits or overflows
        return fp;
    }
};

int main() {
    uint32_t totalBitsA = 4;
    uint32_t fracBitsA = 2;

    uint32_t totalBitsB = 4;
    uint32_t fracBitsB = 2;

    uint32_t totalBitsC = 9;
    uint32_t fracBitsC = 5;

    double da = 1.25;
    double db = 0.5;

    // Define FixedPoint objects a, b with specific precision
    FixedPoint a(da, fracBitsA, totalBitsA, OverflowMode::Overflow);
    FixedPoint b(db, fracBitsB, totalBitsB, OverflowMode::Overflow);

    // Define FixedPoint object c with different precision
    FixedPoint c(fracBitsC, totalBitsC, OverflowMode::Overflow);

    // Perform multiplication with c = a * b
    c = a * b;

    // Output the results
    std::cout << "a (Overflow) = " << a.GetDoubleValue() << " (" << a.GetIntValue() << ")" << std::endl;
    a.printConfig();

    std::cout << "b (Overflow) = " << b.GetDoubleValue() << " (" << b.GetIntValue() << ")" << std::endl;
    b.printConfig();

    std::cout << "c (Overflow) = " << c.GetDoubleValue() << " (" << c.GetIntValue() << ")" << std::endl;
    c.printConfig();

    return 0;
}
