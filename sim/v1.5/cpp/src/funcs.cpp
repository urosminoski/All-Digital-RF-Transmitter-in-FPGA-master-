#include "funcs.hpp"

using json = nlohmann::json;

std::vector<double> firCoeff = {
0.005720593689315046,
-0.021303333540937264,
-0.0017849974158785041,
0.04752645856668174,
-0.04105077600834315,
-0.06083656208222833,
0.13000914816799372,
0.002578349420800305,
-0.23090162792797114,
0.18266656433539388,
0.2431148740719331,
-0.5037150475047983,
-0.0040435708662145495,
0.8908853266274948,
-0.7710997638433279,
-1.2120859107436242,
4.001428420654369,
7.999999999999999,
4.001428420654369,
-1.2120859107436242,
-0.7710997638433279,
0.8908853266274948,
-0.0040435708662145495,
-0.5037150475047983,
0.2431148740719331,
0.18266656433539388,
-0.23090162792797114,
0.002578349420800305,
0.13000914816799372,
-0.06083656208222833,
-0.04105077600834315,
0.04752645856668174,
-0.0017849974158785041,
-0.021303333540937264,
0.005720593689315046
};

static bool parseDataLine(const std::string& line, bool isComplex, std::vector<double>& realData, std::vector<std::complex<double>>& complexData);
static void readMetadata(std::ifstream& inputFile, std::map<std::string, double>& metadata);
// static bool detectFormat(const std::string& line, bool& isComplex);
// static void fir(std::vector<double>&, std::vector<double>&, std::vector<double>&);
static void deltaSigma(const std::vector<double>&, std::vector<double>&);
// static void parallelToSerialConverter(const std::vector<int>&, const std::vector<std::vector<int>>&, std::vector<int>&);


/**
 * @brief Reads metadata from an input file and rewinds the file pointer to the first data line.
 * 
 * @param inputFile An open input file stream.
 * @param metadata A map to store key-value pairs from the metadata.
 * 
 * @details
 * Processes all lines starting with '#' as metadata. Stops reading when a non-metadata
 * line is encountered and rewinds the file pointer so that the main function can process the data lines.
 */
void readMetadata(std::ifstream& inputFile, std::map<std::string, double>& metadata) {
    metadata.clear(); // Ensure the metadata map is empty before processing
    std::string line;

    while (std::getline(inputFile, line)) {
        // Check if the line is metadata (starts with '#')
        if (line.empty() || line[0] != '#') {
            // If the line is not metadata, rewind the file pointer
            inputFile.seekg(-static_cast<int>(line.size()) - 1, std::ios::cur);
            break; // Exit the loop as we've reached the data section
        }

        // Process metadata lines in the format "#key=value"
        size_t delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(1, delimiter_pos - 1); // Extract key
            std::string value_str = line.substr(delimiter_pos + 1); // Extract value
            try {
                metadata[key] = std::stod(value_str); // Convert value to double
            } catch (const std::invalid_argument&) {
                std::cerr << "Invalid value in metadata: " << value_str << std::endl;
            }
        } else {
            std::cerr << "Malformed metadata line: " << line << std::endl;
        }
    }
}

/**
 * @brief Parses a line of data and stores it in the appropriate container.
 * 
 * @param line A string containing a single line of data to parse.
 * @param isComplex A boolean indicating whether the line should be treated as complex data (true)
 *                  or real data (false).
 * @param realData A vector to store parsed real data (if `isComplex` is false).
 * @param complexData A vector to store parsed complex data (if `isComplex` is true).
 * 
 * @details
 * This function uses `std::strtold` to parse each numerical value from the input line with 
 * maximum precision supported by `double`. It handles both real and complex data formats.
 * 
 * - For real data, a single numerical value is expected.
 * - For complex data, exactly two numerical values (real and imaginary parts) are expected.
 * 
 * The function performs robust error checking to ensure data integrity and skips whitespace
 * between values.
 * 
 * @return True if the line is successfully parsed; false otherwise.
 */
bool parseDataLine(const std::string& line, bool isComplex,
                   std::vector<double>& realData,
                   std::vector<std::complex<double>>& complexData) {
    // Pointer to the current position in the string
    const char* str = line.c_str();
    // Pointer to store the position after parsing a value
    char* end;
    // Container to store all parsed values from the line
    std::vector<double> values;

    // Reset errno to catch any parsing errors
    errno = 0;

    // Loop through the line and parse each number
    while (*str != '\0') { // Continue until the end of the string
        // Parse a number as double
        double value = std::strtold(str, &end);

        // Check for parsing errors
        if (errno != 0 || str == end) { // errno != 0 indicates an error, or no number was parsed
            std::cerr << "Error: Failed to parse value in line: " << line << std::endl;
            return false;
        }

        // Add the parsed value to the container
        values.push_back(value);

        // // Debug output for the parsed value
        // std::cout << "Parsed value: " << std::setprecision(std::numeric_limits<double>::digits10 + 1) 
        //           << value << std::endl;

        // Move the pointer to the position after the parsed value
        str = end;

        // Skip any whitespace characters between values
        while (*str == ' ') ++str;
    }

    // Check if the data is complex and has exactly two values (real and imaginary parts)
    if (isComplex && values.size() == 2) {
        // Add the parsed values as a complex number
        complexData.emplace_back(values[0], values[1]);
        return true;
    }
    // Check if the data is real and has exactly one value
    else if (!isComplex && values.size() == 1) {
        // Add the parsed value to the real data container
        realData.push_back(values[0]);
        return true;
    }

    // If the line format is inconsistent, print an error
    std::cerr << "Error: Inconsistent data format in line: " << line << std::endl;
    return false;
}

/**
 * @brief Reads data and metadata from a file.
 * 
 * @param fileName The name of the input file to read.
 * @param data A variant that stores either:
 *             - `std::vector<double>`: Real data.
 *             - `std::vector<std::complex<double>>`: Complex data.
 * @param metadata A map to store metadata key-value pairs extracted from the file.
 * 
 * @return True if the file is successfully read and parsed; false otherwise.
 */
bool readFromFile(const std::string& fileName,
                  std::variant<std::vector<double>, std::vector<std::complex<double>>>& data,
                  std::map<std::string, double>& metadata) {
    std::ifstream inputFile(fileName); // Open the input file
    if (!inputFile.is_open()) {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return false;
    }

    inputFile.precision(std::numeric_limits<double>::digits10 + 1);

    metadata.clear();
    bool isComplex = false;
    bool firstDataLine = true;
    std::vector<double> realData;
    std::vector<std::complex<double>> complexData;
    std::string line;

    // Read metadata
    readMetadata(inputFile, metadata);

    // Read and parse data
    while (std::getline(inputFile, line)) {
        if (firstDataLine) {
            // Detect if the data is complex based on the first line
            std::istringstream iss(line);
            int count = 0;
            double tmp;
            while (iss >> tmp) count++;

            if (count == 2) {
                isComplex = true;
            } else if (count == 1) {
                isComplex = false;
            } else {
                std::cerr << "Error: Invalid data format in line: " << line << std::endl;
                return false;
            }
            firstDataLine = false;
        }

        // std::cout << line << std::endl;

        // Parse the line and store the data
        if (!parseDataLine(line, isComplex, realData, complexData)) {
            return false;
        }
    }

    // Assign parsed data to the variant
    if (isComplex) {
        data = std::move(complexData);
    } else {
        data = std::move(realData);
    }

    inputFile.close();
    return true;
}

/**
 * @brief Writes data and metadata to a file.
 * 
 * @param fileName The name of the output file to write.
 * @param data A variant containing either:
 *             - `std::vector<double>`: Real data.
 *             - `std::vector<std::complex<double>>`: Complex data.
 * @param metadata A map containing metadata key-value pairs to be written.
 * 
 * @return True if the file is successfully written; false otherwise.
 */
bool writeToFile(const std::string& fileName,
                 const std::variant<std::vector<double>, std::vector<std::complex<double>>>& data,
                 const std::map<std::string, double>& metadata) 
{
    std::ofstream outputFile(fileName); // Open the output file
    if (!outputFile.is_open()) {
        std::cerr << "Error: Cannot open file " << fileName << " for writing" << std::endl;
        return false;
    }

    // Set maximum precision for output to match double precision
    outputFile << std::setprecision(std::numeric_limits<double>::digits10 + 1);

    // Write metadata
    for (const auto& [key, value] : metadata) {
        outputFile << "#" << key << "=" << value << "\n";
    }

    // Write data
    std::visit(
        [&outputFile](const auto& dataVec) {
            for (const auto& value : dataVec) {
                if constexpr (std::is_same_v<std::decay_t<decltype(value)>, double>) {
                    // Write real data with full precision
                    outputFile << value << '\n';
                } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::complex<double>>) {
                    // Write complex data with full precision
                    outputFile << value.real() << " " << value.imag() << '\n';
                } else {
                    // Unexpected type, log an error
                    std::cerr << "Error: Unexpected data type during write operation.\n";
                }
            }
        },
        data);

    outputFile.close();
    return true;
}

/**
 * @brief Reads a Lookup Table (LUT) from a JSON file and parses it into a 2D vector.
 * 
 * @param fileName The name of the JSON file containing the LUT data.
 * @param LUT A reference to a 2D vector of integers that will store the parsed LUT data.
 *            The vector is cleared before parsing to ensure it contains only the new data.
 * 
 * @details
 * This function opens a JSON file, parses its content, and stores the data in the provided
 * 2D vector `LUT`. It ensures the vector is cleared before adding new data. The function
 * uses the nlohmann::json library for JSON parsing and handles errors related to file
 * access or parsing.
 * 
 * @return Returns `true` if the LUT is successfully read and parsed.
 * @throws std::runtime_error If the file cannot be opened.
 */
bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT)
{
    // Clear the LUT vector to ensure it starts empty
    LUT.clear();

    // Open the file
    std::ifstream file(fileName);

    // Check if the file can be opened successfully
    if (!file.is_open())
    {
        // Throw an exception if the file cannot be opened
        throw std::runtime_error("Could not open file: " + fileName);
        return false; // Unnecessary but included for clarity
    }

    // Create a JSON object to hold the file content
    json j;

    // Parse the file content into the JSON object
    file >> j;

    // Close the file after reading
    file.close();

    // Convert the JSON object to a 2D vector of integers
    LUT = j.get<std::vector<std::vector<int>>>();

    // Return true to indicate successful parsing
    return true;
}

// /**
//  * @brief Applies an FIR filter to real or complex data.
//  * 
//  * @param input A variant containing either:
//  *              - `std::vector<double>`: Real data to be filtered.
//  *              - `std::vector<std::complex<double>>`: Complex data to be filtered.
//  * @param firCoeff A vector of FIR filter coefficients.
//  * @param output A variant that stores the filtered output. It will contain:
//  *               - `std::vector<double>`: Filtered real data.
//  *               - `std::vector<std::complex<double>>`: Filtered complex data.
//  * 
//  * @details
//  * For real data, this function directly applies the FIR filter using the `fir` function.
//  * For complex data, it separates the real and imaginary parts, applies the FIR filter 
//  * to each part individually, and combines the filtered parts back into a complex result.
//  */
// void firWrapper(const std::variant<std::vector<double>, std::vector<std::complex<double>>>& input,
//                 const std::vector<double>& firCoeff,
//                 std::variant<std::vector<double>, std::vector<std::complex<double>>>& output)
// {
//     // Use std::visit to handle the variant input
//     std::visit(
//         [&output, &firCoeff](const auto& inputVec) {
//             using T = std::decay_t<decltype(inputVec)>;

//             if constexpr (std::is_same_v<T, std::vector<double>>) {
//                 // Real data
//                 std::vector<double> realOutput;

//                 // Apply FIR filter to real data
//                 fir(inputVec, firCoeff, realOutput);

//                 // Assign the filtered real data to the output variant
//                 output = std::move(realOutput);

//             } else if constexpr (std::is_same_v<T, std::vector<std::complex<double>>>) {
//                 // Complex data

//                 // Separate the real and imaginary parts of the complex input
//                 std::vector<double> realPart, imagPart;
//                 for (const auto& c : inputVec) {
//                     realPart.push_back(c.real());
//                     imagPart.push_back(c.imag());
//                 }

//                 // Containers for the filtered real and imaginary parts
//                 std::vector<double> realOutput, imagOutput;

//                 // Apply FIR filter to the real and imaginary parts separately
//                 fir(realPart, firCoeff, realOutput);
//                 fir(imagPart, firCoeff, imagOutput);

//                 // Combine the filtered real and imaginary parts into a complex output
//                 std::vector<std::complex<double>> complexOutput;
//                 for (size_t i = 0; i < realOutput.size(); ++i) {
//                     complexOutput.emplace_back(realOutput[i], imagOutput[i]);
//                 }

