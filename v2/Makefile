# V2 Component-Based Game Engine Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -I/opt/homebrew/include/SDL2
LIBS = -lm -L/opt/homebrew/lib -lSDL2

# Directories
SRC_DIR = .
BUILD_DIR = build

# Source files
SOURCES = core.c systems.c render_3d.c test.c
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BUILD_DIR)/test

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link executable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run the test
run: $(TARGET)
	./$(TARGET)

# Run with performance analysis
profile: $(TARGET)
	time ./$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -DNDEBUG -O3
release: clean $(TARGET)

.PHONY: all clean run profile debug release
