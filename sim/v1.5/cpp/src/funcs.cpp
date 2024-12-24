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

// Helper function to read metadata and rewind the file pointer to the first data line
// Parameters:
// - inputFile: an open input file stream
// - metadata: a map to store key-value pairs from the metadata
// This function processes all lines starting with '#' as metadata. It stops
// reading when a non-metadata line is encountered and rewinds the file pointer
// so that the main function can process the data lines.
static void readMetadata(std::ifstream& inputFile, std::map<std::string, double>& metadata) 
{
    metadata.clear(); // Ensure the metadata map is empty before processing
    std::string line;

    while (std::getline(inputFile, line)) 
    {
        // Check if the line is metadata (starts with '#')
        if (line[0] != '#') 
        {
            // If the line is not metadata, rewind the file pointer
            // Move back by the size of the current line + 1 (for the newline character)
            inputFile.seekg(-static_cast<int>(line.size()) - 1, std::ios::cur);
            break; // Exit the loop as we've reached the data section
        }

        // Process metadata lines in the format "#key=value"
        size_t delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) 
        {
            // Extract the key (skipping the '#') and the value
            std::string key = line.substr(1, delimiter_pos - 1); // Skip '#'
            std::string value_str = line.substr(delimiter_pos + 1);
            try {
                // Convert the value to double and store it in the metadata map
                metadata[key] = std::stod(value_str);
            } catch (const std::invalid_argument&) {
                // Handle cases where the value cannot be converted to a number
                std::cerr << "Invalid value in metadata: " << value_str << std::endl;
            }
        } else {
            // Handle cases where the metadata line is malformed
            std::cerr << "Malformed metadata line: " << line << std::endl;
        }
    }
}


// Helper function to detect the format of a data line (real or complex)
// Parameters:
// - line: A string containing a single line of data to be analyzed.
// - isComplex: A reference to a boolean variable that will be set to true if the
//              line is determined to represent complex data (two values) or false
//              if the line represents real data (one value).
// Returns:
// - true if the line format is valid (either real or complex).
// - false if the line format is invalid (not one or two values).
// This function checks the number of values in the line. If the line contains
// exactly one value, it is treated as real data. If it contains exactly two
// values, it is treated as complex data. Any other number of values is considered
// invalid.
static bool detectFormat(const std::string& line, bool& isComplex) 
{
    std::istringstream iss(line);   // Create a string stream to parse the line
    std::vector<double> values;     // Vector to store parsed numerical values
    double value;

    // Extract all numerical values from the line
    while (iss >> value) {
        values.push_back(value); // Add each value to the vector
    }

    // Check the number of extracted values to determine the format
    if (values.size() == 2) {
        // Two values indicate complex data (real and imaginary part)
        isComplex = true;
    } else if (values.size() == 1) {
        // One value indicates real data
        isComplex = false;
    } else {
        // Any other number of values is invalid
        std::cerr << "Error: Invalid data format in line: " << line << std::endl;
        return false; // Return false to indicate a format error
    }

    return true; // Line format is valid
}


// Helper function to parse a line of data and store it in the appropriate container
// Parameters:
// - line: A string containing a single line of data to parse.
// - isComplex: A boolean indicating whether the line should be treated as complex data (true)
//              or real data (false).
// - realData: A vector to store parsed real data (if isComplex is false).
// - complexData: A vector to store parsed complex data (if isComplex is true).
// Returns:
// - true if the line is successfully parsed and stored in the correct container.
// - false if the line's format does not match the expected format (real or complex).
// This function extracts numerical values from the line, determines if the data is valid
// based on the isComplex flag, and stores it in the appropriate vector. If the line format
// is inconsistent, it logs an error and returns false.
static bool parseDataLine(const std::string& line, bool isComplex,
                   std::vector<double>& realData,
                   std::vector<std::complex<double>>& complexData) 
{
    std::istringstream iss(line);  // Create a string stream to parse the line
    std::vector<double> values;   // Vector to store parsed numerical values
    double value;

    // Extract all numerical values from the line
    while (iss >> value) {
        values.push_back(value);  // Add each value to the vector
    }

    // Handle complex data
    if (isComplex) {
        if (values.size() == 2) {
            // Two values are valid for complex data (real and imaginary parts)
            complexData.emplace_back(values[0], values[1]); // Add as a complex number
            return true; // Parsing succeeded
        }
    } 
    // Handle real data
    else {
        if (values.size() == 1) {
            // One value is valid for real data
            realData.push_back(values[0]); // Add to real data
            return true; // Parsing succeeded
        }
    }

    // If the format does not match the expectation, log an error
    std::cerr << "Error: Inconsistent data format in line: " << line << std::endl;
    return false; // Parsing failed
}


