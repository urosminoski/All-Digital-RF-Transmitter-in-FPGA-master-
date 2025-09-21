#include "fxpDsp.hpp"

// Internal base class for ac_fixed types
class FxpBase {
public:
    virtual ~FxpBase() = default;

    // Method to return the stored value as a double
    virtual double getDouble() const = 0;
};

// Derived template class for specific ac_fixed types
template<int LocalW, int LocalI, bool LocalS, ac_q_mode LocalQ, ac_o_mode LocalO>
class FxpDerived : public FxpBase {
private:
    ac_fixed<LocalW, LocalI, LocalS, LocalQ, LocalO> value;

public:
    explicit FxpDerived(double v) : value(v) {}
    double getDouble() const override { return value.to_double(); }
};

// Factory class for creating ac_fixed instances dynamically
template<int W, bool S, ac_q_mode Q, ac_o_mode O>
class FxpFactory {
private:
    constexpr static int IStart = 1;

    static int calculateI(double value) {
        if (value < 0) { value = -value; }
        return static_cast<int>(std::ceil(std::log2(value + 1)) + 1);
    }

    template<int... IValues>
    std::unique_ptr<FxpBase> createFixedImpl(double value, int targetI, std::integer_sequence<int, IValues...>) const {
        std::unique_ptr<FxpBase> result = nullptr;

        ([&]() {
            if (targetI == IValues) {
                result = std::make_unique<FxpDerived<W, IValues, S, Q, O>>(value);
            }
        }(), ...);

        if (!result) {
            throw std::runtime_error("Unsupported I value");
        }
        return result;
    }

