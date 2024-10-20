#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

// Function to display the symmetric sequence
void displaySequence(const std::vector<int>& sequence, std::ostream &out) {
    for (int bit : sequence) {
        out << (bit == 1 ? "1" : "0");
    }
    out << std::endl;
}

// Function to generate the first half of the sequence based on the number of ones
std::vector<int> generateFirstHalf(int numOnes) {
    std::vector<int> half(16, 0);  // 16-bit half sequence
    for (int i = 0; i < numOnes; ++i) {
        half[i] = 1;
    }
    return half;
}

// Function to mirror the first half to generate the full 32-bit sequence
std::vector<int> mirrorSequence(const std::vector<int>& firstHalf) {
    std::vector<int> fullSequence = firstHalf;
    for (int i = firstHalf.size() - 1; i >= 0; --i) {
        fullSequence.push_back(firstHalf[i]);
    }
    return fullSequence;
}

// Iterative function to generate all LUT permutations
void generateLUTPermutations(std::ofstream& outfile) {
    std::vector<std::vector<int>> lut(9);  // LUT for levels -8 to +8

    // Fixed sequences for -8 and 8
    lut[0] = std::vector<int>(32, 0);  // All zeros for level -8
    lut[8] = std::vector<int>(32, 1);  // All ones for level +8

    // Generate all permutations for levels -7 to 7
    for (int level = -7; level <= 7; ++level) {
        int numOnes = level + 8;  // Calculate the number of ones for the first half
        std::vector<int> firstHalf = generateFirstHalf(numOnes);

        // Generate permutations of the first half
        do {
            // Create the symmetric sequence by mirroring the first half
            std::vector<int> fullSequence = mirrorSequence(firstHalf);
            lut[level + 8] = fullSequence;  // Update the LUT for this level

            // Write the LUT to the file
            for (int i = 0; i <= 8; ++i) {
                outfile << "Level: " << i - 8 << std::endl;
                displaySequence(lut[i], outfile);
            }
            outfile << std::endl;

        } while (std::next_permutation(firstHalf.begin(), firstHalf.end()));
    }
}

int main() {
    std::ofstream outfile("lut_permutations.txt");
    if (!outfile.is_open()) {
        std::cerr << "Failed to open the file!" << std::endl;
        return 1;
    }

    std::cout << "Starting LUT generation..." << std::endl;
    
    // Generate all LUT permutations and save to the file
    generateLUTPermutations(outfile);

    outfile.close();
    std::cout << "LUT permutations saved to lut_permutations.txt" << std::endl;

    return 0;
}