// Function to read data and metadata from a file
// Parameters:
// - fileName: The name of the input file to read.
// - data: A variant that stores either real data (std::vector<double>) or
//         complex data (std::vector<std::complex<double>>), depending on the file's content.
// - metadata: A map to store metadata key-value pairs extracted from the file.
// Returns:
// - true if the file is successfully read and parsed.
// - false if there are errors, such as invalid file format or issues with opening the file.
// This function reads a file containing metadata (lines starting with '#') and
// data (real or complex). It determines the format of the data based on the first
// non-metadata line and ensures consistency for the rest of the file.
bool readFromFile(const std::string& fileName,
                  std::variant<std::vector<double>, std::vector<std::complex<double>>>& data,
                  std::map<std::string, double>& metadata) 
{
    // Step 1: Open the input file
    std::ifstream inputFile(fileName); // Create an input file stream
    if (!inputFile.is_open()) {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return false; // Return false if the file cannot be opened
    }

    metadata.clear(); // Clear any previous metadata
    bool isComplex = false; // Flag to indicate whether the data is complex
    bool firstDataLine = true; // Flag to identify the first data line
    std::vector<double> realData; // Container for real data
    std::vector<std::complex<double>> complexData; // Container for complex data

    std::string line;

    // Step 2: Read metadata
    // Metadata lines start with '#' and are stored in the metadata map
    readMetadata(inputFile, metadata);

    // Step 3: Read and parse data
    while (std::getline(inputFile, line)) {
        if (firstDataLine) {
            // Detect the data format (real or complex) based on the first data line
            if (!detectFormat(line, isComplex)) {
                return false; // Return false if the format cannot be determined
            }
            firstDataLine = false; // Only detect format for the first data line
        }

        // Parse the current line based on the detected format
        if (!parseDataLine(line, isComplex, realData, complexData)) {
            return false; // Return false if the line format is inconsistent
        }
    }

    // Step 4: Assign the parsed data to the variant
    // Use std::move to transfer ownership of the data container to the variant
    if (isComplex) {
        data = std::move(complexData);
    } else {
        data = std::move(realData);
    }

    // Step 5: Close the input file
    inputFile.close();
    return true; // Return true if the file is successfully read and parsed
}

