#include <iostream>
#include <sg14/fixed_point>
#include <typeinfo>
#include <cxxabi.h>

int main() {
    // Create a fixed-point number with 12 integer bits and 7 fractional bits
    auto x = sg14::make_fixed<4, 6>{1.25};

    // Output the underlying representation and the actual value
    std::cout << "x.data() (raw integer) = " << static_cast<int>(x.data()) << std::endl;
    std::cout << "x (fixed-point value) = " << x << std::endl;

    // Print the type of the fixed-point object
    int status;
    char* demangled = abi::__cxa_demangle(typeid(x).name(), 0, 0, &status);
    std::cout << "Type of x: " << (status == 0 ? demangled : typeid(x).name()) << std::endl;
    free(demangled);

    return 0;
}
