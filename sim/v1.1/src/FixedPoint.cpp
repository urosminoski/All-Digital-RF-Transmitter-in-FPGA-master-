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

// Resize method
void FixedPoint::Resize(uint8_t newTotalBits, uint8_t newFracBits) {
    if (newFracBits > fracBits) {
        rawValue <<= (newFracBits - fracBits);
    } else if (newFracBits < fracBits) {
        int32_t roundingFactor = (1 << (fracBits - newFracBits - 1));
        if (rawValue < 0) {
            roundingFactor = -roundingFactor;
        }
        rawValue = (rawValue + roundingFactor) >> (fracBits - newFracBits);
    }
    fracBits = newFracBits;
    totalBits = newTotalBits;
    applyOverflowOrSaturation();
}

// Addition operator
FixedPoint FixedPoint::operator+(const FixedPoint& other) const {
    FixedPoint alignedThis = *this;
    FixedPoint alignedOther = other;

    // Align the precision by resizing the operands
    if (alignedThis.fracBits != alignedOther.fracBits || alignedThis.totalBits != alignedOther.totalBits) {
        uint8_t maxFracBits = std::max(alignedThis.fracBits, alignedOther.fracBits);
        uint8_t maxTotalBits = std::max(alignedThis.totalBits, alignedOther.totalBits);

        alignedThis.Resize(maxTotalBits, maxFracBits);
        alignedOther.Resize(maxTotalBits, maxFracBits);
    }

    int32_t result = alignedThis.rawValue + alignedOther.rawValue;
    return fromRawValue(result, alignedThis.fracBits, alignedThis.totalBits, alignedThis.overflowMode);
}

// Subtraction operator
FixedPoint FixedPoint::operator-(const FixedPoint& other) const {
    FixedPoint alignedThis = *this;
    FixedPoint alignedOther = other;

    // Align the precision by resizing the operands
    if (alignedThis.fracBits != alignedOther.fracBits || alignedThis.totalBits != alignedOther.totalBits) {
        uint8_t maxFracBits = std::max(alignedThis.fracBits, alignedOther.fracBits);
        uint8_t maxTotalBits = std::max(alignedThis.totalBits, alignedOther.totalBits);

        alignedThis.Resize(maxTotalBits, maxFracBits);
        alignedOther.Resize(maxTotalBits, maxFracBits);
    }

    int32_t result = alignedThis.rawValue - alignedOther.rawValue;
    return fromRawValue(result, alignedThis.fracBits, alignedThis.totalBits, alignedThis.overflowMode);
}

// Multiplication operator
FixedPoint FixedPoint::operator*(const FixedPoint& other) const {
    // Align fractional bits by resizing the operands
    uint32_t resultFracBits = this->fracBits + other.fracBits;
    uint32_t resultTotalBits = this->totalBits + other.totalBits;
    
    int64_t resultRawValue = static_cast<int64_t>(this->rawValue) * static_cast<int64_t>(other.rawValue);
    
    // Adjust the result to fit within the new fractional bits
    if (resultFracBits > 32) {
        resultRawValue >>= (resultFracBits - 32);
        resultFracBits = 32;
    }

    return fromRawValue(resultRawValue, resultFracBits, resultTotalBits, this->overflowMode);
}

// Assignment operator with precision adjustment
FixedPoint& FixedPoint::operator=(const FixedPoint& other) {
    if (fracBits > other.fracBits) {
        rawValue = other.rawValue << (fracBits - other.fracBits);
    } else if (fracBits < other.fracBits) {
        int32_t roundingFactor = (1 << (other.fracBits - fracBits - 1));
        if (other.rawValue < 0) {
            roundingFactor = -roundingFactor;
        }
        rawValue = (other.rawValue + roundingFactor) >> (other.fracBits - fracBits);
    } else {
        rawValue = other.rawValue;
    }

    applyOverflowOrSaturation();
    return *this;
}

// Assignment operator for assigning a double value
FixedPoint& FixedPoint::operator=(const double value) {
    rawValue = ConvertDoubleToFixed(value, fracBits);
    applyOverflowOrSaturation();
    return *this;
}

// Assignment operator for assigning an int value
FixedPoint& FixedPoint::operator=(const int value) {
    rawValue = value << fracBits;  // Shift left to account for fractional bits
    applyOverflowOrSaturation();
    return *this;
}

// Convert from double to fixed-point
int32_t FixedPoint::ConvertDoubleToFixed(double value, uint32_t fracBits) {
    return static_cast<int32_t>(value * (1 << fracBits) + (value >= 0 ? 0.5 : -0.5));
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
FixedPoint FixedPoint::fromRawValue(int64_t rawValue, uint32_t fracBits, uint32_t totalBits, OverflowMode mode) {
    FixedPoint fp;
    fp.rawValue = static_cast<int32_t>(rawValue);  // Truncate if necessary
    fp.fracBits = fracBits;
    fp.totalBits = totalBits;
    fp.overflowMode = mode;
    fp.applyOverflowOrSaturation();
    return fp;
}

} // End of namespace fxp
