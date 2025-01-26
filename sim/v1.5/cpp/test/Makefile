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

# Automatically find all .cpp and .hpp files
CPP_FILES = $(wildcard $(SRC_DIR)/*.cpp)
HPP_FILES = $(wildcard $(INC_DIR)/*.hpp)

# Output file
OUTPUT = $(BUILD_DIR)/fxpdsp_test

# Default target
all: $(OUTPUT)

# Build executable
$(OUTPUT): $(CPP_FILES) $(HPP_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CPP_FILES) -o $(OUTPUT)

# Run the program
run: $(OUTPUT)
	./$(OUTPUT)

# Clean build directory
clean:
	@rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean run