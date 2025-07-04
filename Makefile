# Component-Based Game Engine Makefile
CC = clang
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2 -g -Isrc
LIBS = -lm
OS := $(shell uname)

# ODE Physics Configuration
ifeq ($(OS),Darwin)
    # macOS with Homebrew
    ODE_INCLUDE = -I/opt/homebrew/include
    ODE_LIB = -L/opt/homebrew/lib -lode
else
    # Linux system-wide installation
    ODE_INCLUDE = 
    ODE_LIB = -lode
endif
# Use double precision for ODE (ODE defines it internally)
CFLAGS += $(ODE_INCLUDE) -DUSE_ODE_PHYSICS
LIBS += $(ODE_LIB)

# YAML support for scene loading
ifeq ($(OS),Darwin)
    # macOS with Homebrew
    YAML_INCLUDE = -I/opt/homebrew/include
    YAML_LIB = -L/opt/homebrew/lib -lyaml
else
    # Linux system-wide installation
    YAML_INCLUDE = 
    YAML_LIB = -lyaml
endif
CFLAGS += $(YAML_INCLUDE)
LIBS += $(YAML_LIB)

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

# Source files - now includes Microui instead of Nuklear
SOURCES = core.c systems.c system/physics.c system/ode_physics.c system/collision.c system/ai.c system/camera.c system/lod.c system/performance.c system/memory.c system/material.c system/gamepad.c system/input.c input_processing.c system/thrusters.c system/thruster_points_system.c system/control.c system/scripted_flight.c component/look_target.c component/thruster_points_component.c thruster_points.c render_thrust_cones.c assets.c asset_loader/asset_loader_index.c asset_loader/asset_loader_mesh.c asset_loader/asset_loader_material.c render_3d.c render_camera.c render_lighting.c render_mesh.c microui/microui.c ui_microui.c ui_microui_adapter.c ui.c ui_api.c ui_scene.c ui_components_stubs.c ui_adaptive_controls_stubs.c data.c graphics_api.c gpu_resources.c scene_state.c scene_script.c scene_yaml_loader.c entity_yaml_loader.c scripts/logo_scene.c scripts/derelict_navigation_scene.c scripts/flight_test_scene.c scripts/ode_test_scene.c scripts/scene_selector_scene.c scripts/ship_launch_test_scene.c config.c hidapi_mac.c main.c
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BUILD_DIR)/cgame

# Default target - try to build assets, then executable
all: assets $(TARGET)

# Build with assets (may fail if asset compiler has issues)
with-assets: assets $(TARGET)

# Help target - show available commands
help:
	@echo "🎮 CGame Engine - Available Make Targets"
	@echo "========================================"
	@echo ""
	@echo "🏗️  BUILD TARGETS:"
	@echo "  all              - Build game with assets (default)"
	@echo "  with-assets      - Build game with assets (explicit)"
	@echo "  debug            - Build with debug flags (-DDEBUG -O0)"
	@echo "  release          - Build optimized release version"
	@echo "  wasm             - Build WebAssembly version"
	@echo ""
	@echo "🧪 TEST TARGETS:"
	@echo "  test             - Run comprehensive test suite (math, UI, performance, physics, thrusters, control, camera, input, flight integration)"
	@echo ""
	@echo "🎨 ASSET TARGETS:"
	@echo "  assets           - Compile assets to binary format"
	@echo "  assets-force     - Force recompile all assets"
	@echo "  generate-assets  - Generate procedural mesh assets"
	@echo "  regenerate-assets- Clean and regenerate all assets"
	@echo "  view-meshes      - Launch mesh viewer"
	@echo "  validate-thrusters - Validate thruster attachment points"
	@echo ""
	@echo "🏃 RUN TARGETS:"
	@echo "  run              - Run the game (use SCENE=name for specific scene)"
	@echo "  profile          - Run with performance timing"
	@echo ""
	@echo "🧹 CLEANUP TARGETS:"
	@echo "  clean            - Remove all build files"
	@echo "  clean-assets     - Remove compiled assets only"
	@echo "  clean-source-assets - Remove generated source assets"
	@echo ""
	@echo "📚 DOCUMENTATION:"
	@echo "  docs             - Generate API documentation with Doxygen"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "💡 Examples:"
	@echo "  make run SCENE=flight_test    - Run specific scene"
	@echo "  make generate-asset MESH=cube - Generate specific mesh"

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

# Validate thruster definitions
validate-thrusters: $(BUILD_DIR)/validate_thrusters
	@echo "🔍 Validating thruster attachment points..."
	@./$(BUILD_DIR)/validate_thrusters data/thrusters/ assets/meshes/
	@echo "✅ Thruster validation complete"

