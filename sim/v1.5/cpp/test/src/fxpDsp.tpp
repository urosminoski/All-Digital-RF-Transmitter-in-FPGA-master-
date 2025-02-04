#include "fxpDsp.hpp"

// Converts a vector of ac_fixed fixed-point numbers to a vector of doubles
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
static void fxp2double(std::vector<ac_fixed<W, I, S, Q, O>>& input,
                       std::vector<double>& output) {
    // Ensure input vector isn't empty
    if (input.empty()) {
        throw std::runtime_error("Input is empty!");
    }

    // Convert each ac_fixed value to a double and store in the output vector
    for (const auto& valFxp : input) {
        output.emplace_back(valFxp.to_double());
    }
}

// Converts a vector of ac_complex fixed-point numbers to a vector of std::complex<double>
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
static void fxp2complex(std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>& input,
                        std::vector<std::complex<double>>& output) {
    // Ensure input vector isn't empty
    if (input.empty()) {
        throw std::runtime_error("Input is empty!");
    }

    // Convert each ac_complex value to std::complex<double> and store in the output vector
    for (const auto& valFxp : input) {
        output.emplace_back(
            valFxp.r().to_double(), // Real part
            valFxp.i().to_double()  // Imaginary part
        );
    }
}

// Quantizes a vector of doubles into a vector of ac_fixed fixed-point numbers
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
static void quantizeRealFxp(std::vector<double>& input, 
                            std::vector<ac_fixed<W, I, S, Q, O>>& output) {
    // Ensure input vector isn't empty
    if (input.empty()) {
        throw std::runtime_error("Input is empty!");
    }

    // Convert each double value to ac_fixed and store in the output vector
    for (const auto& val : input) {
        output.emplace_back(ac_fixed<W, I, S, Q, O>(val));
    }
}

// Quantizes a vector of std::complex<double> into a vector of ac_complex fixed-point numbers
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
static void quantizeComplexFxp(std::vector<std::complex<double>>& input, 
                               std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>>& output) {
    // Ensure input vector isn't empty
    if (input.empty()) {
        throw std::runtime_error("Input is empty!");
    }

    // Reserve space for the output vector to avoid reallocations
    output.reserve(input.size());

    // Convert each std::complex<double> value to ac_complex and store in the output vector
    for (const auto& val : input) {
        output.emplace_back(
            ac_fixed<W, I, S, Q, O>(val.real()), // Quantize the real part
            ac_fixed<W, I, S, Q, O>(val.imag())  // Quantize the imaginary part
        );
    }
}

// Wrapper function to quantize a vector of doubles and convert back to doubles
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void quantizeReal(std::vector<double>& input,
                  std::vector<double>& output) {
    // Ensure input vector isn't empty
    if (input.empty()) {
        throw std::runtime_error("Input is empty!");
    }
    
    // Temporary vector to store fixed-point values
    std::vector<ac_fixed<W, I, S, Q, O>> outputFxp;

    // Quantize input doubles to fixed-point values
    quantizeRealFxp<W, I, S, Q, O>(input, outputFxp);

    // Convert fixed-point values back to doubles
    fxp2double<W, I, S, Q, O>(outputFxp, output);
}

// Wrapper function to quantize a vector of std::complex<double> and convert back to std::complex<double>
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void quantizeComplex(std::vector<std::complex<double>>& input,
                     std::vector<std::complex<double>>& output) {
    // Ensure input vector isn't empty
    if (input.empty()) {
        throw std::runtime_error("Input is empty!");
    }
    
    // Temporary vector to store fixed-point complex values
    std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>> outputFxp;

    // Quantize input complex values to fixed-point complex values
    quantizeComplexFxp<W, I, S, Q, O>(input, outputFxp);

    // Convert fixed-point complex values back to std::complex<double>
    fxp2complex<W, I, S, Q, O>(outputFxp, output);
}


template<typename InputType, typename SignalType, typename CoeffType>
static void firConvolution(InputType& input, InputType& output, std::vector<SignalType>& delayLine,
                           std::vector<CoeffType>& firCoeff, size_t& k) {
    size_t M = firCoeff.size();
    SignalType sum = 0;
    delayLine[k] = input;

    for (size_t i = 0; i < M; i++) {
        sum += firCoeff[i] * delayLine[k++];
        if (k == M) {
            k = 0;
        }
    }
    output = sum;
    if (k-- == 0) {
        k = M - 1;
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void firReal(std::vector<double>& signal,
             std::vector<double>& firCoeff) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeff.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    using InputType     = ac_fixed<W, I, S, Q, O>;
    using SignalType    = ac_fixed<2*W, 2*I, S, Q, O>;
    using CoeffType     = ac_fixed<W, I, S, Q, O>;

    // Quantize firCoeff
    std::vector<CoeffType> firCoeffFxp;
    quantizeRealFxp<W, I, S, Q, O>(firCoeff, firCoeffFxp);
    // Define delayLine
    std::vector<SignalType> delayLine(firCoeff.size(), SignalType(0));
    size_t k = 0;

    for (size_t i = 0; i < signal.size(); i++) {
        InputType inputFxp = signal[i];
        InputType outputFxp = 0;;
        firConvolution<InputType, SignalType, CoeffType>(inputFxp, outputFxp, delayLine, firCoeffFxp, k);
        signal[i] = outputFxp.to_double();
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
static void firReal(std::vector<double>& signal,
                    std::vector<ac_fixed<W, I, S, Q, O>>& firCoeffFxp) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeffFxp.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    using InputType     = ac_fixed<W, I, S, Q, O>;
    using SignalType    = ac_fixed<2*W, 2*I, S, Q, O>;
    using CoeffType     = ac_fixed<W, I, S, Q, O>;

    // Define delayLine
    std::vector<SignalType> delayLine(firCoeffFxp.size(), SignalType(0));
    size_t k = 0;

    for (size_t i = 0; i < signal.size(); i++) {
        InputType inputFxp = signal[i];
        InputType outputFxp = 0;;
        firConvolution<InputType, SignalType, CoeffType>(inputFxp, outputFxp, delayLine, firCoeffFxp, k);
        signal[i] = outputFxp.to_double();
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void firComplex(std::vector<std::complex<double>>& signal,
                std::vector<double>& firCoeff) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeff.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    using InputType     = ac_complex<ac_fixed<W, I, S, Q, O>>;
    using SignalType    = ac_complex<ac_fixed<2*W, 2*I, S, Q, O>>;
    using CoeffType     = ac_fixed<W, I, S, Q, O>;

    // Quantize firCoeff
    std::vector<CoeffType> firCoeffFxp;
    quantizeRealFxp<W, I, S, Q, O>(firCoeff, firCoeffFxp);
    // Define delayLine
    std::vector<SignalType> delayLine(firCoeff.size(), SignalType(0.0));
    size_t k = 0;

    for (size_t i = 0; i < signal.size(); i++) {
        InputType inputFxp = InputType(
            signal[i].real(),
            signal[i].imag()
        );
        InputType outputFxp = 0;
        firConvolution<InputType, SignalType, CoeffType>(inputFxp, outputFxp, delayLine, firCoeffFxp, k);
        signal[i] = std::complex<double>(
            outputFxp.r().to_double(),
            outputFxp.i().to_double()
        );
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
static void firComplex(std::vector<std::complex<double>>& signal,
                       std::vector<ac_fixed<W, I, S, Q, O>>& firCoeffFxp) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeffFxp.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    using InputType     = ac_complex<ac_fixed<W, I, S, Q, O>>;
    using SignalType    = ac_complex<ac_fixed<2*W, 2*I, S, Q, O>>;
    using CoeffType     = ac_fixed<W, I, S, Q, O>;

    // Define delayLine
    std::vector<SignalType> delayLine(firCoeffFxp.size(), SignalType(0.0));
    size_t k = 0;

    for (size_t i = 0; i < signal.size(); i++) {
        InputType inputFxp = InputType(
            signal[i].real(),
            signal[i].imag()
        );
        InputType outputFxp = 0;
        firConvolution<InputType, SignalType, CoeffType>(inputFxp, outputFxp, delayLine, firCoeffFxp, k);
        signal[i] = std::complex<double>(
            outputFxp.r().to_double(),
            outputFxp.i().to_double()
        );
    }
}

template<typename CoeffType>
static void makePolyFir(std::vector<double>& firCoeff, 
                        std::vector<std::vector<CoeffType>>& polyFirCoeffFxp,
                        size_t interpolationRatio) {
    if (firCoeff.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    // Copy firCoeff to avoid modifying original
    std::vector<double> firCoeff_padded = firCoeff;

    // Calculate the next power of two
    size_t targetSize = std::pow(2, std::ceil(std::log2(firCoeff_padded.size())));

    // Resize padded vector and pad it with zeros
    firCoeff_padded.resize(targetSize, 0.0);

    // Ensure firCoeff_poly has 'interpolationRatio' rows
    polyFirCoeffFxp.resize(interpolationRatio);
    // Divide coefficients into interpolationRatio parts
    for (size_t i = 0; i < firCoeff_padded.size(); i++) {
        CoeffType fxpCoeff = firCoeff_padded[i];
        polyFirCoeffFxp[i % interpolationRatio].emplace_back(fxpCoeff);
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolatorReal(std::vector<double>& signal,
                      std::vector<double>& firCoeff,
                      size_t interpolationRatio) {
    if (signal.empty()) {
        throw std::runtime_error("signal is empty!");
    }
    if (firCoeff.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    using InputType     = ac_fixed<W, I, S, Q, O>;
    using SignalType    = ac_fixed<2*W, 2*I, S, Q, O>;
    using CoeffType     = ac_fixed<W, I, S, Q, O>;

    // Quantize the FIR coefficients into polyphase form
    std::vector<std::vector<CoeffType>> polyFirCoeffFxp;
    polyFirCoeffFxp.reserve(interpolationRatio);
    makePolyFir<CoeffType>(firCoeff, polyFirCoeffFxp, interpolationRatio);

    // Initialize delay lines for each polyphase branch
    std::vector<std::vector<SignalType>> delayLine(
        interpolationRatio,
        std::vector<SignalType>(polyFirCoeffFxp[0].size(), SignalType(0.0))
    );
    std::vector<size_t> k(interpolationRatio, 0); // Circular buffer index for each phase

    // Prepare an extended signal for interpolation
    std::vector<double> extendedSignal;
    extendedSignal.reserve(signal.size() * interpolationRatio);

    // Iterate over the original signal
    for (size_t i = 0; i < signal.size(); ++i) {
        for (size_t j = 0; j < interpolationRatio; ++j) {
            InputType inputFxp = signal[i]; // Quantized input sample
            InputType outputFxp = 0.0;

            // Perform convolution for the current polyphase filter
            firConvolution<InputType, SignalType, CoeffType>(inputFxp, outputFxp, delayLine[j], polyFirCoeffFxp[j], k[j]);

            // Append the interpolated value to the extended signal
            extendedSignal.emplace_back(outputFxp.to_double());
        }
    }

    // Replace the original signal with the extended/interpolated signal
    signal = std::move(extendedSignal);
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolatorComplex(std::vector<std::complex<double>>& signal,
                         std::vector<double>& firCoeff, 
                         size_t interpolationRatio) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeff.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    using InputType     = ac_complex<ac_fixed<W, I, S, Q, O>>;
    using SignalType    = ac_complex<ac_fixed<2*W, 2*I, S, Q, O>>;
    using CoeffType     = ac_fixed<W, I, S, Q, O>;

    // Make quantizied polyphase FIR
    std::vector<std::vector<CoeffType>> polyFirCoeffFxp;
    polyFirCoeffFxp.reserve(interpolationRatio);
    makePolyFir<CoeffType>(firCoeff, polyFirCoeffFxp, interpolationRatio);

    std::vector<std::vector<SignalType>> delayLine(
        interpolationRatio,
        std::vector<SignalType>(polyFirCoeffFxp[0].size(), SignalType(0.0))
    );
    std::vector<size_t> k(interpolationRatio, 0);   // Circular buffer index for each phase

    // Prepare an extended signal for interpolation
    std::vector<std::complex<double>> extendedSignal;
    extendedSignal.reserve(signal.size() * interpolationRatio);

    for (size_t i = 0; i < signal.size(); i++) {
        for (size_t j = 0; j < interpolationRatio; j++) {
            InputType inputFxp = InputType(
                signal[i].real(),
                signal[i].imag()
            );
            InputType outputFxp = 0.0;
            firConvolution<InputType, SignalType, CoeffType>(inputFxp, outputFxp, delayLine[j], polyFirCoeffFxp[j], k[j]);
            extendedSignal.emplace_back(
                outputFxp.r().to_double(),
                outputFxp.i().to_double()
            );
        }
    }
    // Replace the original signal with the extended/interpolated signal
    signal = std::move(extendedSignal);
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolationReal(std::vector<double>& signal,
                       std::vector<std::vector<double>>& firCoeffs) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeffs.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    for (size_t i = 0; i < firCoeffs.size(); i++) {
        interpolatorReal<W, I, S, Q, O>(signal, firCoeffs[i], 2);
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolationComplex(std::vector<std::complex<double>>& signal,
                          std::vector<std::vector<double>>& firCoeffs) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeffs.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    for (size_t i = 0; i < firCoeffs.size(); i++) {
        interpolatorComplex<W, I, S, Q, O>(signal, firCoeffs[i], 2);
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void delayReal(std::vector<double>& signal, 
               std::vector<double>& firCoeff, 
               size_t interpolationRatio,
               size_t delayRatio) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeff.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }
    if (delayRatio >= interpolationRatio) {
        throw std::runtime_error("Delay cannot be greater then interpolation ratio!");
    }

    using CoeffType = ac_fixed<W, I, S, Q, O>;

    std::vector<std::vector<CoeffType>> polyFirCoeffFxp;
    polyFirCoeffFxp.reserve(interpolationRatio);
    makePolyFir<CoeffType>(firCoeff, polyFirCoeffFxp, interpolationRatio);

    size_t delayIndex = interpolationRatio - delayRatio - 1;
    firReal<W, I, S, Q, O>(signal, polyFirCoeffFxp[delayIndex]);
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void delayComplex(std::vector<std::complex<double>>& signal, 
                  std::vector<double>& firCoeff, 
                  size_t interpolationRatio,
                  size_t delayRatio) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeff.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }
    if (delayRatio >= interpolationRatio) {
        throw std::runtime_error("Delay cannot be greater then interpolation ratio!");
    }

    using CoeffType = ac_fixed<W, I, S, Q, O>;

    std::vector<std::vector<CoeffType>> polyFirCoeffFxp;
    polyFirCoeffFxp.reserve(interpolationRatio);
    makePolyFir<CoeffType>(firCoeff, polyFirCoeffFxp, interpolationRatio);

    size_t delayIndex = interpolationRatio - delayRatio - 1;
    firComplex<W, I, S, Q, O>(signal, polyFirCoeffFxp[delayIndex]);
}