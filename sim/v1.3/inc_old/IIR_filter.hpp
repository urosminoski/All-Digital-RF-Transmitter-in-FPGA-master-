#ifndef IIR_FILTER_H
#define IIR_FILTER_H

#include <iostream>
#include <vector>
#include "../inc/FixedPoint.hpp"

void fixedPointDirectOne_IIR(const std::vector<fxp::FixedPoint> &inSignal,
                            const std::vector<fxp::FixedPoint> &b,
                            const std::vector<fxp::FixedPoint> &a,
                            std::vector<fxp::FixedPoint> &outSignal);

#endif  // IIR_FILTER_H 