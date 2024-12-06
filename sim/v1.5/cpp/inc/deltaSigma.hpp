#ifndef DELTASIGMA_HPP
#define DELTASIGMA_HPP

#include <iostream>
#include <vector>
#include <ac_fixed.h>

/**
 * @brief Processes a vector of double values using a delta-sigma modulation algorithm.
 *
 * @param x The input vector of doubles to be processed.
 * @param n_word The word length for fixed-point representation.
 * @param n_frac The fractional length for fixed-point representation.
 */
void deltaSigma(std::vector<double>& x, std::vector<double>& y);

#endif // DELTASIGMA_HPP
