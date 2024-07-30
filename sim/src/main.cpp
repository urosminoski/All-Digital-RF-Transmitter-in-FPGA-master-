#include <iostream>
#include <cstdint>
#include <type_traits>

// Helper functions for conversion
int32_t DoubleToFixed(double d, uint32_t nFrac) {
    return int32_t(d * double(1 << nFrac) + (d >= 0 ? 0.5 : -0.5));
}

double FixedToDouble(int32_t d, uint32_t nFrac) {
    return double(d) / double(1 << nFrac);
}

// FixedPoint class
class FixedPoint {
public:
    // Constructors
    FixedPoint() : value(0), nFrac(0) {}
    FixedPoint(double d, uint32_t fb) : value(DoubleToFixed(d, fb)), nFrac(fb){}

    // Get double value
    double GetDoubleValue() const{
        return FixedToDouble(value, nFrac);
    }

    // Get int value
    int32_t GetIntValue() const{
        return value;
    }

    // Print configuration of FixedPoint object
    void printConfig() const {
        bool isSigned = std::is_signed<decltype(value)>::value;
        uint32_t totalBits = sizeof(value) * 8;
        uint32_t integerBits = totalBits - nFrac;
        std::cout << (isSigned ? "s" : "u") << totalBits << "." << nFrac << std::endl;
    }

    // Arithmetic operations
    FixedPoint operator+(const FixedPoint& other) const{
        if(nFrac != other.nFrac){
            throw std::invalid_argument("Mismached fractional bits!");
        }
        return FixedPoint::fromRawValue(value + other.value, nFrac);
    }

    FixedPoint operator*(const FixedPoint& other) const{
        if(nFrac != other.nFrac){
            throw std::invalid_argument("Mismached fractional bits!");
        }
        int64_t result = int64_t(value) * int64_t(other.value);
        // result += (1 << (nFrac - 1)); // Add rounding term
        return FixedPoint::fromRawValue(result, (nFrac << 1));
    }

    // // Arithmetic operators
    // FixedPoint operator+(const FixedPoint& other) const {
    //     return FixedPoint::fromRawValue(value + other.value);
    // }

    // FixedPoint operator-(const FixedPoint& other) const {
    //     return FixedPoint::fromRawValue(value - other.value);
    // }

    // FixedPoint operator*(const FixedPoint& other) const {
    //     return FixedPoint::fromRawValue((int64_t(value) * int64_t(other.value)) >> db);
    // }

    // FixedPoint operator/(const FixedPoint& other) const {
    //     return FixedPoint::fromRawValue((int64_t(value) << db) / other.value);
    // }

    // // Output operator for printing
    // friend std::ostream& operator<<(std::ostream& os, const FixedPoint& fp) {
    //     return os << double(fp);
    // }

private:
    int32_t value;
    uint32_t nFrac;

    // Private constructor to create from raw value
    static FixedPoint fromRawValue(int32_t rawValue, uint32_t fb){
        FixedPoint fp;
        fp.value = rawValue;
        fp.nFrac = fb;
        return fp;
    }

    // // Private constructor to create from raw value
    // constexpr static FixedPoint fromRawValue(int32_t rawValue) {
    //     FixedPoint fp;
    //     fp.value = rawValue;
    //     return fp;
    // }
};

int main() {
    // Example usage of the FixedPoint class
    uint32_t db = 2;
    FixedPoint a(2.25, db);
    FixedPoint b(4.75, db);

    FixedPoint c = a + b;
    FixedPoint d = a * b;

    std::cout << "a = " << a.GetDoubleValue() << " (" << a.GetIntValue() << ")" << std::endl;
    std::cout << "b = " << b.GetDoubleValue() << " (" << b.GetIntValue() << ")" << std::endl;
    std::cout << "c = " << c.GetDoubleValue() << " (" << c.GetIntValue() << ")" << std::endl;
    std::cout << "d = " << d.GetDoubleValue() << " (" << d.GetIntValue() << ")" << std::endl;

    std::cout << "Configuration of d: ";
    d.printConfig();


    return 0;
}
