#ifndef QUANTIZE_HPP
#define QUANTIZE_HPP

#include <vector>
#include <complex>
#include <stdexcept>
#include <ac_fixed.h>     
#include <ac_complex.h> 

// Function to quantize real values
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void quantize_real(std::vector<double>& input, 
                  std::vector<double>& output);

// Function to quantize complex values
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void quantize_complex(std::vector<std::complex<double>>& input, 
                     std::vector<std::complex<double>>& output);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void fir_real(std::vector<double>& input,
             std::vector<double>& firCoeff);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void fir_complex(std::vector<std::complex<double>>& input,
                std::vector<double>& firCoeff);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolator_real(std::vector<double>& signal,
                       std::vector<double>& firCoeff, 
                       size_t interpolationRatio);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolator_complex(std::vector<std::complex<double>>& signal,
                          std::vector<double>& firCoeff, 
                          size_t interpolationRatio);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolation_real(std::vector<double>& signal,
                        std::vector<std::vector<double>>& firCoeffs);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolation_complex(std::vector<std::complex<double>>& signal,
                           std::vector<std::vector<double>>& firCoeffs);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void delay_real(std::vector<double>& signal, 
               std::vector<double>& firCoeff, 
               size_t interpolationRatio,
               size_t delayRatio);

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void delay_complex(std::vector<std::complex<double>>& signal, 
                  std::vector<double>& firCoeff, 
                  size_t interpolationRatio,
                  size_t delayRatio);

template<int in_w, int in_i, int iir_w, int iir_i, int out_w,
         bool s, ac_q_mode Q, ac_o_mode O>
void deltaSigma_real(std::vector<double>& signal,
                     const std::vector<std::vector<double>>& iirCoeff,
                     const bool full_precision = false);

void rfiq(const std::vector<double>& realPart,
          const std::vector<double>& imagPart,
          std::vector<double>& recSignal);

#include "../src/fxpDsp.tpp"

#endif // QUANTIZE_HPP
