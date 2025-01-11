#include <iostream>
#include <ac_fixed.h>
#include <ac_complex.h>
#include <complex>

int main() {
    using SignalType = ac_complex<ac_fixed<12, 4, true, AC_RND, AC_SAT>>;
    using SignalType2 = ac_fixed<12, 4, true, AC_RND, AC_SAT>;

    SignalType a;
    std::complex<double> b(1.92341231234, -0.23458762123123);

    a = SignalType(0.0, 0.0);

    // Output the real and imaginary parts
    std::cout << "Real = " << a.r().to_double() << ", Imag = " << a.i().to_double() << std::endl;

    // // Access the underlying type
    // using UnderlyingType = typename SignalType::value_type;

    // std::cout << "W = " << UnderlyingType::width << ", I = " << UnderlyingType::i_width << std::endl;

    return 0;
}
