BIN_FILE = bin
SRC_FILE = main
PY_FILE  = print_result.py

BIN_DIR = bin
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
PY_DIR  = python

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

CPP = g++
OPT = -O0
CPPFLAGS = -g -Wall

all: $(BIN_DIR)/$(BIN_FILE)

$(BIN_DIR)/$(BIN_FILE): $(OBJS) | $(BIN_DIR)
	$(CPP) $(CFLAGS) -o $@ $^ -g

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp Makefile | $(OBJ_DIR)
	$(CPP) $(CFLAGS) -I$(INC_DIR) -c $< -o $@ -g

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run: $(BIN_DIR)/$(BIN_FILE)
	./$(BIN_DIR)/$(BIN_FILE)
	./$(PY_DIR)/$(PY_FILE)

clean:
	rm -r $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean