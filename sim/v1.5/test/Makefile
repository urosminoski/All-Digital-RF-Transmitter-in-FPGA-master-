# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Wno-deprecated-copy -MMD -MP

# Include directories
LIB_DIR1 = /home/urosminoski/Desktop/FAKS/libs/ac_types/include/
INCLUDES = -Iinc -I$(LIB_DIR1)

# Directories
SRC_DIR = src
OBJ_DIR = obj
BUILD_DIR = build

# Source files and object files
SRCS = $(SRC_DIR)/delta-sigma.cpp $(SRC_DIR)/fileProcessing.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

# Final binary
TARGET = $(BUILD_DIR)/delta-sigma

# Default target
all: $(TARGET)

# Compile rule (object files in obj/)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link rule (binaries in build/)
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Run target
run: $(TARGET)
	./$<

# Clean target
clean:
	@rm -rf $(OBJ_DIR) $(BUILD_DIR)

# Dependency includes
-include $(DEPS)

.PHONY: all clean run
