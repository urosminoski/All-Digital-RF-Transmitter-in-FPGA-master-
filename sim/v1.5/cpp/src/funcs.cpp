#include "funcs.hpp"

using json = nlohmann::json;

// Function to read LUT from a JSON file
void readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT)
{
    LUT.clear();  // Make sure that vector is celared

    std::ifstream file(fileName);

    // Check if file can be opened
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + fileName);
    }

    json j;
    file >> j;
    file.close();

    // Parse JSON into a 2D vector
    LUT = j.get<std::vector<std::vector<int>>>();
}


// Function to read data from a file into a vector
bool readFromFile(const std::string& fileName,
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
bool writeToFile(const std::string& fileName,
                 const std::vector<std::complex<int>>& data,
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