// Function to write data and metadata to a file
// Parameters:
// - fileName: The name of the output file to write.
// - data: A variant containing either real data (std::vector<double>) or
//         complex data (std::vector<std::complex<double>>).
// - metadata: A map containing metadata key-value pairs to be written.
// Returns:
// - true if the file is successfully written.
// - false if there are errors, such as issues with opening the file.
bool writeToFile(const std::string& fileName,
                 const std::variant<std::vector<double>, std::vector<std::complex<double>>>& data,
                 const std::map<std::string, double>& metadata) 
{
    // Step 1: Open the output file
    std::ofstream outputFile(fileName);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Cannot open file " << fileName << " for writing" << std::endl;
        return false;
    }

    // Step 2: Write metadata
    for (const auto& [key, value] : metadata) {
        outputFile << "#" << key << "=" << value << "\n";
    }
    
    // Step 3: Write data
    // Using std::visit with lambda function
    std::visit(
        [&outputFile](const auto& dataVec){
            for(const auto& value : dataVec){
                if constexpr (std::is_same_v<decltype(value), double>){
                    // Handle Real data
                    outputFile << value << '\n';
                } else if constexpr (std::is_same_v<decltype(value), std::complex<double>>) {
                    // Handle Complex data
                    outputFile << value.real() << " " << value.imag() << '\n';
                }
            }
        }, data);

    // // Step 3: Write data
    // if (std::holds_alternative<std::vector<double>>(data)) {
    //     // Handle real data
    //     const auto& realData = std::get<std::vector<double>>(data);
    //     for (const auto& value : realData) {
    //         outputFile << value << "\n";
    //     }
    // } else if (std::holds_alternative<std::vector<std::complex<double>>>(data)) {
    //     // Handle complex data
    //     const auto& complexData = std::get<std::vector<std::complex<double>>>(data);
    //     for (const auto& value : complexData) {
    //         outputFile << value.real() << " " << value.imag() << "\n";
    //     }
    // } else {
    //     std::cerr << "Error: Unknown data type in variant" << std::endl;
    //     return false;
    // }

    // Step 4: Close the output file
    outputFile.close();
    return true; // Return true if the file is successfully written
}


// Function to read LUT from a JSON file
bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT)
{
    LUT.clear();  // Make sure that vector is celared

    std::ifstream file(fileName);

    // Check if file can be opened
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + fileName);
        return false;
    }

    json j;
    file >> j;
    file.close();

    // Parse JSON into a 2D vector
    LUT = j.get<std::vector<std::vector<int>>>();
    return true;
}


// Function to read data from a file into a vector
bool readFromFileComplex(const std::string& fileName,
                  std::vector<std::complex<double>>& data,
                  std::map<std::string, double>& metadata)
{
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return false;
    }

    data.clear();       // Clear the vector to ensure no residual data
    metadata.clear();   // Clear metadata map
    std::string line;

    // Updated regex to match complex numbers with scientific notation
    std::regex complexRegex(R"(\(([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)([-+]\d*\.?\d+(?:[eE][-+]?\d+)?)j\))");

    // Read metadata
    while (std::getline(inputFile, line))
    {
        if (line[0] == '#') // Check if metadata line
        {
            std::string key, value_str;
            size_t delimiter_pos = line.find('=');
            if (delimiter_pos != std::string::npos)
            {
                key = line.substr(1, delimiter_pos - 1); // Extract key (skip '#')
                value_str = line.substr(delimiter_pos + 1); // Extract value
                try
                {
                    metadata[key] = std::stod(value_str); // Convert value to double
                }
                catch (const std::invalid_argument&)
                {
                    std::cerr << "Invalid value in metadata: " << value_str << std::endl;
                }
            }
            else
            {
                std::cerr << "Malformed metadata line: " << line << std::endl;
            }
        }
        else
        {
            // Match complex numbers or real numbers
            std::smatch match;
            if (std::regex_match(line, match, complexRegex))
            {
                double realPart = std::stod(match[1].str());
                double imagPart = std::stod(match[2].str());
                data.emplace_back(realPart, imagPart);
            }
            else
            {
                try
                {
                    // Handle real numbers
                    data.emplace_back(std::stod(line), 0.0);
                }
                catch (const std::invalid_argument&)
                {
                    std::cerr << "Error: Invalid data format in file: " << line << std::endl;
                    return false;
                }
            }
        }
    }

    inputFile.close();
    return true;
}