//                 // Assign the filtered complex data to the output variant
//                 output = std::move(complexOutput);
//             }
//         }, input);
// }

// /**
//  * @brief Applies an FIR filter to real-valued data.
//  * 
//  * @param x A vector of real-valued input data.
//  * @param firCoeff A vector of FIR filter coefficients.
//  * @param y A vector to store the filtered output data.
//  * 
//  * @details
//  * This function implements a discrete FIR filter using a circular buffer to manage
//  * the delay line. The filter coefficients are internally converted to a fixed-point
//  * representation for numerical precision.
//  */
// static void fir(std::vector<double>& x, std::vector<double>& firCoeff, std::vector<double>& y)
// {
//     y.clear();  // Ensure output vector is empty
    
//     // Discretize FIR coeffitients to fxFIR type
//     std::vector<fxFIR> fxFirCoeff;
//     for(size_t i = 0; i < firCoeff.size(); i++)
//     {
//         fxFirCoeff.push_back(firCoeff[i]);
//     }

//     // Perform filtering
//     std::vector<fxFIR> delayLine(fxFirCoeff.size(), fxFIR(0));  // Delay line is simulationg circular buffer
//     unsigned long int k = 0;      // Index of circular buffer

//     for(size_t n = 0; n < x.size(); n++)
//     {
//         delayLine[k] = x[n];
//         fxFIR sum = 0;

//         for(size_t m = 0; m < delayLine.size(); m++)
//         {
//             // Perform convolution
//             sum += fxFirCoeff[m] * delayLine[k++];

//             // Simulating circular buffer
//             if(k == fxFirCoeff.size())
//             {
//                 k = 0;
//             }
//         }
//         // Save sum to output vector
//         y.push_back(sum.to_double());

//         // Simulating circular buffer
//         if(k-- == 0)
//         {
//             k = fxFirCoeff.size() - 1;
//         }
//     }
// }

/**
 * Wrapper function for Delta-Sigma modulation.
 * This function processes real or complex data using the Delta-Sigma algorithm.
 * For complex data, the real and imaginary parts are processed separately.
 * 
 * @param input: A variant containing either:
 *               - std::vector<double>: Real data to be processed.
 *               - std::vector<std::complex<double>>: Complex data to be processed.
 * @param output: A variant that stores the processed result. It will contain:
 *                - std::vector<double>: Processed real data.
 *                - std::vector<std::complex<double>>: Processed complex data.
 */
void deltaSigmaWrapper(const std::variant<std::vector<double>, std::vector<std::complex<double>>>& input,
                       std::variant<std::vector<double>, std::vector<std::complex<double>>>& output)
{
    // Use std::visit to handle the variant input
    std::visit(
        [&output](const auto& inputVec) {
            // Deduce the type of inputVec (either std::vector<double> or std::vector<std::complex<double>>)
            using T = std::decay_t<decltype(inputVec)>;

            // Handle real data
            if constexpr (std::is_same_v<T, std::vector<double>>) {
                // Create a container for the delta-sigma output
                std::vector<double> realOutput;

                // Process the real data using deltaSigma
                deltaSigma(inputVec, realOutput);

                // Store the real result in the output variant
                output = std::move(realOutput);
            } 
            // Handle complex data
            else if constexpr (std::is_same_v<T, std::vector<std::complex<double>>>) {
                // Separate the real and imaginary parts of the complex input
                std::vector<double> realPart, imagPart;
                for (const auto& c : inputVec) {
                    realPart.push_back(c.real());
                    imagPart.push_back(c.imag());
                }

                // Containers for the delta-sigma outputs of real and imaginary parts
                std::vector<double> realOutput, imagOutput;

                // Process the real and imaginary parts separately using deltaSigma
                deltaSigma(realPart, realOutput);
                deltaSigma(imagPart, imagOutput);

                // Combine the processed real and imaginary parts into a complex output
                std::vector<std::complex<double>> complexOutput;
                for (size_t i = 0; i < realOutput.size(); ++i) {
                    complexOutput.emplace_back(realOutput[i], imagOutput[i]);
                }

                // Store the complex result in the output variant
                output = std::move(complexOutput);
            }
        },
        input // Pass the input variant to std::visit
    );
}

