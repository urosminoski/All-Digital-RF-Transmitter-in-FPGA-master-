#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <iostream>
#include <cstdint>
#include <type_traits>
#include <cmath>

namespace fxp{

enum class OverflowMode {
    Saturate,
    Overflow
};

class FixedPoint {
public:
    // Constructors
    FixedPoint();
    FixedPoint(double value, uint8_t totalBits, uint8_t fracBits, OverflowMode mode = OverflowMode::Overflow);
    FixedPoint(uint8_t totalBits, uint8_t fracBits, OverflowMode mode = OverflowMode::Overflow);

    // Convert to double
    double ToDouble() const;

    // Convert to integer
    int32_t ToInt() const;

    // Getters
    uint8_t GetTotalBits() const;
    uint8_t GetFracBits() const;
    OverflowMode GetOverflowMode() const;

    // Print the configuration of FixedPoint object
    void PrintConfig() const;

    // Resize method
    void Resize(uint8_t newTotalBits, uint8_t newFracBits);

    // Arithmetic operators
    FixedPoint operator+(const FixedPoint& other) const;
    FixedPoint operator-(const FixedPoint& other) const;
    FixedPoint operator*(const FixedPoint& other) const;

    // Assignment operator with precision adjustment
    // FixedPoint& operator=(const FixedPoint& other);

private:
    int32_t rawValue;           // Raw fixed-point value
    uint32_t fracBits;          // Number of fractional bits
    uint32_t totalBits;         // Total number of bits (integer + fractional)
    OverflowMode overflowMode;  // Mode for handling overflow (Saturate or Overflow)

    // Convert from double to fixed-point
    static int32_t ConvertDoubleToFixed(double value, uint32_t fracBits);

    // Apply overflow or saturation based on the mode
    void applyOverflowOrSaturation();

    // Saturate the value to fit within the allowed range
    void saturate();

    // Apply mask for overflow (wrap around)
    void maskOverflow();

    // Create a FixedPoint from a raw value
    static FixedPoint fromRawValue(int32_t rawValue, uint32_t fracBits, uint32_t totalBits, OverflowMode mode);

    // Ensure the two FixedPoint objects are compatible for arithmetic operations
    void checkCompatibility(const FixedPoint& other) const;
};

}   // End of namespace fxp

#endif // FIXEDPOINT_H
