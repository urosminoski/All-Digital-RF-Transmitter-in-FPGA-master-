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

# ----------------------------
# 👇 DELTA-SIGMA TARGET SETUP
# ----------------------------

# Source files and object files for delta-sigma
SRCS = $(SRC_DIR)/delta-sigma.cpp $(SRC_DIR)/fileProcessing.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
TARGET = $(BUILD_DIR)/delta-sigma

# -------------------------
# 👇 RF-PROCESS TARGET SETUP
# -------------------------

# Source and object files for rf-process
RF_SRCS = $(SRC_DIR)/rf_process.cpp $(SRC_DIR)/fileProcessing.cpp
RF_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(RF_SRCS))
RF_DEPS = $(RF_OBJS:.o=.d)
RF_TARGET = $(BUILD_DIR)/rf-process

# -----------------------
# 👇 Default Build Target
# -----------------------
all: $(TARGET)

# -------------------------------
# 👇 Compile Rule for Any .cpp -> .o
# -------------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# -----------------------------
# 👇 Link Rule: Delta-Sigma Main
# -----------------------------
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@

# ---------------------------
# 👇 Link Rule: RF Process Main
# ---------------------------
rf: $(RF_TARGET)

$(RF_TARGET): $(RF_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

# ---------------------
# 👇 Run Delta-Sigma Main
# ---------------------
run: $(TARGET)
	./$<

# ----------------
# 👇 Clean All
# ----------------
clean:
	@rm -rf $(OBJ_DIR) $(BUILD_DIR)

# -----------------------------
# 👇 Include Dependency Headers
# -----------------------------
-include $(DEPS)
-include $(RF_DEPS)

.PHONY: all clean run rf