/**
 * @brief Delta-Sigma modulation algorithm.
 * 
 * @param x A vector of real-valued input samples.
 * @param y A vector to store the quantized output values as integers.
 * 
 * @details
 * This function implements a Delta-Sigma modulation algorithm to quantize input samples.
 * It uses multiple stages (H0, H1, H2) with specified coefficients for processing and 
 * feedback. The quantized output values are stored as integers in the `y` vector.
 * 
 * @note
 * The function uses fixed-point arithmetic (`ac_fixed`) for precise computations, and
 * assumes that the coefficients are tuned for the specific modulation requirements.
 */
static void deltaSigma(const std::vector<double>& x, std::vector<double>& y)
{
    // Ensure output vector is empty before starting
    y.clear();

    // H0 Coefficients: First stage coefficients for filtering and feedback
    ac_fixed<12, 4, true, AC_RND, AC_SAT> b00 = 7.3765809;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a01 = 0.3466036;

    // H1 Coefficients: Second stage coefficients for filtering and feedback
    ac_fixed<12, 1, true, AC_RND, AC_SAT> b10 = 0.424071040;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a11 = 0.66591402;
    ac_fixed<12, 3, true, AC_RND, AC_SAT> b11 = 2.782608716;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a12 = 0.16260264;

    // H2 Coefficients: Third stage coefficients for filtering and feedback
    ac_fixed<12, 4, true, AC_RND, AC_SAT> b20 = 4.606822182;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a21 = 0.62380242;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> b21 = 0.023331537;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a22 = 0.4509869;

    // Define a fixed-point type for intermediate calculations
    typedef ac_fixed<24, 5, true, AC_RND, AC_SAT> fx_ss;

    // Initialize variables for intermediate and feedback computations
    fx_ss y_iir = 0, e = 0, y_i = 0;
    fx_ss x0 = 0, x0d = 0; // Stage 0 variables
    fx_ss x1 = 0, w1 = 0, w1d = 0, w1dd = 0; // Stage 1 variables
    fx_ss x2 = 0, w2 = 0, w2d = 0, w2dd = 0; // Stage 2 variables
    ac_fixed<4, 4, true, AC_RND, AC_SAT> v = 0; // Quantized output
    ac_fixed<12, 4, true, AC_RND, AC_SAT> xin = 0; // Input sample (converted to fixed-point)

    // Process each input sample
    for (size_t i = 0; i < x.size(); i++)
    {
        // Convert the current input sample to fixed-point
        xin = x[i];

        // Add feedback to input to calculate intermediate value
        y_i = xin + y_iir;

        // Quantize the intermediate value
        v = y_i; // Quantization to nearest integer
        y.push_back(static_cast<double>(v.to_int())); // Store quantized value in output vector

        // Compute the quantization error
        e = y_i - v;

        // Stage 0 processing (H0)
        x0 = b00 * e + a01 * x0d;

        // Stage 1 processing (H1)
        w1 = e + a11 * w1d - a12 * w1dd;
        x1 = b10 * w1 - b11 * w1d;

        // Stage 2 processing (H2)
        w2 = e + a21 * w2d - a22 * w2dd;
        x2 = b21 * w2d - b20 * w2;

        // Combine results from all stages for feedback
        y_iir = x0 + x1 + x2;

        // Update feedback variables for the next iteration
        x0d = x0;       // Stage 0 feedback
        w1dd = w1d;     // Stage 1 delayed feedback
        w1d = w1;       // Stage 1 feedback
        w2dd = w2d;     // Stage 2 delayed feedback
        w2d = w2;       // Stage 2 feedback
    }
}

// /**
//  * @brief Wrapper function for parallel-to-serial conversion of real or complex data.
//  * 
//  * @param input A variant containing either:
//  *              - `std::vector<double>`: Real data to be processed.
//  *              - `std::vector<std::complex<double>>`: Complex data to be processed.
//  * @param LUT A lookup table for parallel-to-serial conversion.
//  * @param output A variant that stores the converted output. It will contain:
//  *               - `std::vector<double>`: Converted real data.
//  *               - `std::vector<std::complex<double>>`: Converted complex data.
//  * 
//  * @details
//  * This function handles both real and complex data by separating the real and imaginary parts
//  * for complex data and applying the parallel-to-serial conversion individually. The results
//  * are combined into the appropriate output format.
//  */
// void parallelToSerialConverterWrapper(const std::variant<std::vector<double>, std::vector<std::complex<double>>>& input,
//                                       const std::vector<std::vector<int>>& LUT,
//                                       std::variant<std::vector<double>, std::vector<std::complex<double>>>& output)
// {
//     // Use std::visit to handle the variant input
//     std::visit(
//         [&output, &LUT](const auto& inputVec) {
//             // Deduce the type of inputVec (either std::vector<double> or std::vector<std::complex<double>>)
//             using T = std::decay_t<decltype(inputVec)>;

