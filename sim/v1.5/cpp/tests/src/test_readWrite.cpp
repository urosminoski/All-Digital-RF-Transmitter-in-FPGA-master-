#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>  // For std::remove

// Function to compare two files line by line
bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream in1(file1); // Open the first file for reading
    std::ifstream in2(file2); // Open the second file for reading

    // Check if both files were successfully opened
    if (!in1.is_open() || !in2.is_open()) {
        std::cerr << "Error: Could not open files for comparison: "
                  << file1 << " or " << file2 << std::endl;
        return false; // Return false if either file could not be opened
    }

    std::string line1, line2;

    // Compare files line by line
    while (std::getline(in1, line1) && std::getline(in2, line2)) {
        if (line1 != line2) {
            return false; // Files differ if any pair of lines is not equal
        }
    }
    // Read one more line from the second file to ensure it also reaches EOF.
    // Without this step, if both files are the same size, the first file would
    // reach EOF first, exiting the loop before the second file gets a chance
    // to advance to its EOF.
    std::getline(in2, line2);

    // Check if both files have reached EOF
    return in1.eof() && in2.eof();
}


// Main test function
int main() {
    // File paths
    const std::string dataComplex = "tests/data/dataComplex.txt";
    const std::string dataReal = "tests/data/dataReal.txt";
    const std::string dataComplexTmp = "tests/data/dataComplex_tmp.txt";
    const std::string dataRealTmp = "tests/data/dataReal_tmp.txt";

    // Step 1: Read dataComplex.txt and write to dataComplex_tmp.txt
    {
        std::ifstream inFile(dataComplex);
        std::ofstream outFile(dataComplexTmp);

        if (!inFile.is_open() || !outFile.is_open()) {
            std::cerr << "Error: Could not open dataComplex files." << std::endl;
            return 1;
        }

        std::string line;
        while (std::getline(inFile, line)) {
            outFile << line << "\n";
        }
    }

    // Step 2: Read dataReal.txt and write to dataReal_tmp.txt
    {
        std::ifstream inFile(dataReal);
        std::ofstream outFile(dataRealTmp);

        if (!inFile.is_open() || !outFile.is_open()) {
            std::cerr << "Error: Could not open dataReal files." << std::endl;
            return 1;
        }

        std::string line;
        while (std::getline(inFile, line)) {
            outFile << line << "\n";
        }
    }

    // Step 3: Compare dataComplex.txt with dataComplex_tmp.txt
    bool complexTestResult = compareFiles(dataComplex, dataComplexTmp);

    // Step 4: Compare dataReal.txt with dataReal_tmp.txt
    bool realTestResult = compareFiles(dataReal, dataRealTmp);

    // std::cout << complexTestResult << " " << realTestResult << ::std::endl;

    // Step 5: Delete temporary files
    std::remove(dataComplexTmp.c_str());
    std::remove(dataRealTmp.c_str());

    // Step 6: Report results
    if (complexTestResult && realTestResult) {
        std::cout << "Test passed: All files match!" << std::endl;
    } else {
        std::cout << "Test failed: File comparison did not match." << std::endl;
    }

    return (complexTestResult && realTestResult) ? 0 : 1;
}
