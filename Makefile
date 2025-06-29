# Component-Based Game Engine Makefile
CC = clang
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Isrc
LIBS = -lm
OS := $(shell uname)

# Platform-specific flags
ifeq ($(OS),Darwin)
    # macOS
    CFLAGS += -DSOKOL_METAL
    LIBS += -framework Metal -framework MetalKit -framework AppKit
else
    # Linux
    CFLAGS += -DSOKOL_GLCORE
    LIBS += -lGL -lX11 -lm
endif

# Directories
SRC_DIR = src
BUILD_DIR = build
DATA_DIR = data
ASSETS_DIR = assets
TOOLS_DIR = tools

# Asset compilation
PYTHON = ./.venv/bin/python3
ASSET_COMPILER = $(TOOLS_DIR)/asset_compiler.py
BUILD_ASSETS_DIR = $(BUILD_DIR)/assets

# Source files
SOURCES = core.c systems.c assets.c render_3d.c render_camera.c render_lighting.c render_mesh.c ui.c data.c main.c
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BUILD_DIR)/cgame

# Default target - try to build assets, then executable
all: assets $(TARGET)

# Build with assets (may fail if asset compiler has issues)
with-assets: assets $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile assets using Python asset compiler
assets: $(BUILD_ASSETS_DIR)

$(BUILD_ASSETS_DIR): $(ASSET_COMPILER) $(shell find $(ASSETS_DIR) -name "*.obj" -o -name "*.mesh") | $(BUILD_DIR)
	@echo "🔨 Compiling assets..."
	@echo "📋 Note: Asset compilation requires Python dependencies (trimesh, cairo, numpy, scipy)"
	@echo "📋 Install with: pip install trimesh cairosvg numpy scipy jsonschema"
	$(PYTHON) $(ASSET_COMPILER) --source_dir $(ASSETS_DIR)/meshes --build_dir $(BUILD_DIR)/assets/meshes
	@echo "✅ Asset compilation attempted (check output above for any errors)"

# Force asset recompilation
assets-force:
	@echo "🔨 Force recompiling assets..."
	$(PYTHON) $(ASSET_COMPILER) --source_dir $(ASSETS_DIR)/meshes --build_dir $(BUILD_DIR)/assets/meshes --overwrite
	@echo "✅ Asset compilation complete"

# Link executable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Special rule for main.c on macOS to compile as Objective-C
ifeq ($(OS),Darwin)
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -x objective-c -c $< -o $@
$(BUILD_DIR)/render_3d.o: $(SRC_DIR)/render_3d.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -x objective-c -c $< -o $@
endif


# Clean build files and compiled assets
clean:
	rm -rf $(BUILD_DIR)

# Clean only compiled assets
clean-assets:
	rm -rf $(BUILD_ASSETS_DIR)

# Run the test, allow passing a scene name, e.g., make run SCENE=spaceport_alpha
run: $(TARGET)
	./$(TARGET) $(SCENE)

# Run with performance analysis
profile: $(TARGET)
	time ./$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -DNDEBUG -O3
release: clean $(TARGET)

.PHONY: all with-assets clean clean-assets assets assets-force run profile debug release