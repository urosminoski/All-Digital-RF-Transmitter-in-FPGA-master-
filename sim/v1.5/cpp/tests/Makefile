SRC_DIR = src
COMMON_SRC_DIR = ../src
BIN_DIR = bin
INC_DIR1 = inc
INC_DIR2 = ../inc
LIB_DIR1 = /home/urosminoski/Desktop/FAKS/libs/ac_types/include/
LIB_DIR2 = /home/urosminoski/Desktop/FAKS/libs/json/include/

CPP = g++
CPPFLAGS = -g -Wall -Wextra -O0 -w -I$(INC_DIR1) -I$(INC_DIR2) -I$(LIB_DIR1) -I$(LIB_DIR2)

# Source files
TEST1_SRC = $(SRC_DIR)/test_readWrite.cpp
TEST2_SRC = $(SRC_DIR)/test_serialization.cpp
TEST3_SRC = $(SRC_DIR)/test_deltaSigma.cpp
COMMON_SRCS = $(filter-out $(COMMON_SRC_DIR)/main.cpp, $(wildcard $(COMMON_SRC_DIR)/*.cpp))

# Binaries
TEST1_BIN = $(BIN_DIR)/test_readWrite
TEST2_BIN = $(BIN_DIR)/test_serialization
TEST3_BIN = $(BIN_DIR)/test_deltaSigma

.PHONY: all clean test1 test2 run_test1 run_test2

# Default target: build all
all: test1 test2 test3

# Rule to build test1
$(TEST1_BIN): $(TEST1_SRC) $(COMMON_SRCS) | $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^

test1: $(TEST1_BIN)

# Rule to build test2
$(TEST2_BIN): $(TEST2_SRC) $(COMMON_SRCS) | $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^

test2: $(TEST2_BIN)

# Rule to build test3
$(TEST3_BIN): $(TEST3_SRC) $(COMMON_SRCS) | $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^

test3: $(TEST3_BIN)

# Run test1
run_test1: $(TEST1_BIN)
	@echo "Running test1..."
	@$(TEST1_BIN)

# Run test2
run_test2: $(TEST2_BIN)
	@echo "Running test2..."
	@$(TEST2_BIN)

# Run test3
run_test3: $(TEST3_BIN)
	@echo "Running test3..."
	@$(TEST3_BIN)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $@

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR)