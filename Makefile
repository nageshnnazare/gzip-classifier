# Compiler and Flags
CXX      := g++
CXXFLAGS := -Wall -Wextra -Wpedantic -std=c++17 -Iinclude
LDFLAGS  := 

# Directories
SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
BIN_DIR   := bin

# Target Executable
TARGET := $(BIN_DIR)/app

# Find all source files and map them to object files
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $@"
	@$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

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
	@$(TARGET)

.PHONY: all clean run
