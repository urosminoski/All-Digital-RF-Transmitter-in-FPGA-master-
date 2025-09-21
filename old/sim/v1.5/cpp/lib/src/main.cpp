#include <iostream>
#include "fxpDsp.hpp"

bool readLUT(const std::string& fileName, std::vector<std::vector<int>>& LUT);
void readArray (const std::string& fileName, std::vector<double>& vector);
void readArray_2D (const std::string& fileName, std::vector<std::vector<double>>& matrix);

// #define IIR_FILTERS { \
//   {7.3765809    , 0             , 0 , 1 , -0.3466036    , 0             }, \
//   {0.424071040  , -2.782608716  , 0 , 1 , -0.66591402   , 0.16260264    }, \
//   {-4.606822182 , 0.023331537   , 0 , 1 , -0.62380242   , 0.4509869     }  \
// }


#define FIR_COEFF {-2.63665205e-17, 3.06687619e-01, 5.00000000e-01, 3.06687619e-01, -2.63665205e-17}

int main() {
    using MyFxpDsp = FxpDsp<12, 4, true, AC_RND, AC_SAT>;

     // Load FIR and IIR coeffitients from files
    std::string fileName_deltaSigma_iirs    = "./data/input/deltaSigma_iirs.txt"; 
    std::string fileName_interpolate_firs   = "./data/input/polyFirCofficients_60dB.txt"; 
    std::vector<std::vector<double>> deltaSigma_iirs;
    std::vector<std::vector<double>> interpolate_firs;
    readArray_2D(fileName_deltaSigma_iirs, deltaSigma_iirs);
    readArray_2D(fileName_interpolate_firs, interpolate_firs);

    // Load LUT data from file
    std::string file_path_lut = "./data/input/LUT4.json";
    std::vector<std::vector<int>> LUT;
    readLUT(file_path_lut, LUT);

    // Real Data
    std::string fileName_sinData_real       = "./data/input/sinData.txt";
    std::string fileName_interpolated_real  = "./data/output/sinData_interpolated.txt";
    std::string fileName_deltaSigma_real    = "./data/output/sinData_deltaSigma.txt";
    std::string fileName_serialized_real    = "./data/output/sinData_serial.txt";
    std::string fileName_fir_real           = "./data/output/sinData_fir.txt";
    // Complex Data
    std::string fileName_sinData_complex        = "./data/input/sinDataComplex.txt";
    std::string fileName_interpolated_complex   = "./data/output/sinDataComplex_interpolated.txt";
    std::string fileName_deltaSigma_complex     = "./data/output/sinDataComplex_deltaSigma.txt";
    std::string fileName_serialized_complex     = "./data/output/sinDataComplex_serial.txt";
    std::string fileName_fir_complex            = "./data/output/sinDataComplex_fir.txt";

    MyFxpDsp realSignal(true);
    realSignal.readFromFile(fileName_sinData_real);
    realSignal.interpolation<12, 4>(interpolate_firs);
    realSignal.writeToFile(fileName_interpolated_real);
    // realSignal.scale(8);
    realSignal.deltaSigma<12, 4, 4, 4, true>(deltaSigma_iirs);
    realSignal.writeToFile(fileName_deltaSigma_real);

    MyFxpDsp complexSignal;
    complexSignal.readFromFile(fileName_sinData_complex);
    complexSignal.interpolation<12, 4>(interpolate_firs);
    complexSignal.writeToFile(fileName_interpolated_complex);
    // complexSignal.scale(8);
    complexSignal.deltaSigma<12, 4, 4, 4, true>(deltaSigma_iirs);
    complexSignal.writeToFile(fileName_deltaSigma_complex);

    return 0;
}

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
    nlohmann::json j;

    // Parse the file content into the JSON object
    file >> j;

    // Close the file after reading
    file.close();

    // Convert the JSON object to a 2D vector of integers
    LUT = j.get<std::vector<std::vector<int>>>();

    // Return true to indicate successful parsing
    return true;
}

void readArray (const std::string& fileName, std::vector<double>& vector) {
    std::ifstream file(fileName);
    // Ensure file is opened
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        double value;
        while (ss >> value) {
            vector.push_back(value);
        }
    }

    // Close file
    file.close();
}

void readArray_2D (const std::string& fileName, std::vector<std::vector<double>>& matrix) {
    std::ifstream file(fileName);
    // Ensure file is opened
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines or comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::vector<double> row;
        double value;
        while (ss >> value) {
            row.push_back(value);
        }
        if (!row.empty()) {
            matrix.push_back(row);
        }
    }

    // Close file
    file.close();
}