    double convertValue(double value) const {
        constexpr int IEnd = W;
        int targetI = calculateI(value);

        if (targetI < IStart || targetI > IEnd) {
            throw std::runtime_error("Value out of range for I");
        }

        auto fixedObj = createFixedImpl(value, targetI, std::make_integer_sequence<int, IEnd + 1 - IStart>());
        return fixedObj->getDouble();
    }

public:
    std::vector<std::vector<double>> convertMatrix(const std::vector<std::vector<double>>& matrix) const {
        std::vector<std::vector<double>> result = matrix;
        for (size_t i = 0; i < matrix.size(); ++i) {
            for (size_t j = 0; j < matrix[i].size(); ++j) {
                result[i][j] = convertValue(matrix[i][j]);
            }
        }
        return result;
    }
};

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
static void quantize_realFxp(std::vector<double>& input, 
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
static void quantize_complexFxp(std::vector<std::complex<double>>& input, 
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
void quantize_real(std::vector<double>& input,
                  std::vector<double>& output) {
    // Ensure input vector isn't empty
    if (input.empty()) {
        throw std::runtime_error("Input is empty!");
    }
    
    // Temporary vector to store fixed-point values
    std::vector<ac_fixed<W, I, S, Q, O>> outputFxp;

    // Quantize input doubles to fixed-point values
    quantize_realFxp<W, I, S, Q, O>(input, outputFxp);

    // Convert fixed-point values back to doubles
    fxp2double<W, I, S, Q, O>(outputFxp, output);
}

// Wrapper function to quantize a vector of std::complex<double> and convert back to std::complex<double>
template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void quantize_complex(std::vector<std::complex<double>>& input,
                     std::vector<std::complex<double>>& output) {
    // Ensure input vector isn't empty
    if (input.empty()) {
        throw std::runtime_error("Input is empty!");
    }
    
    // Temporary vector to store fixed-point complex values
    std::vector<ac_complex<ac_fixed<W, I, S, Q, O>>> outputFxp;

    // Quantize input complex values to fixed-point complex values
    quantize_complexFxp<W, I, S, Q, O>(input, outputFxp);

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
void fir_real(std::vector<double>& signal,
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
    quantize_realFxp<W, I, S, Q, O>(firCoeff, firCoeffFxp);
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
static void fir_real(std::vector<double>& signal,
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
void fir_complex(std::vector<std::complex<double>>& signal,
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
    quantize_realFxp<W, I, S, Q, O>(firCoeff, firCoeffFxp);
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
static void fir_complex(std::vector<std::complex<double>>& signal,
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
void interpolator_real(std::vector<double>& signal,
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
void interpolator_complex(std::vector<std::complex<double>>& signal,
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
void interpolation_real(std::vector<double>& signal,
                        std::vector<std::vector<double>>& firCoeffs) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeffs.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    for (size_t i = 0; i < firCoeffs.size(); i++) {
        interpolator_real<W, I, S, Q, O>(signal, firCoeffs[i], 2);
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void interpolation_complex(std::vector<std::complex<double>>& signal,
                          std::vector<std::vector<double>>& firCoeffs) {
    if (signal.empty()) {
        throw std::runtime_error("input is empty!");
    }
    if (firCoeffs.empty()) {
        throw std::runtime_error("firCoeff is empty!");
    }

    for (size_t i = 0; i < firCoeffs.size(); i++) {
        interpolator_complex<W, I, S, Q, O>(signal, firCoeffs[i], 2);
    }
}

template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
void delay_real(std::vector<double>& signal, 
               std::vector<double>& firCoeff, 
               size_t interpolationRatio,
               size_t delayRatio) {
    if (signal.empty()) {
        throw std::runtime_error("Input signal is empty!");
    }
    if (firCoeff.empty()) {
        throw std::runtime_error("FIR coefficients are empty!");
    }
    if (delayRatio >= interpolationRatio) {
        throw std::runtime_error("Delay cannot be greater than interpolation ratio!");
    }
    std::cout << "N = " << firCoeff.size() << std::endl;

    using CoeffType = ac_fixed<W, I, S, Q, O>;

    // Generate polyphase filter coefficients
    std::vector<std::vector<CoeffType>> polyFirCoeffFxp;
    polyFirCoeffFxp.reserve(interpolationRatio);
    makePolyFir<CoeffType>(firCoeff, polyFirCoeffFxp, interpolationRatio);

    size_t delayIndex = interpolationRatio - delayRatio - 1;
    size_t delay = static_cast<size_t>((polyFirCoeffFxp[delayIndex].size() - 1) / 2);

    // Step 1: Concatenate signal with signal[:delay]
    std::vector<double> extended_signal = signal;  // Copy original signal
    extended_signal.insert(extended_signal.end(), signal.begin(), signal.begin() + delay);

    // Step 2: Apply FIR filtering
    fir_real<W, I, S, Q, O>(extended_signal, polyFirCoeffFxp[delayIndex]);

    // Step 3: Extract signal[delay:]
    signal.assign(extended_signal.begin() + delay, extended_signal.end());
}

// template<int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
// void delay_complex(std::vector<std::complex<double>>& signal, 
//                   std::vector<double>& firCoeff, 
//                   size_t interpolationRatio,
//                   size_t delayRatio) {
//     if (signal.empty()) {
//         throw std::runtime_error("input is empty!");
//     }
//     if (firCoeff.empty()) {
//         throw std::runtime_error("firCoeff is empty!");
//     }
//     if (delayRatio >= interpolationRatio) {
//         throw std::runtime_error("Delay cannot be greater then interpolation ratio!");
//     }

//     using CoeffType = ac_fixed<W, I, S, Q, O>;

//     std::vector<std::vector<CoeffType>> polyFirCoeffFxp;
//     polyFirCoeffFxp.reserve(interpolationRatio);
//     makePolyFir<CoeffType>(firCoeff, polyFirCoeffFxp, interpolationRatio);

//     size_t delayIndex = interpolationRatio - delayRatio - 1;
//     fir_complex<W, I, S, Q, O>(signal, polyFirCoeffFxp[delayIndex]);
// }

template<typename SignalType, typename CoeffType>
static void iirParallel_single(SignalType& input, SignalType& output,
                               const std::vector<std::vector<CoeffType>>& iirCoeff,
                               std::vector<std::vector<SignalType>>& delayLine) {

    output = 0;
    for (size_t i = 0; i < iirCoeff.size(); i++) {
        delayLine[i][0] = input - CoeffType(iirCoeff[i][4])*delayLine[i][1] -
                                  CoeffType(iirCoeff[i][5])*delayLine[i][2];
        output += CoeffType(iirCoeff[i][0])*delayLine[i][0] +
                  CoeffType(iirCoeff[i][1])*delayLine[i][1] +
                  CoeffType(iirCoeff[i][2])*delayLine[i][2];
        delayLine[i][2] = delayLine[i][1];
        delayLine[i][1] = delayLine[i][0];
    }
}

template<int in_w, int in_i, int iir_w, int iir_i, int out_w,
         bool S, ac_q_mode Q, ac_o_mode O>
void deltaSigma_real(std::vector<double>& signal,
                     const std::vector<std::vector<double>>& iirCoeff,
                     const std::string mid_tread,
                     const bool full_precision) {
    // Ensure signal is not empty
    if (signal.empty()) {
        throw std::runtime_error("Input signal is empty!");
    }
    // Ensure IIR coeff is not empty
    if (iirCoeff.empty()) {
        throw std::runtime_error("IIR coeffitients are empty!");
    }

    using CoeffType     = ac_fixed<iir_w, iir_i, S, Q, O>;
    using SignalType    = ac_fixed<in_w + iir_w, in_i + iir_i, S, Q, O>;
    using InType        = ac_fixed<in_w, in_i, S, Q, O>;
    using OutType       = ac_fixed<out_w, out_w, S, Q, O>; 

    // Discretize IIR coeffitients
    std::vector<std::vector<double>> iirCoeffs_double;
    iirCoeffs_double.reserve(iirCoeff.size());
    if (!full_precision) {
        for (size_t i = 0; i < iirCoeff.size(); i++) {
            std::vector<double> row;
            row.reserve(iirCoeff[i].size());
            for (size_t j = 0; j < iirCoeff[i].size(); j++) {
                row.emplace_back(CoeffType(iirCoeff[i][j]).to_double());
            }
            iirCoeffs_double.emplace_back(row);
        }
    } else {
        FxpFactory<iir_w, S, Q, O> factory;
        iirCoeffs_double = factory.convertMatrix(iirCoeff);
    }

    using CoeffType_2   = ac_fixed<32, 12, S, Q, O>;
    std::vector<std::vector<CoeffType_2>> iirCoeffs_fxp;
    iirCoeffs_fxp.reserve(iirCoeffs_double.size());
    for (size_t i = 0; i < iirCoeffs_double.size(); i++) {
        std::vector<CoeffType_2> row;
        row.reserve(iirCoeffs_double.size());
        for (size_t j = 0; j < iirCoeffs_double[i].size(); j++) {
            row.emplace_back(iirCoeffs_double[i][j]);
        }
        iirCoeffs_fxp.emplace_back(row);
    }

    // Initialize variables for intermediate and feedback computations
    SignalType iirOutput = 0, error = 0, intermediateOutput = 0;
    OutType outputSample = 0;
    // Initialize IIR delay lines
    std::vector<std::vector<SignalType>> delayLine(
        iirCoeff.size(),
        std::vector<SignalType>(3, SignalType())
    );

    for (auto& sample : signal) {
        InType fxpSample = sample;
        if (mid_tread == "mid-tread") {
            intermediateOutput = fxpSample + iirOutput;
        } else {
            intermediateOutput  = fxpSample + iirOutput + (SignalType)0.5; // Addition of 0.5 LSB if mid-rise
        }
        outputSample        = intermediateOutput;
        sample = outputSample.to_double();      // In-place processing
        error = intermediateOutput - outputSample;
        iirParallel_single<SignalType, CoeffType_2>(error, iirOutput, iirCoeffs_fxp, delayLine);
    }
}

template <size_t bitW>
void serialConverter(std::vector<double>& signal,
                     std::vector<std::vector<double>>& LUT) {
    // Validate LUT: Check that LUT is not empty
    if (LUT.empty()) {
        throw std::invalid_argument("LUT cannot be empty!");
    }

    // Validate LUT: Check that all rows have the same number of columns
    size_t columSize = LUT[0].size();
    for (const auto& row : LUT) {
        if (row.empty()) {
            throw std::invalid_argument("LUT rows cannot be empty!");
        }
        if (row.size() != columSize) {
            throw std::invalid_argument("All rows in LUT must have the same size!");
        }
    }

    int correction = 1 << (bitW - 1);
    std::vector<double> signalSerial;
    signalSerial.reserve(signal.size() * columSize);

    for (const auto& val : signal) {
        auto correctedValue = val + correction;

        if (correctedValue < 0 || static_cast<size_t>(correctedValue) >= LUT.size()) {
            throw std::out_of_range("Input value results in out-of-range LUT index!");
        }

        size_t position = static_cast<size_t>(correctedValue);
        const auto& lutRow = LUT[LUT.size() - 1 - position];
        signalSerial.insert(signalSerial.end(), lutRow.begin(), lutRow.end());
    }
    signal = std::move(signalSerial);
}

void rfiq(const std::vector<double>& realPart,
          const std::vector<double>& imagPart,
          std::vector<double>& recSignal) {
    // Validate realPart and imagPart: Check that they are not empty, and are equal in size
    if (realPart.empty() || imagPart.empty()) {
        throw std::invalid_argument("Input signals cannot be empty!");
    }
    if (realPart.size() != imagPart.size()) {
        throw std::invalid_argument("Input signals must be same in size!");
    }

    std::vector<double> result;
    result.reserve(realPart.size() * 4);
    for (size_t i = 0; i < realPart.size(); i++) {
        result.emplace_back(realPart[i]);
        result.emplace_back(imagPart[i]);
        result.emplace_back(-realPart[i]);
        result.emplace_back(-imagPart[i]);
    }

    recSignal = std::move(result);
}

