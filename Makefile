# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lSDL2 -lm

# Directories
SRC_DIR = .
BUILD_DIR = build

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BUILD_DIR)/game

# Default target
all: $(TARGET)

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link the executable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run the game
run: $(TARGET)
	./$(TARGET)

# Install SDL2 on macOS (requires Homebrew)
install-deps:
	@echo "Installing SDL2 using Homebrew..."
	@if command -v brew >/dev/null 2>&1; then \
		brew list sdl2 >/dev/null 2>&1 || brew install sdl2; \
		echo "SDL2 installed successfully!"; \
	else \
		echo "Homebrew not found. Please install Homebrew first:"; \
		echo "  /bin/bash -c \"\$$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""; \
		echo "Then run 'make install-deps' again."; \
	fi

# Phony targets
.PHONY: all clean run install-deps
