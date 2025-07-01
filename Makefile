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
    LIBS += -lGL -lX11 -lXi -lXcursor -lXrandr -lm
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
SOURCES = core.c systems.c system/physics.c system/collision.c system/ai.c system/camera.c system/lod.c system/performance.c assets.c asset_loader/asset_loader_index.c asset_loader/asset_loader_mesh.c asset_loader/asset_loader_material.c render_3d.c render_camera.c render_lighting.c render_mesh.c ui.c data.c graphics_api.c gpu_resources.c scene_state.c scene_script.c scripts/logo_scene.c main.c
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

# Run performance test
test-performance:
	@echo "⚡ Running asset performance test..."
	$(PYTHON) $(TOOLS_DIR)/test_asset_performance.py

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
wasm: assets-wasm | $(BUILD_DIR)
	@echo "🌐 Building for WebAssembly..."
	@echo "📋 Note: This requires Emscripten SDK to be installed and activated"
	@echo "📋 Install with: https://emscripten.org/docs/getting_started/downloads.html"
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
		-s FORCE_FILESYSTEM=1 \
		--preload-file $(BUILD_ASSETS_DIR)@/assets \
		--shell-file src/shell.html \
		src/core.c src/systems.c src/render_3d.c src/render_camera.c \
		src/render_lighting.c src/render_mesh.c src/ui.c src/data.c src/graphics_api.c src/gpu_resources.c src/main.c \
		-o $(BUILD_DIR)/cgame.html

# Compile assets for WASM (simplified)
assets-wasm: $(BUILD_ASSETS_DIR)
	@echo "🔨 Preparing assets for WASM build..."
	@echo "📋 Using existing compiled assets from build/assets/"

# ============================================================================
# TEST TARGETS - Sprint 15: Unity Testing Framework Integration
# ============================================================================

# Phase 1: Core Math Tests (No dependencies)
TEST_MATH_SRC = tests/test_main_simple.c tests/test_core_math.c tests/core_math.c tests/vendor/unity.c
TEST_MATH_TARGET = $(BUILD_DIR)/cgame_tests_math

# Phase 2 & 3: Full Integration Tests (With Sokol and engine dependencies)
TEST_FULL_SRC = tests/test_runner.c tests/test_core_math.c tests/test_assets.c tests/test_rendering.c tests/vendor/unity.c
ENGINE_SRC_FOR_TEST = src/assets.c src/asset_loader/asset_loader_index.c src/asset_loader/asset_loader_mesh.c src/asset_loader/asset_loader_material.c src/gpu_resources.c src/core.c
TEST_FULL_TARGET = $(BUILD_DIR)/cgame_tests_full

# Default test target - run Phase 1 (stable math tests)
test: test-math

# Phase 1: Math tests only (stable and fast)
test-math: $(TEST_MATH_TARGET)
	@echo "🧪 Running Phase 1: Core Math Tests..."
	./$(TEST_MATH_TARGET)
	@echo "✅ Phase 1 tests completed"

# Phase 2 & 3: Full integration tests (experimental)
test-full: $(TEST_FULL_TARGET)
	@echo "🧪 Running Full Test Suite (Phases 1-3)..."
	./$(TEST_FULL_TARGET)
	@echo "✅ All tests completed"

# Build Phase 1 test executable (math only)
$(TEST_MATH_TARGET): $(TEST_MATH_SRC) | $(BUILD_DIR)
	@echo "🔨 Building Phase 1 test suite (math only)..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Itests -Itests/vendor -DUNITY_TESTING -o $@ $(TEST_MATH_SRC) -lm

# Build full test executable (all phases)
$(TEST_FULL_TARGET): $(TEST_FULL_SRC) $(ENGINE_SRC_FOR_TEST) | $(BUILD_DIR)
	@echo "🔨 Building full test suite (experimental)..."
ifeq ($(OS),Darwin)
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -DUNITY_TESTING -DSOKOL_DUMMY_BACKEND -DCGAME_TESTING -Wno-error=macro-redefined -Wno-error=implicit-function-declaration -o $@ $(TEST_FULL_SRC) $(ENGINE_SRC_FOR_TEST) -lm
else
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -DUNITY_TESTING -DSOKOL_DUMMY_BACKEND -DCGAME_TESTING -Wno-error=macro-redefined -Wno-error=implicit-function-declaration -D_POSIX_C_SOURCE=199309L -o $@ $(TEST_FULL_SRC) $(ENGINE_SRC_FOR_TEST) -lm -lGL -lX11 -lXi -lXcursor -lXrandr
endif

# Sprint 10.5 Task 1: Test index.json path resolution
test_sprint_10_5_task_1: | $(BUILD_DIR)
	@echo "🧪 Building and running Sprint 10.5 Task 1 test (standalone)..."
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/test_task_1_standalone tests/sprint_10_5/test_task_1_standalone.c
	./$(BUILD_DIR)/test_task_1_standalone
	@echo "✅ Sprint 10.5 Task 1 test complete"

# Sprint 10.5 Task 1: Integration test with actual assets.c (requires Objective-C)
test_sprint_10_5_task_1_integration: | $(BUILD_DIR)
	@echo "🧪 Building and running Sprint 10.5 Task 1 integration test..."
	$(CC) $(CFLAGS) -x objective-c -o $(BUILD_DIR)/test_task_1_integration tests/sprint_10_5/test_task_1.c src/assets.c src/render_gpu.c src/graphics_api.c $(LIBS)
	./$(BUILD_DIR)/test_task_1_integration
	@echo "✅ Sprint 10.5 Task 1 integration test complete"

.PHONY: all with-assets clean clean-assets assets assets-force assets-wasm run profile debug release wasm test test-math test-full test_sprint_10_5_task_1 test_sprint_10_5_task_1_integration

# Sprint 10.5 Task 2: Test dynamic memory allocation in mesh parser
test_sprint_10_5_task_2: | $(BUILD_DIR)
	@echo "🧪 Building and running Sprint 10.5 Task 2 test (standalone)..."
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/test_task_2_standalone tests/sprint_10_5/test_task_2_standalone.c
	./$(BUILD_DIR)/test_task_2_standalone
	@echo "✅ Sprint 10.5 Task 2 test complete"

.PHONY: test_sprint_10_5_task_2

# Sprint 10.5 Task 3: Test GPU resource validation
test_sprint_10_5_task_3: | $(BUILD_DIR)
	@echo "🧪 Building and running Sprint 10.5 Task 3 test (standalone)..."
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/test_task_3_standalone tests/sprint_10_5/test_task_3_standalone.c
	./$(BUILD_DIR)/test_task_3_standalone
	@echo "✅ Sprint 10.5 Task 3 test complete"

.PHONY: test_sprint_10_5_task_3

# Sprint 19 Task 1: Test LOD system
test_lod: | $(BUILD_DIR)
	@echo "🧪 Building and running LOD system tests..."
	$(CC) $(CFLAGS) -DSOKOL_DUMMY_BACKEND -DCGAME_TESTING -o $(BUILD_DIR)/test_lod \
		tests/test_performance_lod_simple.c tests/vendor/unity.c \
		src/system/lod.c src/core.c \
		-lm
	./$(BUILD_DIR)/test_lod
	@echo "✅ LOD system tests complete"

.PHONY: test_lod