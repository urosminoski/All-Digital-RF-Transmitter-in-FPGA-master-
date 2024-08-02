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
        checkCompatibility(other);
        int32_t result = value + other.value;
        return FixedPoint::fromRawValue(result, nFrac, totalBits, mode);
    }

    // Multiplies two FixedPoint numbers and returns a result with a higher precision
    FixedPoint operator*(const FixedPoint& other) const {
        int64_t extendedResult = int64_t(value) * int64_t(other.value);
        // No need to shift yet, as we want to maintain full precision for further operations
        return FixedPoint::fromRawValueExtended(extendedResult, nFrac + other.nFrac, totalBits * 2, mode);
    }

    // Assignment operator with downscaling of precision
    FixedPoint& operator=(const FixedPoint& other) {
        // Scale the result down to the target precision
        int64_t scaledValue = other.value >> (other.nFrac - nFrac); // Adjust the fractional part
        value = static_cast<int32_t>(scaledValue);
        applyOverflowOrSaturation();
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

    void applyOverflowOrSaturation(int64_t& adjustedValue) {
        if (mode == OverflowMode::Saturate) {
            saturate(adjustedValue);
        } else {
            maskOverflow(adjustedValue);
        }
    }

    void saturate() {
        int32_t maxVal = (1 << (totalBits - 1)) - 1;
        int32_t minVal = -(1 << (totalBits - 1));
        if (value > maxVal) value = maxVal;
        if (value < minVal) value = minVal;
    }

    void saturate(int64_t& extendedValue) {
        int64_t maxVal = (1LL << (totalBits - 1)) - 1;
        int64_t minVal = -(1LL << (totalBits - 1));
        if (extendedValue > maxVal) extendedValue = maxVal;
        if (extendedValue < minVal) extendedValue = minVal;
    }

    void maskOverflow() {
        int32_t mask = (1 << totalBits) - 1;
        int32_t signMask = 1 << (totalBits - 1);

        value &= mask;

        if (value & signMask) {
            value |= ~mask;
        }
    }

    void maskOverflow(int64_t& extendedValue) {
        int64_t mask = (1LL << totalBits) - 1;
        int64_t signMask = 1LL << (totalBits - 1);

        extendedValue &= mask;

        if (extendedValue & signMask) {
            extendedValue |= ~mask;
        }
    }

    // Used for intermediate results with extended precision
    static FixedPoint fromRawValueExtended(int64_t rawValue, uint32_t fb, uint32_t tb, OverflowMode om) {
        FixedPoint fp;
        fp.value = static_cast<int32_t>(rawValue >> fb); // Right shift to get back to original precision
        fp.nFrac = fb / 2;  // Adjust back to the original fractional bits
        fp.totalBits = tb / 2;  // Adjust back to the original total bits
        fp.mode = om;
        fp.applyOverflowOrSaturation();
        return fp;
    }

    static FixedPoint fromRawValue(int32_t rawValue, uint32_t fb, uint32_t tb, OverflowMode om) {
        FixedPoint fp;
        fp.value = rawValue;
        fp.nFrac = fb;
        fp.totalBits = tb;
        fp.mode = om;
        fp.applyOverflowOrSaturation();
        return fp;
    }

    void checkCompatibility(const FixedPoint& other) const {
        if (nFrac != other.nFrac || totalBits != other.totalBits) {
            throw std::invalid_argument("Mismatched FixedPoint configurations!");
        }
    }
};

int main() {
    uint32_t totalBits = 8;
    uint32_t fracBits = 2;

    double da = 1.25;
    double db = 0.25;
    double dc = 1.5;

    // Initialize `a`, `b`, and `c` with specific values
    FixedPoint a(da, fracBits, totalBits, OverflowMode::Overflow);
    FixedPoint b(db, fracBits, totalBits, OverflowMode::Overflow);
    FixedPoint c(dc, fracBits, totalBits, OverflowMode::Overflow);

    // Declare `d` with higher precision
    uint32_t totalBitsD = totalBits * 3;
    uint32_t fracBitsD = fracBits * 3;
    FixedPoint d(fracBitsD, totalBitsD, OverflowMode::Overflow);

    // Perform the multiplication with higher precision result
    d = a * b * c;

    // Output the results
    std::cout << "a (Overflow) = " << a.GetDoubleValue() << " (" << a.GetIntValue() << ")" << std::endl;
    a.printConfig();

    std::cout << "b (Overflow) = " << b.GetDoubleValue() << " (" << b.GetIntValue() << ")" << std::endl;
    b.printConfig();

    std::cout << "c (Overflow) = " << c.GetDoubleValue() << " (" << c.GetIntValue() << ")" << std::endl;
    c.printConfig();

    std::cout << "d (Overflow) = " << d.GetDoubleValue() << " (" << d.GetIntValue() << ")" << std::endl;
    d.printConfig();

    return 0;
}
