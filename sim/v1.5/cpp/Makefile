BIN_FILE = bin
SRC_FILE = main

BIN_DIR = bin
SRC_DIR = src
INC_DIR = inc
LIB_DIR = /home/urosminoski/Desktop/FAKS/libs/ac_types/include/
OBJ_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

CPP = g++
CPPFLAGS = -g -Wall -Wextra -O0 -I$(INC_DIR) -I$(LIB_DIR) -Wno-deprecated-copy

.PHONY: all clean run

all: $(BIN_DIR)/$(BIN_FILE)

$(BIN_DIR)/$(BIN_FILE): $(OBJS) | $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -o $@ $^ -g

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp Makefile | $(OBJ_DIR)
	$(CPP) $(CPPFLAGS) -c $< -o $@ -g

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run: $(BIN_DIR)/$(BIN_FILE)
	./$(BIN_DIR)/$(BIN_FILE)

clean:
	-rm -r $(OBJ_DIR) $(BIN_DIR)