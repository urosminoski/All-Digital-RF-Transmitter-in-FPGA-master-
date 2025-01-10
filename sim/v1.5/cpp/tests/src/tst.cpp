#include <iostream>
#include <memory>
#include <cmath> // For log2
#include <ac_fixed.h>

// Abstract base class
class FixedBase {
public:
    virtual ~FixedBase() = default;

    // Method to return the stored value as a double
    virtual double getValueAsDouble() const = 0;
};

// Derived template class for specific ac_fixed types
template<int W, int I, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FixedDerived : public FixedBase {
private:
    ac_fixed<W, I, S, Q, O> value;

public:
    // Constructor to initialize the value
    explicit FixedDerived(double v) : value(v) {}

    // Return the stored value as a double
    double getValueAsDouble() const override {
        return value.to_double();
    }
};

// Template class for the factory
template<int W, bool S = true, ac_q_mode Q = AC_RND, ac_o_mode O = AC_SAT>
class FixedFactory {
private:
    constexpr static int IStart = 1; // Minimum I value

    // Internal helper function to calculate I
    static int calculateI(double value) {
        int Inew = static_cast<int>(std::ceil(std::log2(value + 1)) + 1);
        std::cout << Inew << std::endl;
        return Inew;
    }

    // Internal helper template for handling different I values
    template<int... IValues>
    std::unique_ptr<FixedBase> createFixedImpl(double value, int targetI, std::integer_sequence<int, IValues...>) const {
        std::unique_ptr<FixedBase> result = nullptr;

        // Use a fold expression with a lambda
        ([&]() {
            if (targetI == IValues) {
                result = std::make_unique<FixedDerived<W, IValues, S, Q, O>>(value);
            }
        }(), ...);

        if (!result) {
            throw std::runtime_error("Unsupported I value");
        }
        return result;
    }

public:
    // Public method to create a FixedBase object
    std::unique_ptr<FixedBase> createFixed(double value) const {
        constexpr int IEnd = W; // Maximum I value

        // Calculate I dynamically as int(log2(int(value) + 1))
        int targetI = calculateI(value);

        // Validate targetI
        if (targetI < IStart || targetI > IEnd) {
            throw std::runtime_error("Value out of range for I");
        }

        // Use the internal helper template to create the object
        return createFixedImpl(value, targetI, std::make_integer_sequence<int, IEnd + 1 - IStart>());
    }
};

int main() {
    try {
        // Create a factory with specific W, S, Q, and O parameters
        FixedFactory<8, true, AC_RND, AC_SAT> factory;

        // Example input values
        double value1 = 5.2384923754875; // Maps to I = int(log2(5 + 1)) = 2
        double value2 = 0.2384923754875; // Maps to I = int(log2(15 + 1)) = 4

        // Use the factory to create FixedBase objects
        std::unique_ptr<FixedBase> fixed1 = factory.createFixed(value1);
        std::unique_ptr<FixedBase> fixed2 = factory.createFixed(value2);

        // Access their values as doubles
        std::cout << "fixed1 as double: " << fixed1->getValueAsDouble() << std::endl;
        std::cout << "fixed2 as double: " << fixed2->getValueAsDouble() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}
