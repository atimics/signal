# Component-Based Game Engine Makefile
CC = clang
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2 -g -Isrc
LIBS = -lm
OS := $(shell uname)

# Platform-specific flags
ifeq ($(OS),Darwin)
    # macOS
    CFLAGS += -DSOKOL_METAL
    LIBS += -framework Metal -framework MetalKit -framework AppKit -framework QuartzCore
else
    # Linux - define POSIX for clock_gettime and suppress problematic warnings
    CFLAGS += -DSOKOL_GLCORE -D_POSIX_C_SOURCE=199309L
    CFLAGS += -Wno-error=implicit-function-declaration
    CFLAGS += -Wno-error=missing-field-initializers
    CFLAGS += -Wno-error=unused-but-set-variable
    CFLAGS += -Wno-error=null-pointer-subtraction
    CFLAGS += -Wno-error=implicit-int
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

# Special compilation rules for main.c (platform-specific)
ifeq ($(OS),Darwin)
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-error=unused-but-set-variable -Wno-error=null-pointer-subtraction -x objective-c -c $< -o $@
$(BUILD_DIR)/render_3d.o: $(SRC_DIR)/render_3d.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -x objective-c -c $< -o $@
else
# Linux - additional warning suppressions for third-party headers
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-error=implicit-function-declaration -Wno-error=implicit-int -c $< -o $@
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

# WebAssembly build (requires Emscripten)
wasm: | $(BUILD_DIR)
	@echo "🌐 Building for WebAssembly..."
	@echo "📋 Note: This requires Emscripten SDK to be installed and activated"
	@echo "📋 Install with: https://emscripten.org/docs/getting_started/downloads.html"
	@echo "📋 Skipping asset compilation for WASM build"
	emcc -std=c99 -O2 -Isrc \
		-DSOKOL_GLES3 \
		-DSOKOL_IMPL \
		-DNUKLEAR_IMPLEMENTATION \
		-DEMSCRIPTEN \
		-Wno-unused-function \
		-Wno-unused-variable \
		-s USE_WEBGL2=1 -s FULL_ES3=1 \
		-s WASM=1 -s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_FUNCTIONS='["_main"]' \
		--shell-file src/shell.html \
		src/core.c src/systems.c src/render_3d.c src/render_camera.c \
		src/render_lighting.c src/render_mesh.c src/ui.c src/data.c src/main.c \
		-o $(BUILD_DIR)/cgame.html

.PHONY: all with-assets clean clean-assets assets assets-force run profile debug release wasm