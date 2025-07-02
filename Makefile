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
    # Add hidapi support for gamepad input
    LIBS += -framework IOKit -framework CoreFoundation
else
    # Linux - define POSIX for clock_gettime and suppress problematic warnings
    CFLAGS += -DSOKOL_GLCORE -D_POSIX_C_SOURCE=199309L
    CFLAGS += -Wno-error=implicit-function-declaration
    CFLAGS += -Wno-error=missing-field-initializers
    CFLAGS += -Wno-error=unused-but-set-variable
    CFLAGS += -Wno-error=null-pointer-subtraction
    CFLAGS += -Wno-error=implicit-int
    LIBS += -lGL -lX11 -lXi -lXcursor -lXrandr -lm
    # Add hidapi support for gamepad input
    LIBS += -ludev -lrt -lpthread
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
SOURCES = core.c systems.c system/physics.c system/collision.c system/ai.c system/camera.c system/lod.c system/performance.c system/memory.c system/material.c system/gamepad.c system/input.c assets.c asset_loader/asset_loader_index.c asset_loader/asset_loader_mesh.c asset_loader/asset_loader_material.c render_3d.c render_camera.c render_lighting.c render_mesh.c ui.c ui_api.c ui_scene.c ui_components.c data.c graphics_api.c gpu_resources.c scene_state.c scene_script.c scripts/logo_scene.c scripts/derelict_navigation_scene.c scripts/flight_test_scene.c scripts/scene_selector_scene.c config.c hidapi_mac.c main.c
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

# Compile assets using the new binary pipeline
assets: $(BUILD_ASSETS_DIR)

$(BUILD_ASSETS_DIR): | $(BUILD_DIR)
	@echo "🔨 Compiling assets to binary format..."
	$(PYTHON) $(TOOLS_DIR)/build_pipeline.py
	@echo "✅ Asset compilation complete."

# Force asset recompilation
assets-force:
	@echo "🔨 Force recompiling assets..."
	$(PYTHON) $(TOOLS_DIR)/build_pipeline.py --force
	@echo "✅ Asset compilation complete."

# Generate all procedural source assets using clean pipeline
generate-assets:
	@echo "🌱 Generating all mesh assets with UV layouts..."
	$(PYTHON) $(TOOLS_DIR)/clean_asset_pipeline.py --all
	@echo "✅ Source asset generation complete."

# Generate specific asset using clean pipeline  
generate-asset:
	@echo "🌱 Generating mesh asset: $(MESH)..."
	$(PYTHON) $(TOOLS_DIR)/clean_asset_pipeline.py --mesh $(MESH)
	@echo "✅ Asset $(MESH) generated."

# Full asset regeneration and compilation
regenerate-assets: clean-source-assets generate-assets assets

# Clean source assets (SVGs, PNGs, OBJs)
clean-source-assets:
	@echo "🧹 Cleaning source assets..."
	find $(ASSETS_DIR)/meshes/props -name "*.obj" -delete
	find $(ASSETS_DIR)/meshes/props -name "*.svg" -delete
	find $(ASSETS_DIR)/meshes/props -name "*.png" -delete
	find $(ASSETS_DIR)/meshes/props -name "*.mtl" -delete
	find $(ASSETS_DIR)/meshes/props -name "metadata.json" -delete
	@echo "✅ Source assets cleaned."

# Launch mesh viewer
view-meshes:
	@echo "🎨 Launching mesh viewer..."
	$(PYTHON) $(TOOLS_DIR)/launch_mesh_viewer.py

# Link executable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Special compilation rules for main.c (platform-specific)
ifeq ($(OS),Darwin)
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-error=unused-but-set-variable -Wno-error=null-pointer-subtraction -x objective-c -c $< -o $@
$(BUILD_DIR)/render_3d.o: $(SRC_DIR)/render_3d.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -x objective-c -c $< -o $@
$(BUILD_DIR)/graphics_api.o: $(SRC_DIR)/graphics_api.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -x objective-c -c $< -o $@
# UI compilation with relaxed warnings for third-party libraries
$(BUILD_DIR)/ui.o: $(SRC_DIR)/ui.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-error=unused-but-set-variable -Wno-error=null-pointer-subtraction -x objective-c -c $< -o $@
else
# Linux - additional warning suppressions for third-party headers
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-error=implicit-function-declaration -Wno-error=implicit-int -c $< -o $@
# UI compilation with relaxed warnings for third-party libraries
$(BUILD_DIR)/ui.o: $(SRC_DIR)/ui.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-error=unused-but-set-variable -Wno-error=null-pointer-subtraction -Wno-error=implicit-function-declaration -c $< -o $@
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
wasm: assets-wasm | $(BUILD_DIR)
	@echo "🌐 Building for WebAssembly..."
	@echo "📋 Note: This requires Emscripten SDK to be installed and activated"
	@echo "📋 Install with: https://emscripten.org/docs/getting_started/downloads.html"
	@mkdir -p $(BUILD_DIR)
	emcc -std=c99 -O2 -Isrc \
		-DSOKOL_GLES3 \
		-DSOKOL_IMPL \
		
		-DEMSCRIPTEN \
		-Wno-unused-function \
		-Wno-unused-variable \
		-Wno-unused-parameter \
		-s USE_WEBGL2=1 -s FULL_ES3=1 \
		-s WASM=1 -s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_FUNCTIONS='["_main"]' \
		-s FORCE_FILESYSTEM=1 \
		-s INITIAL_MEMORY=67108864 \
		--preload-file $(BUILD_ASSETS_DIR)@/assets \
		--shell-file src/shell.html \
		$(addprefix src/,$(SOURCES)) \
		-o $(BUILD_DIR)/cgame.html
	@echo "✅ WebAssembly build complete: $(BUILD_DIR)/cgame.html"
	@echo "📋 Serve with: python3 -m http.server 8000 (from build/ directory)"

# Compile assets for WASM (simplified)
assets-wasm: $(BUILD_ASSETS_DIR)
	@echo "🔨 Preparing assets for WASM build..."
	@echo "📋 Using existing compiled assets from build/assets/"

# ============================================================================
# DOCUMENTATION TARGETS
# ============================================================================

# Generate API documentation using Doxygen
docs: Doxyfile
	@echo "📚 Generating API documentation..."
	doxygen Doxyfile
	@echo "✅ API documentation generated in docs/api_docs/html"

# ============================================================================
# UNIFIED TEST SYSTEM
# ============================================================================

# Core test sources
TEST_MATH_SRC = tests/test_main_simple.c tests/test_core_math.c tests/core_math.c tests/vendor/unity.c
TEST_UI_SRC = tests/unit/test_ui_system.c tests/vendor/unity.c
TEST_SYSTEMS_SRC = tests/unit/test_systems.c tests/vendor/unity.c
TEST_PERFORMANCE_SRC = tests/performance/test_performance_critical.c tests/vendor/unity.c

# Test executables
TEST_MATH_TARGET = $(BUILD_DIR)/test_math
TEST_UI_TARGET = $(BUILD_DIR)/test_ui_system
TEST_SYSTEMS_TARGET = $(BUILD_DIR)/test_systems
TEST_PERFORMANCE_TARGET = $(BUILD_DIR)/test_performance

# Engine sources for testing (minimal dependencies)
ENGINE_TEST_SRC = src/ui_api.c src/ui_scene.c src/ui_components.c src/core.c \
                  tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c

# Main test target - runs all essential tests
test: $(TEST_MATH_TARGET) $(TEST_UI_TARGET)
	@echo "🧪 Running CGame Test Suite"
	@echo "=============================="
	@echo "📐 Core Math Tests..."
	./$(TEST_MATH_TARGET)
	@echo ""
	@echo "🎨 UI System Tests..."
	./$(TEST_UI_TARGET)
	@echo ""
	@echo "✅ All tests completed successfully!"

# Build math tests (no dependencies)
$(TEST_MATH_TARGET): $(TEST_MATH_SRC) | $(BUILD_DIR)
	@echo "🔨 Building math tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Itests -Itests/vendor -DUNITY_TESTING -o $@ $(TEST_MATH_SRC) -lm

# Build UI tests (with test stubs)
$(TEST_UI_TARGET): $(TEST_UI_SRC) | $(BUILD_DIR)
	@echo "🔨 Building UI tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_UI_SRC) $(ENGINE_TEST_SRC) -lm

# Optional: Performance testing (can be resource intensive)
test-performance: $(TEST_PERFORMANCE_TARGET)
	@echo "🚀 Running performance tests..."
	./$(TEST_PERFORMANCE_TARGET)
	@echo "✅ Performance tests completed"

$(TEST_PERFORMANCE_TARGET): $(TEST_PERFORMANCE_SRC) | $(BUILD_DIR)
	@echo "🔨 Building performance tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_PERFORMANCE_SRC) $(ENGINE_TEST_SRC) -lm

.PHONY: all with-assets clean clean-assets assets assets-force assets-wasm run profile debug release wasm test
