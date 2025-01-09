SRC_DIR = src
COMMON_SRC_DIR = ../src
BIN_DIR = bin
INC_DIR1 = inc
INC_DIR2 = ../inc
LIB_DIR1 = /home/urosminoski/Desktop/FAKS/libs/ac_types/include/
LIB_DIR2 = /home/urosminoski/Desktop/FAKS/libs/json/include/

CPP = g++
CPPFLAGS = -g -Wall -Wextra -O0 -w -I$(INC_DIR1) -I$(LIB_DIR1) -I$(LIB_DIR2)

# Source files
TEST1_SRC = $(SRC_DIR)/test_readWrite.cpp
TEST2_SRC = $(SRC_DIR)/test_serialization.cpp
TEST3_SRC = $(SRC_DIR)/test_deltaSigma.cpp
TEST4_SRC = $(SRC_DIR)/test_fixedPointComposition.cpp
TEST5_SRC = $(SRC_DIR)/test_fxpClass.cpp
TEST6_SRC = $(SRC_DIR)/test_fxpClass_2.cpp
COMMON_SRCS = $(filter-out $(COMMON_SRC_DIR)/main.cpp, $(wildcard $(COMMON_SRC_DIR)/*.cpp))

# Binaries
TEST1_BIN = $(BIN_DIR)/test_readWrite
TEST2_BIN = $(BIN_DIR)/test_serialization
TEST3_BIN = $(BIN_DIR)/test_deltaSigma
TEST4_BIN = $(BIN_DIR)/test_fixedPointComposition
TEST5_BIN = $(BIN_DIR)/test_fxpClass
TEST6_BIN = $(BIN_DIR)/test_fxpClass_2

.PHONY: all clean test1 test2 test3 test4 test5 test6 run_test1 run_test2 run_test3 run_test4 run_test5 run_test6

# Default target: build all
all: test1 test2 test3 test4 test5 test6

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

# Rule to build test4
$(TEST4_BIN): $(TEST4_SRC) | $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^

test4: $(TEST4_BIN)

# Rule to build test5
$(TEST5_BIN): $(TEST5_SRC) | $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^

TEST5_BIN: $(TEST5_BIN)

# Rule to build test6
$(TEST6_BIN): $(TEST6_SRC) | $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^

TEST6_BIN: $(TEST6_BIN)

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

# Run test4
run_test4: $(TEST4_BIN)
	@echo "Running test4..."
	@$(TEST4_BIN)

# Run test5
run_test5: $(TEST5_BIN)
	@echo "Running test5..."
	@$(TEST5_BIN)

# Run test6
run_test6: $(TEST6_BIN)
	@echo "Running test6..."
	@$(TEST6_BIN)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $@

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR)