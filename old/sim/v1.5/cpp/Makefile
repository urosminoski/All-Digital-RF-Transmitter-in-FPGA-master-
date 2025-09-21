BIN_FILE = bin
SRC_FILE = main

BIN_DIR = bin
SRC_DIR = src
INC_DIR = inc
LIB_DIR1 = /home/urosminoski/Desktop/FAKS/libs/ac_types/include/
LIB_DIR2 = /home/urosminoski/Desktop/FAKS/libs/json/include/
OBJ_DIR = obj
TESTS_DIR = tests
TEST_SRC_DIR = $(TESTS_DIR)/src
TEST_BIN_DIR = $(TESTS_DIR)/bin
TEST_OBJ_DIR = $(TESTS_DIR)/obj
TEST_DATA_DIR = $(TESTS_DIR)/data

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
SRCS_NO_MAIN = $(filter-out $(SRC_DIR)/main.cpp, $(SRCS)) # Exclude main.cpp for tests
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_NO_MAIN = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS_NO_MAIN))
TEST_SRCS = $(wildcard $(TEST_SRC_DIR)/*.cpp)
TEST_OBJS = $(patsubst $(TEST_SRC_DIR)/%.cpp, $(TEST_OBJ_DIR)/%.o, $(TEST_SRCS))

CPP = g++
CPPFLAGS = -g -Wall -Wextra -O0 -I$(INC_DIR) -I$(LIB_DIR1) -I$(LIB_DIR2) -Wno-deprecated-copy

.PHONY: all clean run tests run-tests run-test

# Default target
all: $(BIN_DIR)/$(BIN_FILE)

# Main binary build
$(BIN_DIR)/$(BIN_FILE): $(OBJS) | $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^ -g

# Object file build rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp Makefile | $(OBJ_DIR)
	$(CPP) $(CPPFLAGS) -c $< -o $@ -g

# Directories
$(BIN_DIR) $(OBJ_DIR) $(TEST_BIN_DIR) $(TEST_OBJ_DIR):
	mkdir -p $@

# Run the main program
run: $(BIN_DIR)/$(BIN_FILE)
	./$(BIN_DIR)/$(BIN_FILE)

# Build all tests
tests: $(patsubst $(TEST_SRC_DIR)/%.cpp, $(TEST_BIN_DIR)/%, $(TEST_SRCS))

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp Makefile | $(TEST_OBJ_DIR)
	$(CPP) $(CPPFLAGS) -c $< -o $@ -g

$(TEST_BIN_DIR)/%: $(TEST_OBJ_DIR)/%.o $(OBJS_NO_MAIN) | $(TEST_BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^ -g

# Run all tests
run-tests: tests
	@for test_bin in $(wildcard $(TEST_BIN_DIR)/*); do \
		echo "Running $$test_bin..."; \
		$$test_bin || exit 1; \
	done
	@echo "All tests passed!"

# Compile and run a single test file
TEST_FILE ?= serialization.cpp
run-test: $(TEST_OBJ_DIR)/$(TEST_FILE:.cpp=.o) $(OBJS_NO_MAIN) | $(TEST_BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $(TEST_BIN_DIR)/$(TEST_FILE:.cpp=) $^ -g
	@echo "Running $(TEST_BIN_DIR)/$(TEST_FILE:.cpp=)..."
	@$(TEST_BIN_DIR)/$(TEST_FILE:.cpp=)

# Clean all build artifacts
clean:
	-rm -rf $(OBJ_DIR)/* $(BIN_DIR)/* $(TEST_OBJ_DIR)/* $(TEST_BIN_DIR)/*