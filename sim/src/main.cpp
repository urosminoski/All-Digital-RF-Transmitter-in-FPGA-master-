#include <iostream>
#include <iomanip>

#include "../inc/fixed.hpp"
#include "../inc/math.hpp"
#include "../inc/ios.hpp"

namespace fpm {
    using fixed_16_16 = fixed<std::int32_t, std::int64_t, 16>;  // Q16.16 format
    using fixed_24_8  = fixed<std::int32_t, std::int64_t, 8>;   // Q24.8 format
    using fixed_8_24  = fixed<std::int32_t, std::int64_t, 24>;  // Q8.24 format
}

int main()
{
    fpm::fixed_16_16 a {2.3425};
    std::cout << std::setprecision(3) << std::scientific << a << std::endl;
    
    return 0;
}