# Build thruster validation tool
$(BUILD_DIR)/validate_thrusters: tools/validate_thrusters.c | $(BUILD_DIR)
	$(CC) -Wall -Wextra -O2 -o $@ $< -lm

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
# Filter out platform-specific, ODE, YAML files for WASM (UI now works with Microui!)
WASM_SOURCES = $(filter-out hidapi_mac.c system/ode_physics.c input_processing.c scene_yaml_loader.c entity_yaml_loader.c scripts/ode_test_scene.c, $(SOURCES))
wasm: assets-wasm | $(BUILD_DIR)
	@echo "🌐 Building for WebAssembly..."
	@echo "📋 Note: This requires Emscripten SDK to be installed and activated"
	@echo "📋 Install with: https://emscripten.org/docs/getting_started/downloads.html"
	@mkdir -p $(BUILD_DIR)
	emcc $(addprefix $(SRC_DIR)/,$(WASM_SOURCES)) \
		-std=c99 -O2 -Isrc \
		-DSOKOL_GLES3 \
		-DEMSCRIPTEN \
		-DWASM_BUILD \
		-D_USE_MATH_DEFINES \
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

# Core test sources - Updated to match actual file structure
TEST_CORE_MATH_SRC = tests/core/test_math.c tests/vendor/unity.c
TEST_CORE_COMPONENTS_SRC = tests/core/test_components.c tests/vendor/unity.c  
TEST_CORE_WORLD_SRC = tests/core/test_world.c tests/vendor/unity.c
TEST_UI_SRC = tests/ui/test_ui_system.c tests/vendor/unity.c
TEST_RENDERING_SRC = tests/rendering/test_rendering.c tests/vendor/unity.c

# Test executables  
TEST_CORE_MATH_TARGET = $(BUILD_DIR)/test_core_math
TEST_CORE_COMPONENTS_TARGET = $(BUILD_DIR)/test_core_components
TEST_CORE_WORLD_TARGET = $(BUILD_DIR)/test_core_world
TEST_UI_TARGET = $(BUILD_DIR)/test_ui
TEST_RENDERING_TARGET = $(BUILD_DIR)/test_rendering

# Engine sources for testing (minimal dependencies)
ENGINE_TEST_SRC = src/ui_api.c src/ui_scene.c src/ui_components.c src/core.c \
                  tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c

# New critical test targets
TEST_INPUT_CRITICAL_TARGET = $(BUILD_DIR)/test_input_critical
TEST_FLIGHT_SCENE_CRITICAL_TARGET = $(BUILD_DIR)/test_flight_scene_critical

# New critical test source files
TEST_INPUT_CRITICAL_SRC = tests/systems/test_input_system_critical.c tests/vendor/unity.c
TEST_FLIGHT_SCENE_CRITICAL_SRC = tests/integration/test_flight_test_scene_critical.c tests/vendor/unity.c

# System test targets - match actual file locations
TEST_PHYSICS_TARGET = $(BUILD_DIR)/test_physics_6dof
TEST_THRUSTERS_TARGET = $(BUILD_DIR)/test_thrusters  
TEST_CONTROL_TARGET = $(BUILD_DIR)/test_control
TEST_CAMERA_TARGET = $(BUILD_DIR)/test_camera
TEST_INPUT_TARGET = $(BUILD_DIR)/test_input
TEST_FLIGHT_INTEGRATION_TARGET = $(BUILD_DIR)/test_flight_integration

# Test source files for systems - corrected paths
TEST_PHYSICS_SRC = tests/systems/test_physics_6dof.c tests/vendor/unity.c
TEST_THRUSTERS_SRC = tests/systems/test_thrusters.c tests/vendor/unity.c
TEST_CONTROL_SRC = tests/systems/test_control.c tests/vendor/unity.c
TEST_CAMERA_SRC = tests/systems/test_camera.c tests/vendor/unity.c
TEST_INPUT_SRC = tests/systems/test_input.c tests/vendor/unity.c
TEST_FLIGHT_INTEGRATION_SRC = tests/integration/test_flight_integration.c tests/vendor/unity.c

# Critical physics tests for Sprint 21 velocity integration bug
PHYSICS_TEST_SOURCES = tests/vendor/unity.c src/core.c src/system/physics.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c
build/test_physics_critical: tests/systems/test_physics_critical.c $(PHYSICS_TEST_SOURCES)
	@echo "🔧 Building critical physics tests..."
	$(CC) $(TEST_CFLAGS) \
		-o $@ tests/systems/test_physics_critical.c $(PHYSICS_TEST_SOURCES) $(LDFLAGS)

