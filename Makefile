# gzip-classifier: gzip-normalized compression distance (NCD) text classifier.
# Builds all sources under src/ into build/bin/app.

# Compiler and Flags
CXX      := g++
CXXFLAGS := -Wall -Wextra -Wpedantic -std=c++17 -Iinclude -O3 -ggdb
LDFLAGS  := -lz

# Directories
SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
BIN_DIR   := build/bin

# Target Executable
TARGET := $(BIN_DIR)/app

# Formating
FORMAT := clang-format --style=llvm -i

# Find all source files and map them to object files
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
HEADERS := $(wildcard $(INC_DIR)/*.hpp)

# Default target
all: format $(TARGET)

format: ${SOURCES} ${HEADERS}
	@echo "Formatting $@..."
	$(FORMAT) $(HEADERS) $(SOURCES)

# Link the executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $@..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create directories if they don't exist
$(BIN_DIR) $(BUILD_DIR):
	@mkdir -p $@

# Clean target
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)

# Run the executable
run: all
	@echo "Running $(TARGET)..."
	time $(TARGET)

.PHONY: all clean run