//             // Handle real data
//             if constexpr (std::is_same_v<T, std::vector<double>>) {
//                 // Create a container for the delta-sigma output
//                 std::vector<int> realOutput;

//                 // Process the real data using deltaSigma
//                 parallelToSerialConverter(const_cast<std::vector<double>&>(inputVec), LUT, realOutput);

//                 // Store the real result in the output variant
//                 output = std::move(static_cast<std::vector<double>>(realOutput)));
//             } 
//             // Handle complex data
//             else if constexpr (std::is_same_v<T, std::vector<std::complex<double>>>) {
//                 // Separate the real and imaginary parts of the complex input
//                 std::vector<double> realPart, imagPart;
//                 for (const auto& c : inputVec) {
//                     realPart.push_back(c.real());
//                     imagPart.push_back(c.imag());
//                 }

//                 // Containers for the delta-sigma outputs of real and imaginary parts
//                 std::vector<int> realOutput, imagOutput;

//                 // Process the real and imaginary parts separately using deltaSigma
//                 parallelToSerialConverter(realPart, LUT, realOutput);
//                 parallelToSerialConverter(imagPart, LUT, imagOutput);

//                 // Combine the processed real and imaginary parts into a complex output
//                 std::vector<std::complex<double>> complexOutput;
//                 for (size_t i = 0; i < realOutput.size(); ++i) {
//                     complexOutput.emplace_back(
//                         static_cast<double>(realOutput[i]), // Real part
//                         static_cast<double>(imagOutput[i])  // Imaginary part
//                     );
//                 }

//                 // Store the complex result in the output variant
//                 output = std::move(complexOutput);
//             }
//         },
//         input // Pass the input variant to std::visit
//     );
// }

// /**
//  * @brief Converts a parallel signal into a serial signal using a lookup table (LUT).
//  * 
//  * @param inputSignal A vector of integer input values representing the parallel signal.
//  * @param LUT A lookup table represented as a 2D vector of integers, where each row defines
//  *            the mapping for a specific input value.
//  * @param outputSignal A vector to store the resulting serialized signal.
//  * 
//  * @details
//  * This function processes the input signal by mapping each value to a row in the LUT.
//  * The mapped row's values are appended to the output signal, effectively serializing
//  * the input data. The LUT is validated to ensure all rows have the same size.
//  * 
//  * @throws std::invalid_argument If the rows in the LUT are not of equal size.
//  * @throws std::out_of_range If an input signal value is outside the valid range of the LUT.
//  */
// static void parallelToSerialConverter(const std::vector<int>& inputSignal,
//                                const std::vector<std::vector<int>>& LUT,
//                                std::vector<int>& outputSignal)
// {
//     // Ensure the output vector is empty before appending new data
//     outputSignal.clear();

//     // Validate the LUT: Check that all rows have the same number of columns
//     size_t columnSize = LUT[0].size();
//     for (const auto& row : LUT) {
//         if (row.size() != columnSize) {
//             throw std::invalid_argument("All rows in the LUT must have the same size!");
//         }
//     }

//     // Correction factor for input signal values (shifts the input to match LUT indexing)
//     int correction = 1 << (BITS_NUM - 1);

//     // Process each value in the input signal
//     for (const auto& value : inputSignal) {
//         // Compute the row index in the LUT by applying the correction
//         int pos = value + correction;

//         // Ensure the computed index is within the bounds of the LUT
//         if (pos < 0 || static_cast<size_t>(pos) >= LUT.size()) {
//             throw std::out_of_range("Input value out of LUT range!");
//         }

//         // Retrieve the corresponding row from the LUT
//         const auto& lutRow = LUT[LUT.size() - 1 - pos];

//         // Append the LUT row's values to the output signal
//         outputSignal.insert(outputSignal.end(), lutRow.begin(), lutRow.end());
//     }
// }
