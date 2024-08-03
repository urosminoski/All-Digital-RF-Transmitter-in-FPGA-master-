#include <iostream>
#include "../inc/FixedPoint.hpp"

// Test the FixedPoint class
int main() {
    FixedPoint a(0.125, 5, 4, OverflowMode::Saturate);
    FixedPoint b(0.75, 5, 4, OverflowMode::Saturate);
    FixedPoint c(0.2, 5, 4, OverflowMode::Saturate);

    FixedPoint tmp(0.0195312, 5, 4, OverflowMode::Saturate);

    std::cout << "a (Saturate) = " << a.ToDouble() << " (" << a.ToInt() << ")" << std::endl;
    a.PrintConfig();
    std::cout << "b (Saturate) = " << b.ToDouble() << " (" << b.ToInt() << ")" << std::endl;
    b.PrintConfig();
    std::cout << "c (Saturate) = " << c.ToDouble() << " (" << c.ToInt() << ")" << std::endl;
    c.PrintConfig();
    std::cout << "tmp (Saturate) = " << tmp.ToDouble() << " (" << tmp.ToInt() << ")" << std::endl;
    tmp.PrintConfig();

    FixedPoint c1(15, 12, OverflowMode::Saturate);
    FixedPoint c2(5, 4, OverflowMode::Saturate);

    c1 = a*b*c;
    c2 = a*b*c;

    std::cout << "c1 (Saturate) = " << c1.ToDouble() << " (" << c1.ToInt() << ")" << std::endl;
    c1.PrintConfig();
    std::cout << "c2 (Saturate) = " << c2.ToDouble() << " (" << c2.ToInt() << ")" << std::endl;
    c2.PrintConfig();

    FixedPoint d1(10, 8, OverflowMode::Saturate);
    FixedPoint d2(5, 4, OverflowMode::Saturate);

    d1 = a+b+c;
    d2 = a+b+c;

    std::cout << "d1 (Saturate) = " << d1.ToDouble() << " (" << d1.ToInt() << ")" << std::endl;
    d1.PrintConfig();
    std::cout << "d2 (Saturate) = " << d2.ToDouble() << " (" << d2.ToInt() << ")" << std::endl;
    d2.PrintConfig();

    

    return 0;
}
