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
bool readFromFile(const std::string& fileName, std::vector<double>& data)
{
    std::ifstream inputFile(fileName);

    if (!inputFile.is_open())
    {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return false;
    }

    double value;
    data.clear();

    while (inputFile >> value)
    {
        data.push_back(value);
    }

    inputFile.close();

    if (data.empty())
    {
        std::cerr << "Error: No data read from file " << fileName << std::endl;
        return false;
    }

    return true;
}

// Function to write data from a vector to a file
bool writeToFile(const std::string& fileName, const std::vector<int>& data)
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

    for (const auto& value : data)
    {
        outputFile << value << '\n';
    }

    outputFile.close();
    return true;
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