# Main test target - runs all essential tests including new comprehensive tests
test: $(TEST_CORE_MATH_TARGET) $(TEST_CORE_COMPONENTS_TARGET) $(TEST_CORE_WORLD_TARGET) $(TEST_UI_TARGET) $(TEST_RENDERING_TARGET) $(TEST_PHYSICS_TARGET) $(TEST_THRUSTERS_TARGET) $(TEST_CONTROL_TARGET) $(TEST_CAMERA_TARGET) $(TEST_INPUT_TARGET) $(TEST_FLIGHT_INTEGRATION_TARGET) $(TEST_INPUT_CRITICAL_TARGET) $(TEST_FLIGHT_SCENE_CRITICAL_TARGET) $(TEST_INPUT_CRITICAL_TARGET) $(TEST_FLIGHT_SCENE_CRITICAL_TARGET)
	@echo "🧪 Running Comprehensive CGame Test Suite"
	@echo "=========================================="
	@echo "📐 Core Math Tests..."
	./$(TEST_CORE_MATH_TARGET)
	@echo ""
	@echo "🔧 Core Components Tests..."
	./$(TEST_CORE_COMPONENTS_TARGET)
	@echo ""
	@echo "� Core World Tests..."
	./$(TEST_CORE_WORLD_TARGET)
	@echo ""
	@echo "�🎨 UI System Tests..."
	./$(TEST_UI_TARGET)
	@echo ""
	@echo "�️ Rendering Tests..."
	./$(TEST_RENDERING_TARGET)
	@echo ""
	@echo "🚀 6DOF Physics Tests..."
	./$(TEST_PHYSICS_TARGET)
	@echo ""
	@echo "🚀 Thruster System Tests..."
	./$(TEST_THRUSTERS_TARGET)
	@echo ""
	@echo "🎮 Control Authority Tests..."
	./$(TEST_CONTROL_TARGET)
	@echo ""
	@echo "📷 Camera System Tests..."
	./$(TEST_CAMERA_TARGET)
	@echo ""
	@echo "🎮 Input System Tests..."
	./$(TEST_INPUT_TARGET)
	@echo ""
	@echo "✈️  Flight Integration Tests..."
	./$(TEST_FLIGHT_INTEGRATION_TARGET)
	@echo ""
	@echo "🧪 Critical Input System Tests..."
	./$(TEST_INPUT_CRITICAL_TARGET)
	@echo ""
	@echo "✈️ Critical Flight Scene Tests..."
	./$(TEST_FLIGHT_SCENE_CRITICAL_TARGET)
	@echo ""
	@echo "✅ All tests completed successfully!"

# Build math tests (no dependencies)
$(TEST_CORE_MATH_TARGET): $(TEST_CORE_MATH_SRC) | $(BUILD_DIR)
	@echo "🔨 Building core math tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_STANDALONE -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_CORE_MATH_SRC) src/core.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build core components tests
$(TEST_CORE_COMPONENTS_TARGET): $(TEST_CORE_COMPONENTS_SRC) | $(BUILD_DIR)
	@echo "🔨 Building core components tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND -DTEST_STANDALONE \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_CORE_COMPONENTS_SRC) src/core.c src/system/physics.c tests/support/test_utilities.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build core world tests  
$(TEST_CORE_WORLD_TARGET): $(TEST_CORE_WORLD_SRC) | $(BUILD_DIR)
	@echo "🔨 Building core world tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND -DTEST_STANDALONE \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_CORE_WORLD_SRC) src/core.c src/system/physics.c tests/support/test_utilities.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build UI tests (with test stubs)
$(TEST_UI_TARGET): $(TEST_UI_SRC) | $(BUILD_DIR)
	@echo "🔨 Building UI tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_UI_SRC) $(ENGINE_TEST_SRC) tests/stubs/nuklear_test_stubs.c -lm

# Build rendering tests
$(TEST_RENDERING_TARGET): $(TEST_RENDERING_SRC) | $(BUILD_DIR)
	@echo "🔨 Building rendering tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND -DTEST_STANDALONE \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_RENDERING_SRC) src/core.c src/render_3d.c src/render_camera.c src/render_lighting.c src/render_mesh.c src/assets.c src/gpu_resources.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build physics 6DOF tests
$(TEST_PHYSICS_TARGET): $(TEST_PHYSICS_SRC) | $(BUILD_DIR)
	@echo "🔨 Building 6DOF physics tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_PHYSICS_SRC) src/core.c src/system/physics.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build thruster system tests  
$(TEST_THRUSTERS_TARGET): $(TEST_THRUSTERS_SRC) | $(BUILD_DIR)
	@echo "🔨 Building thruster system tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_THRUSTERS_SRC) src/core.c src/system/thrusters.c src/system/physics.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build control authority tests
$(TEST_CONTROL_TARGET): $(TEST_CONTROL_SRC) | $(BUILD_DIR)
	@echo "🔨 Building control authority tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_CONTROL_SRC) src/core.c src/system/control.c src/system/thrusters.c src/system/physics.c src/system/input.c src/input_processing.c src/component/look_target.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build camera system tests
