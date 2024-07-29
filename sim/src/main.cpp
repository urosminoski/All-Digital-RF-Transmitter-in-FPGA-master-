#include <iostream>
#include <cstdint>
#include <type_traits>

// Helper functions for conversion
template<size_t db>
int32_t DoubleToFixed(double d) {
    return int32_t(d * double(1 << db) + (d >= 0 ? 0.5 : -0.5));
}

template<size_t db>
double FixedToDouble(int32_t d) {
    return double(d) / double(1 << db);
}

// FixedPoint class
template<size_t db>
class FixedPoint {
public:
    // Constructors
    FixedPoint() : value(0) {}
    FixedPoint(double d) : value(DoubleToFixed<db>(d)) {}

    // Conversion to double
    operator double() const {
        return FixedToDouble<db>(value);
    }

    // Arithmetic operators
    FixedPoint operator+(const FixedPoint& other) const {
        return FixedPoint::fromRawValue(value + other.value);
    }

    FixedPoint operator-(const FixedPoint& other) const {
        return FixedPoint::fromRawValue(value - other.value);
    }

    FixedPoint operator*(const FixedPoint& other) const {
        return FixedPoint::fromRawValue((int64_t(value) * int64_t(other.value)) >> db);
    }

    FixedPoint operator/(const FixedPoint& other) const {
        return FixedPoint::fromRawValue((int64_t(value) << db) / other.value);
    }

    // Output operator for printing
    friend std::ostream& operator<<(std::ostream& os, const FixedPoint& fp) {
        return os << double(fp);
    }

private:
    int32_t value;

    // Private constructor to create from raw value
    constexpr static FixedPoint fromRawValue(int32_t rawValue) {
        FixedPoint fp;
        fp.value = rawValue;
        return fp;
    }
};

int main() {
    // Example usage of the FixedPoint class
    FixedPoint<2> a(2.25);
    FixedPoint<2> b(4.0);
    FixedPoint<2> c = a + b;
    FixedPoint<2> d = a * b;

    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;
    std::cout << "c (a + b): " << c << std::endl;
    std::cout << "d (a * b): " << d << std::endl;

    return 0;
}
