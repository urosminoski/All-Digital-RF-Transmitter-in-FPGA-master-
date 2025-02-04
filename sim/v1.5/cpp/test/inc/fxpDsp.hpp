#ifndef QUANTIZE_HPP
#define QUANTIZE_HPP

#include <vector>
#include <complex>
#include <stdexcept>
#include <ac_fixed.h>     
#include <ac_complex.h> 

// Function to quantize real values
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void quantizeReal(std::vector<double>& input, 
                  std::vector<double>& output);

// Function to quantize complex values
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void quantizeComplex(std::vector<std::complex<double>>& input, 
                     std::vector<std::complex<double>>& output);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void firReal(std::vector<double>& input,
             std::vector<double>& firCoeff);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void firComplex(std::vector<std::complex<double>>& input,
                std::vector<double>& firCoeff);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolatorReal(std::vector<double>& signal,
                      std::vector<double>& firCoeff, 
                      size_t interpolationRatio);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolatorComplex(std::vector<std::complex<double>>& signal,
                         std::vector<double>& firCoeff, 
                         size_t interpolationRatio);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolationReal(std::vector<double>& signal,
                       std::vector<std::vector<double>>& firCoeffs);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolationComplex(std::vector<std::complex<double>>& signal,
                          std::vector<std::vector<double>>& firCoeffs);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void delayReal(std::vector<double>& signal, 
               std::vector<double>& firCoeff, 
               size_t interpolationRatio,
               size_t delayRatio);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void delayComplex(std::vector<std::complex<double>>& signal, 
                  std::vector<double>& firCoeff, 
                  size_t interpolationRatio,
                  size_t delayRatio);

#include "../src/fxpDsp.tpp"

#endif // QUANTIZE_HPP
