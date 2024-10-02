#include "../inc/FixedPoint.hpp"

namespace fxp {

// Constructors
FixedPoint::FixedPoint() : rawValue(0), fracBits(0), totalBits(0), overflowMode(OverflowMode::Overflow) {}

FixedPoint::FixedPoint(double value, uint8_t totalBits, uint8_t fracBits, OverflowMode mode)
    : totalBits(totalBits), fracBits(fracBits), overflowMode(mode) {
    rawValue = ConvertDoubleToFixed(value, fracBits);
    applyOverflowOrSaturation();
}

FixedPoint::FixedPoint(uint8_t totalBits, uint8_t fracBits, OverflowMode mode)
    : rawValue(0), totalBits(totalBits), fracBits(fracBits), overflowMode(mode) {}

// Convert to double
double FixedPoint::ToDouble() const {
    return static_cast<double>(rawValue) / (1 << fracBits);
}

// Convert to integer
int32_t FixedPoint::ToInt() const {
    return rawValue;
}

// Getter methods
uint8_t FixedPoint::GetTotalBits() const {
    return totalBits;
}

uint8_t FixedPoint::GetFracBits() const {
    return fracBits;
}

OverflowMode FixedPoint::GetOverflowMode() const {
    return overflowMode;
}

// Print the configuration of FixedPoint object
void FixedPoint::PrintConfig() const {
    std::cout << "Configuration: " << (std::is_signed<decltype(rawValue)>::value ? "s" : "u")
              << totalBits << "." << fracBits << " format\n" << std::endl;
}

void FixedPoint::Resize(uint8_t newTotalBits, uint8_t newFracBits) {
    // Step 1: Adjust the raw value if the number of fractional bits changes
    if (newFracBits > fracBits) {
        // Increase precision: shift raw value left
        rawValue <<= (newFracBits - fracBits);
    } else if (newFracBits < fracBits) {
        // Decrease precision: shift raw value right with rounding
        int32_t roundingFactor = (1 << (fracBits - newFracBits - 1));
        if(rawValue < 0){
            roundingFactor = -roundingFactor;
        }
        rawValue = (rawValue + roundingFactor) >> (fracBits - newFracBits);
    }
    
    // Step 2: Update the internal fracBits and totalBits
    fracBits = newFracBits;
    totalBits = newTotalBits;

    // Step 3: Handle overflow or saturation if needed
    applyOverflowOrSaturation();
}


// Addition operator
FixedPoint FixedPoint::operator+(const FixedPoint& other) const {
    checkCompatibility(other);
    int32_t result = rawValue + other.rawValue;
    return fromRawValue(result, fracBits, totalBits, overflowMode);
}

// Subtraction operator
FixedPoint FixedPoint::operator-(const FixedPoint& other) const {
    checkCompatibility(other);
    int32_t result = rawValue - other.rawValue;
    return fromRawValue(result, fracBits, totalBits, overflowMode);
}

// Multiplication operator
FixedPoint FixedPoint::operator*(const FixedPoint& other) const {
    checkCompatibility(other);
    int32_t extendedResult = rawValue * other.rawValue;
    return fromRawValue(extendedResult, fracBits + other.fracBits, totalBits + other.totalBits, overflowMode);
}

// Convert from double to fixed-point
int32_t FixedPoint::ConvertDoubleToFixed(double value, uint32_t fracBits) {
    return static_cast<int32_t>(std::round(value * (1 << fracBits)));
}

// Apply overflow or saturation based on the mode
void FixedPoint::applyOverflowOrSaturation() {
    if (overflowMode == OverflowMode::Saturate) {
        saturate();
    } else {
        maskOverflow();
    }
}

// Saturate the value to fit within the allowed range
void FixedPoint::saturate() {
    int32_t maxVal = (1 << (totalBits - 1)) - 1;
    int32_t minVal = -(1 << (totalBits - 1));
    if (rawValue > maxVal) rawValue = maxVal;
    if (rawValue < minVal) rawValue = minVal;
}

// Apply mask for overflow (wrap around)
void FixedPoint::maskOverflow() {
    int32_t mask = (1 << totalBits) - 1;
    rawValue &= mask;
    if (rawValue & (1 << (totalBits - 1))) {
        rawValue |= ~mask;
    }
}

// Create a FixedPoint from a raw value
FixedPoint FixedPoint::fromRawValue(int32_t rawValue, uint32_t fracBits, uint32_t totalBits, OverflowMode mode) {
    FixedPoint fp;
    fp.rawValue = rawValue;
    fp.fracBits = fracBits;
    fp.totalBits = totalBits;
    fp.overflowMode = mode;
    fp.applyOverflowOrSaturation();
    return fp;
}

// Ensure the two FixedPoint objects are compatible for arithmetic operations
void FixedPoint::checkCompatibility(const FixedPoint& other) const {
    if (overflowMode != other.overflowMode) {
        throw std::invalid_argument("Mismatched FixedPoint overflow mode configurations!");
    }
}

} // End of namespace fxp
