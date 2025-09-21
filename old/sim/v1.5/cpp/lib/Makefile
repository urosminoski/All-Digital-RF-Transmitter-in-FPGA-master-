# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

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

# Output file
MAIN_EXEC = $(BUILD_DIR)/fxpdsp_test

# Default target
all: $(MAIN_EXEC)

# Build main executable
$(MAIN_EXEC): $(MAIN_SRC) $(INC_DIR)/*.hpp $(SRC_DIR)/*.tpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

# Clean build directory
clean:
	@rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean