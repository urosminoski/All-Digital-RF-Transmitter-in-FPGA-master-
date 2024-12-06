#include "file_io.hpp"

bool readFromFile(const std::string& fileName, std::vector<double>& data)
{
    std::ifstream inputFile(fileName);

    // Check if the file can be opened
    if (!inputFile.is_open())
    {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return false;
    } 

    double value;
    data.clear(); // Ensure the vector is empty before reading

    // Read values from the file into the vector
    while (inputFile >> value)
    {
        data.push_back(value);
    }

    inputFile.close(); // Close the file

    if (data.empty()) {
        std::cerr << "Error: No data read from file " << fileName << std::endl;
        return false;
    }

    return true;
}

bool writeToFile(const std::string& fileName, const std::vector<double>& data)
{
    // Check if vector is empty
    if (data.empty())
    {
        std::cerr << "Error: Cannot write to file " << fileName << " because the vector is empty." << std::endl;
        return false;
    }

    std::ofstream outputFile(fileName);

    // Check if the file can be opened
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Cannot open file " << fileName << std::endl;
        return false;
    } 

    // Write each value to the file
    for (const auto& value : data)
    {
        outputFile << value << '\n';
    }

    outputFile.close(); // Close the file

    return true;
}
