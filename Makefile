# CGame Engine - CMake Wrapper Makefile
# ============================================================================
# This Makefile provides familiar make commands that delegate to CMake
# All existing `make` commands work seamlessly with the new CMake backend

# ============================================================================
# CMAKE CONFIGURATION
# ============================================================================

CMAKE_BUILD_DIR = build
CMAKE_BUILD_TYPE ?= Debug

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CMAKE_PLATFORM = macOS
else ifeq ($(UNAME_S),Linux)
    CMAKE_PLATFORM = Linux
else
    CMAKE_PLATFORM = Unknown
endif

# ============================================================================
# PRIMARY TARGETS
# ============================================================================

.PHONY: all build clean test assets run help

# Default target - build the game
all: configure
	@echo "🔨 Building CGame Engine..."
	@cmake --build $(CMAKE_BUILD_DIR) --target cgame --parallel
	@echo "✅ Build complete! Executable: $(CMAKE_BUILD_DIR)/cgame"

# Explicit build target (same as all)
build: all

# Run the game
run: all
	@echo "🚀 Starting CGame..."
	@$(CMAKE_BUILD_DIR)/cgame

# Test target - build and run all tests
test: configure
	@echo "🧪 Running CGame test suite..."
	@cmake --build $(CMAKE_BUILD_DIR) --target build_all_tests --parallel
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure --progress

# Control scheme tests
test-controls: configure
	@echo "🎮 Running Control Scheme tests..."
	@cmake --build $(CMAKE_BUILD_DIR) --target test_unified_control_scheme --parallel
	@$(CMAKE_BUILD_DIR)/tests/test_unified_control_scheme

# Asset compilation (if enabled)
assets: configure
	@echo "🎨 Compiling assets..."
	@if cmake --build $(CMAKE_BUILD_DIR) --target assets 2>/dev/null; then \
		echo "✅ Assets compiled successfully"; \
	else \
		echo "ℹ️  Assets disabled (requires numpy), skipping..."; \
	fi

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build directory..."
	@rm -rf $(CMAKE_BUILD_DIR)

# ============================================================================
# CMAKE CONFIGURATION
# ============================================================================

.PHONY: configure

# Configure CMake build (runs automatically when needed)
configure:
	@if [ ! -d $(CMAKE_BUILD_DIR) ] || [ ! -f $(CMAKE_BUILD_DIR)/CMakeCache.txt ]; then \
		echo "🔧 Configuring CMake build for $(CMAKE_PLATFORM)..."; \
		cmake -B $(CMAKE_BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE); \
	fi

# Force reconfigure CMake
reconfigure:
	@echo "🔧 Reconfiguring CMake build..."
	@rm -rf $(CMAKE_BUILD_DIR)
	@cmake -B $(CMAKE_BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

# ============================================================================
# ADVANCED TARGETS
# ============================================================================

.PHONY: test-core test-rendering test-ui debug release status

# Test categories
test-core: configure
	@echo "📐 Running Core tests..."
	@cmake --build $(CMAKE_BUILD_DIR) --target build_all_tests
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "test_core"

test-rendering: configure
	@echo "🎨 Running Rendering tests..."
	@cmake --build $(CMAKE_BUILD_DIR) --target build_all_tests
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "test_render"

test-ui: configure
	@echo "🖥️  Running UI tests..."
	@cmake --build $(CMAKE_BUILD_DIR) --target build_all_tests  
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "test_ui"

# Build configurations
debug: CMAKE_BUILD_TYPE=Debug
debug: reconfigure all
	@echo "🐛 Debug build complete"

release: CMAKE_BUILD_TYPE=Release  
release: reconfigure all
	@echo "🚀 Release build complete"

# Show build status
status:
	@echo "📊 CGame Build Status:"
	@echo "====================="
	@echo "Platform: $(CMAKE_PLATFORM)"
	@echo "Build Type: $(CMAKE_BUILD_TYPE)"
	@echo "Build Directory: $(CMAKE_BUILD_DIR)"
	@if [ -d $(CMAKE_BUILD_DIR) ]; then \
		echo "Build Configured: ✅"; \
		if [ -f $(CMAKE_BUILD_DIR)/cgame ]; then \
			echo "Executable Built: ✅"; \
		else \
			echo "Executable Built: ❌"; \
		fi; \
	else \
		echo "Build Configured: ❌"; \
	fi

# ============================================================================
# HELP
# ============================================================================

help:
	@echo "🎮 CGame Engine - Make Commands"
	@echo "==============================="
	@echo ""
	@echo "🔨 BUILD COMMANDS:"
	@echo "  make             - Build the game (default)"
	@echo "  make all         - Build the game"
	@echo "  make clean       - Clean build artifacts"
	@echo "  make run         - Build and run the game"
	@echo "  make assets      - Compile game assets"
	@echo ""
	@echo "🧪 TEST COMMANDS:"
	@echo "  make test        - Run all tests"
	@echo "  make test-core   - Run core component tests"
	@echo "  make test-rendering - Run rendering tests"
	@echo "  make test-ui     - Run UI tests"
	@echo "  make test-controls - Run control scheme tests"
	@echo ""
	@echo "⚙️  BUILD TYPES:"
	@echo "  make debug       - Debug build"
	@echo "  make release     - Release build"
	@echo ""
	@echo "🔧 UTILITIES:"
	@echo "  make status      - Show build status"
	@echo "  make reconfigure - Force CMake reconfiguration"
	@echo "  make help        - Show this help"
	@echo ""
	@echo "✨ All commands use CMake backend for modern, reliable builds"

# Set default goal
.DEFAULT_GOAL := all