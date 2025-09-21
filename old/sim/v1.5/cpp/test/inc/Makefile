# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Wno-deprecated-copy

# Library include directories
LIB_DIR1 = /home/urosminoski/Desktop/FAKS/libs/ac_types/include/
LIB_DIR2 = /home/urosminoski/Desktop/FAKS/libs/json/include/

# Add include directories
INCLUDES = -Iinc -I$(LIB_DIR1) -I$(LIB_DIR2)

# Directories
SRC_DIR = src
BUILD_DIR = build
INC_DIR = inc

# Source files
MAIN_SRC = $(SRC_DIR)/main.cpp
DELTA_SIGMA_SRC = $(SRC_DIR)/delta-sigma.cpp
COMMON_SRC = $(SRC_DIR)/fileProcessing.cpp
COMMON_TPP = $(SRC_DIR)/fxpDsp.tpp  # This is a template file, so it's included during compilation.

# Output files
MAIN_OUTPUT = $(BUILD_DIR)/fxpdsp_test
DELTA_SIGMA_OUTPUT = $(BUILD_DIR)/delta-sigma

# Default target compiles main.cpp
all: $(MAIN_OUTPUT)

# Compile main.cpp
$(MAIN_OUTPUT): $(MAIN_SRC) $(COMMON_SRC) $(COMMON_TPP)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MAIN_SRC) $(COMMON_SRC) -o $(MAIN_OUTPUT)

# Compile delta-sigma.cpp (ONLY COMPILATION)
compile-deltasigma: $(DELTA_SIGMA_OUTPUT)

# Compile delta-sigma.cpp
$(DELTA_SIGMA_OUTPUT): $(DELTA_SIGMA_SRC) $(COMMON_SRC) $(COMMON_TPP)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DELTA_SIGMA_SRC) $(COMMON_SRC) -o $(DELTA_SIGMA_OUTPUT)

# Run the main program
run: $(MAIN_OUTPUT)
	./$(MAIN_OUTPUT)

# Run delta-sigma executable
deltasigma: $(DELTA_SIGMA_OUTPUT)
	./$(DELTA_SIGMA_OUTPUT)

# Clean build directory
clean:
	@rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean run deltasigma compile-deltasigma