// Function to write data from a vector to a file
// Function to write data from a vector to a file
bool writeToFileComplex(const std::string& fileName,
                 const std::vector<std::complex<double>>& data,
                 const std::map<std::string, double>& metadata)
{
    if (data.empty())
    {
        std::cerr << "Error: Cannot write to file " << fileName << " because the vector is empty." << std::endl;
        return false;
    }

    std::ofstream outputFile(fileName);
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return false;
    }

    // Write metadata
    for (const auto& [key, value] : metadata)
    {
        outputFile << "# " << key << "=" << value << '\n';
    }
    outputFile << "# Signal data below\n";

    // Write data
    for (const auto& value : data)
    {
        // Check if the imaginary part is 0.0
        if (value.imag() == 0.0)
        {
            outputFile << value.real() << '\n'; // Write only the real part
        }
        else
        {
            outputFile << "(" << value.real() << (value.imag() >= 0 ? "+" : "")
                       << value.imag() << "j)" << '\n'; // Write complex number
        }
    }

    outputFile.close();
    return true;
}

// Wrapper function for complex numbers
void deltaSigmaComplex(const std::vector<std::complex<double>>& input, 
                       std::vector<std::complex<int>>& output)
{
    // Separate real and imaginary parts
    std::vector<double> realPart;
    std::vector<double> imagPart;
    std::vector<int> realOutput;
    std::vector<int> imagOutput;

    for (const auto& c : input) {
        realPart.push_back(c.real());
        imagPart.push_back(c.imag());
    }

    // Process the real and imaginary parts separately
    deltaSigma(realPart, realOutput);
    deltaSigma(imagPart, imagOutput);

    // Combine the real and imaginary parts into complex output
    output.clear();
    for (size_t i = 0; i < realOutput.size(); ++i) {
        output.emplace_back(realOutput[i], imagOutput[i]);
    }
}

void firComplex(std::vector<std::complex<double>>& input, std::vector<double>& firCoeff, std::vector<std::complex<double>>& output)
{
    // Separate real and imaginary parts
    std::vector<double> realPart;
    std::vector<double> imagPart;
    std::vector<double> realOutput;
    std::vector<double> imagOutput;

    for (const auto& c : input) {
        realPart.push_back(c.real());
        imagPart.push_back(c.imag());
    }

    // Process the real and imaginary parts separately
    fir(realPart, firCoeff, realOutput);
    fir(imagPart, firCoeff, imagOutput);

    // Combine the real and imaginary parts into complex output
    output.clear();
    for (size_t i = 0; i < realOutput.size(); ++i) {
        output.emplace_back(realOutput[i], imagOutput[i]);
    }
}

void fir(std::vector<double>& x, std::vector<double>& firCoeff, std::vector<double>& y)
{
    y.clear();  // Ensure output vector is empty
    
    // Discretize FIR coeffitients to fxFIR type
    std::vector<fxFIR> fxFirCoeff;
    for(size_t i = 0; i < firCoeff.size(); i++)
    {
        fxFirCoeff.push_back(firCoeff[i]);
    }

    // Perform filtering
    std::vector<fxFIR> delayLine(fxFirCoeff.size(), fxFIR(0));  // Delay line is simulationg circular buffer
    unsigned long int k = 0;      // Index of circular buffer

    for(size_t n = 0; n < x.size(); n++)
    {
        delayLine[k] = x[n];
        fxFIR sum = 0;

        for(size_t m = 0; m < delayLine.size(); m++)
        {
            // Perform convolution
            sum += fxFirCoeff[m] * delayLine[k++];

            // Simulating circular buffer
            if(k == fxFirCoeff.size())
            {
                k = 0;
            }
        }
        // Save sum to output vector
        y.push_back(sum.to_double());

        // Simulating circular buffer
        if(k-- == 0)
        {
            k = fxFirCoeff.size() - 1;
        }
    }
}