$(TEST_CAMERA_TARGET): $(TEST_CAMERA_SRC) | $(BUILD_DIR)
	@echo "🔨 Building camera system tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_CAMERA_SRC) src/core.c src/system/camera.c src/render_camera.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build input system tests
$(TEST_INPUT_TARGET): $(TEST_INPUT_SRC) | $(BUILD_DIR)
	@echo "🔨 Building input system tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_INPUT_SRC) src/core.c src/system/input.c src/input_processing.c src/component/look_target.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build flight integration tests
$(TEST_FLIGHT_INTEGRATION_TARGET): $(TEST_FLIGHT_INTEGRATION_SRC) | $(BUILD_DIR)
	@echo "🔨 Building flight integration tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_FLIGHT_INTEGRATION_SRC) src/core.c src/system/physics.c src/system/thrusters.c src/system/control.c src/system/input.c src/input_processing.c src/system/gamepad.c src/hidapi_mac.c src/component/look_target.c tests/stubs/graphics_api_test_stub.c $(LIBS) -lm

# Build critical input system tests
$(TEST_INPUT_CRITICAL_TARGET): $(TEST_INPUT_CRITICAL_SRC) | $(BUILD_DIR)
	@echo "🔨 Building critical input system tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_INPUT_CRITICAL_SRC) src/core.c src/system/input.c src/input_processing.c src/component/look_target.c tests/stubs/graphics_api_test_stub.c tests/stubs/engine_test_stubs.c -lm

# Build critical flight scene tests
$(TEST_FLIGHT_SCENE_CRITICAL_TARGET): $(TEST_FLIGHT_SCENE_CRITICAL_SRC) | $(BUILD_DIR)
	@echo "🔨 Building critical flight scene tests..."
	$(CC) -Wall -Wextra -std=c99 -O2 -g -Isrc -Itests -Itests/vendor -Itests/stubs \
		-DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND \
		-Wno-error=unused-function -Wno-error=unused-variable \
		-o $@ $(TEST_FLIGHT_SCENE_CRITICAL_SRC) src/core.c src/system/physics.c src/system/thrusters.c src/system/control.c src/system/input.c src/input_processing.c src/system/gamepad.c src/hidapi_mac.c src/component/look_target.c src/scene_state.c tests/stubs/graphics_api_test_stub.c $(LIBS) -lm

# ============================================================================
# MEMORY TESTING TARGETS
# ============================================================================

# Memory leak detection with Valgrind (Linux/macOS with Valgrind installed)
memory-test: $(TEST_TARGETS)
	@echo "🧠 Running memory leak detection with Valgrind..."
	@for test in $(TEST_TARGETS); do \
		echo "Testing $$test for memory leaks..."; \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
			--error-exitcode=1 --suppressions=.valgrind.supp ./$$test || exit 1; \
	done
	@echo "✅ All tests passed memory leak detection"

# Memory error detection with Valgrind
memory-check: $(TEST_TARGETS)
	@echo "🧠 Running memory error detection with Valgrind..."
	@for test in $(TEST_TARGETS); do \
		echo "Checking $$test for memory errors..."; \
		valgrind --tool=memcheck --error-exitcode=1 --track-origins=yes \
			--suppressions=.valgrind.supp ./$$test || exit 1; \
	done
	@echo "✅ All tests passed memory error detection"

# Address Sanitizer builds (faster than Valgrind)
asan-test: clean
	@echo "🧠 Building tests with AddressSanitizer..."
	@$(MAKE) test CFLAGS="$(CFLAGS) -fsanitize=address -fno-omit-frame-pointer"
	@echo "✅ AddressSanitizer tests completed"

# Thread Sanitizer builds
tsan-test: clean
	@echo "🧠 Building tests with ThreadSanitizer..."
	@$(MAKE) test CFLAGS="$(CFLAGS) -fsanitize=thread -fno-omit-frame-pointer"
	@echo "✅ ThreadSanitizer tests completed"

# Undefined Behavior Sanitizer builds
ubsan-test: clean
	@echo "🧠 Building tests with UndefinedBehaviorSanitizer..."
	@$(MAKE) test CFLAGS="$(CFLAGS) -fsanitize=undefined -fno-omit-frame-pointer"
	@echo "✅ UndefinedBehaviorSanitizer tests completed"

# Memory testing suite (runs all memory tests)
memory-suite: asan-test memory-test memory-check
	@echo "✅ Full memory testing suite completed"

.PHONY: all with-assets clean clean-assets assets assets-force assets-wasm run profile debug release wasm test help docs memory-test memory-check asan-test tsan-test ubsan-test memory-suite