// Delta-Sigma modulation algorithm
void deltaSigma(std::vector<double>& x, std::vector<int>& y)
{
    y.clear(); // Ensure output vector is empty

    // H0 Coefficients
    ac_fixed<12, 4, true, AC_RND, AC_SAT> b00 = 7.3765809;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a01 = 0.3466036;

    // H1 Coefficients
    ac_fixed<12, 1, true, AC_RND, AC_SAT> b10 = 0.424071040;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a11 = 0.66591402;
    ac_fixed<12, 3, true, AC_RND, AC_SAT> b11 = 2.782608716;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a12 = 0.16260264;

    // H2 Coefficients
    ac_fixed<12, 4, true, AC_RND, AC_SAT> b20 = 4.606822182;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a21 = 0.62380242;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> b21 = 0.023331537;
    ac_fixed<12, 1, true, AC_RND, AC_SAT> a22 = 0.4509869;

    typedef ac_fixed<24, 5, true, AC_RND, AC_SAT> fx_ss;

    fx_ss y_iir = 0, e = 0, y_i = 0;
    fx_ss x0 = 0, x0d = 0;
    fx_ss x1 = 0, w1 = 0, w1d = 0, w1dd = 0;
    fx_ss x2 = 0, w2 = 0, w2d = 0, w2dd = 0;
    ac_fixed<4, 4, true, AC_RND, AC_SAT> v = 0;
    ac_fixed<12, 4, true, AC_RND, AC_SAT> xin = 0;

    for (size_t i = 0; i < x.size(); i++)
    {
        xin = x[i];
        y_i = xin + y_iir;

        v = y_i;
        y.push_back(v.to_int());

        e = y_i - v;

        x0 = b00 * e + a01 * x0d;

        w1 = e + a11 * w1d - a12 * w1dd;
        x1 = b10 * w1 - b11 * w1d;

        w2 = e + a21 * w2d - a22 * w2dd;
        x2 = b21 * w2d - b20 * w2;

        y_iir = x0 + x1 + x2;

        x0d = x0;
        w1dd = w1d;
        w1d = w1;
        w2dd = w2d;
        w2d = w2;
    }
}

void parallelToSerialConverterComplex(const std::vector<std::complex<int>>& inputSignal,
                                      const std::vector<std::vector<int>>& LUT,
                                      std::vector<std::complex<int>>& outputSignal)
{
    // Separate real and imaginary parts
    std::vector<int> realPart, imagPart;
    for (const auto& complexValue : inputSignal)
    {
        realPart.push_back(complexValue.real());
        imagPart.push_back(complexValue.imag());
    }

    // Process the real part
    std::vector<int> realOutput;
    parallelToSerialConverter(realPart, LUT, realOutput);

    // Process the imaginary part
    std::vector<int> imagOutput;
    parallelToSerialConverter(imagPart, LUT, imagOutput);

    // Combine the results into complex output
    size_t minSize = std::min(realOutput.size(), imagOutput.size()); // Ensure no out-of-bounds issues
    outputSignal.clear();
    for (size_t i = 0; i < minSize; ++i)
    {
        outputSignal.emplace_back(realOutput[i], imagOutput[i]);
    }
}

// Parallel-to-Serial Converter (placeholder)
void parallelToSerialConverter(const std::vector<int>& inputSignal,
                               const std::vector<std::vector<int>>& LUT,
                               std::vector<int>& outputSignal)
{
    outputSignal.clear();   // Ensure output vector is empty

    // Check if all rows in LUT have the same size
    size_t columnSize = LUT[0].size();
    for (const auto& row : LUT)
    {
        if (row.size() != columnSize)
        {
            throw std::invalid_argument("All rows in the LUT must have the same size!");
        }
    }

    int correction = 1 << (BITS_NUM - 1);

    // Process each input signal value
    for (const auto& value : inputSignal)
    {
        // Compute LUT row index
        int pos = value + correction;

        // Validate index bounds
        if (pos < 0 || static_cast<size_t>(pos) >= LUT.size())
        {
            throw std::out_of_range("Input value out of LUT range!");
        }

        // Get corresponding row from the LUT
        const auto& lutRow = LUT[LUT.size() - 1 - pos];

        // Append the row values to the output signal
        outputSignal.insert(outputSignal.end(), lutRow.begin(), lutRow.end());
    }


}

