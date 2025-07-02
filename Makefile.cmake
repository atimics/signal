# CGame Engine - CMake Transition Compatibility Makefile
# ============================================================================
# This Makefile provides backward compatibility during the CMake transition
# All existing `make` commands will continue to work by delegating to CMake
# 
# Key Features:
# - Maintains all existing make targets and behavior
# - Seamlessly delegates to CMake build system
# - Preserves advanced test functionality
# - Supports both build systems during transition period

# ============================================================================
# CMAKE CONFIGURATION
# ============================================================================

CMAKE_BUILD_DIR = build
CMAKE_BUILD_TYPE ?= Debug
CMAKE_GENERATOR ?= "Unix Makefiles"

# Platform detection for appropriate CMake configuration
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CMAKE_PLATFORM = macOS
else ifeq ($(UNAME_S),Linux)
    CMAKE_PLATFORM = Linux
else
    CMAKE_PLATFORM = Unknown
endif

# ============================================================================
# CMAKE TARGETS (Primary build system)
# ============================================================================

.PHONY: cmake-configure cmake-build cmake-clean

# Configure CMake build
cmake-configure:
	@echo "🔧 Configuring CMake build for $(CMAKE_PLATFORM)..."
	@mkdir -p $(CMAKE_BUILD_DIR)
	@cd $(CMAKE_BUILD_DIR) && cmake .. \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
		-G $(CMAKE_GENERATOR)

# Build using CMake
cmake-build: cmake-configure
	@echo "🔨 Building with CMake..."
	@cd $(CMAKE_BUILD_DIR) && cmake --build . --parallel

# Clean CMake build
cmake-clean:
	@echo "🧹 Cleaning CMake build..."
	@rm -rf $(CMAKE_BUILD_DIR)

# ============================================================================
# COMPATIBILITY TARGETS (Delegate to CMake)
# ============================================================================

# Default target - build the game
all: cmake-build
	@echo "✅ Build complete! Executable: $(CMAKE_BUILD_DIR)/cgame"

# Alternative explicit build target
build: all

# Clean target
clean: cmake-clean

# Run the game
run: cmake-build
	@echo "🚀 Starting CGame..."
	@cd $(CMAKE_BUILD_DIR) && ./cgame

# ============================================================================
# ASSET PIPELINE COMPATIBILITY
# ============================================================================

# Asset compilation (delegated to CMake)
assets: cmake-configure
	@echo "🎨 Compiling assets..."
	@cd $(CMAKE_BUILD_DIR) && cmake --build . --target assets

# Force asset recompilation
assets-force: cmake-configure
	@echo "🎨 Force recompiling assets..."
	@cd $(CMAKE_BUILD_DIR) && cmake --build . --target assets --clean-first

# Clean assets
clean-assets: cmake-configure
	@echo "🧹 Cleaning assets..."
	@cd $(CMAKE_BUILD_DIR) && cmake --build . --target clean-assets

# ============================================================================
# TEST SYSTEM COMPATIBILITY
# ============================================================================

# Main test target - runs all tests
test: cmake-build
	@echo "🧪 Running comprehensive CGame test suite..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure --progress

# All tests (comprehensive)
test-all: cmake-build
	@echo "🧪 Running ALL test suites..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure

# Smoke tests (critical path)
test-smoke: cmake-build
	@echo "💫 Running smoke tests (critical path)..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "cmake-core|cmake-integration"

# Core ECS tests
test-core: cmake-build
	@echo "📐 Running Core ECS tests..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "^cmake-core$$"

# Systems tests
test-systems: cmake-build
	@echo "🚀 Running Systems tests..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "^cmake-systems"

# Rendering tests
test-rendering: cmake-build
	@echo "🎨 Running Rendering tests..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "^cmake-rendering$$"

# UI tests
test-ui: cmake-build
	@echo "🖥️  Running UI tests..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "^cmake-ui$$"

# Integration tests
test-integration: cmake-build
	@echo "🔗 Running Integration tests..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "^cmake-integration$$"

# Performance benchmarks
test-performance-benchmarks: cmake-build
	@echo "⚡ Running Performance benchmarks..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "^cmake-performance$$"

# Regression tests
test-regression: cmake-build
	@echo "🛡️  Running Regression tests..."
	@cd $(CMAKE_BUILD_DIR) && ctest --output-on-failure -R "^cmake-regression$$"

# ============================================================================
# ADVANCED TEST FEATURES
# ============================================================================

# Test coverage analysis
test-coverage: cmake-build
	@echo "📊 Analyzing test coverage..."
	@cd $(CMAKE_BUILD_DIR) && cmake --build . --target coverage-report

# Test dashboard generation
test-dashboard: cmake-build
	@echo "🖥️  Generating test dashboard..."
	@cd $(CMAKE_BUILD_DIR) && cmake --build . --target test-dashboard

# Test discovery
test-discover: cmake-build
	@echo "🔍 Discovering tests..."
	@cd $(CMAKE_BUILD_DIR) && ctest --show-only

# ============================================================================
# LEGACY MAKEFILE INTEGRATION
# ============================================================================

# This target provides access to the legacy test system
.PHONY: test-legacy
test-legacy:
	@echo "🧪 Running tests using the legacy advanced test Makefile..."
	@$(MAKE) -f tests/TEST_MAKEFILE_ADVANCED.mk

# Conditionally include the advanced test Makefile for compatibility
# This allows existing complex test operations to continue working if needed
ifneq ($(filter test-legacy,$(MAKECMDGOALS)),)
    include tests/TEST_MAKEFILE_ADVANCED.mk
endif

# Override specific targets to use CMake where appropriate
.PHONY: test-help
test-help:
	@echo "🧪 CGame Test Suite - CMake Transition Version"
	@echo "=============================================="
	@echo ""
	@echo "🎯 PRIMARY TEST TARGETS (CMake-based):"
	@echo "  test                 - Run all tests (comprehensive)"
	@echo "  test-all             - Run all test suites"
	@echo "  test-smoke           - Run critical path smoke tests"
	@echo "  test-core            - Run core ECS tests"
	@echo "  test-systems         - Run all systems tests"
	@echo "  test-systems-physics - Run physics systems tests"
	@echo "  test-systems-control - Run control systems tests"
	@echo "  test-systems-camera  - Run camera systems tests"
	@echo "  test-systems-performance - Run performance systems tests"
	@echo "  test-rendering       - Run rendering tests"
	@echo "  test-ui              - Run UI tests"
	@echo "  test-integration     - Run integration tests"
	@echo "  test-performance-benchmarks - Run performance benchmarks"
	@echo "  test-regression      - Run regression tests"
	@echo ""
	@echo "🔍 ANALYSIS & REPORTING:"
	@echo "  test-coverage        - Analyze test coverage"
	@echo "  test-dashboard       - Generate test dashboard"
	@echo "  test-discover        - Discover available tests"
	@echo ""
	@echo "🛠️  BUILD & MAINTENANCE:"
	@echo "  all                  - Build the game (default)"
	@echo "  clean                - Clean build artifacts"
	@echo "  run                  - Build and run the game"
	@echo "  assets               - Compile game assets"
	@echo "  assets-force         - Force recompile assets"
	@echo ""
	@echo "⚙️  CMAKE SPECIFIC:"
	@echo "  cmake-configure      - Configure CMake build"
	@echo "  cmake-build          - Build with CMake"
	@echo "  cmake-clean          - Clean CMake build"
	@echo ""
	@echo "📖 ADVANCED FEATURES:"
	@echo "  The advanced test Makefile (tests/TEST_MAKEFILE_ADVANCED.mk)"
	@echo "  provides additional features and is included for compatibility."
	@echo ""

# ============================================================================
# DEVELOPMENT AND DEBUGGING TARGETS
# ============================================================================

# Debug build
debug: CMAKE_BUILD_TYPE=Debug
debug: cmake-build
	@echo "🐛 Debug build complete"

# Release build  
release: CMAKE_BUILD_TYPE=Release
release: cmake-build
	@echo "🚀 Release build complete"

# Performance build
performance: CMAKE_BUILD_TYPE=Performance
performance: cmake-build
	@echo "⚡ Performance build complete"

# Profile the application
profile: debug
	@echo "📊 Running profiler..."
	@cd $(CMAKE_BUILD_DIR) && cmake --build . --target profile

# ============================================================================
# DOCUMENTATION TARGETS
# ============================================================================

# Generate documentation
docs: cmake-configure
	@echo "📚 Generating documentation..."
	@cd $(CMAKE_BUILD_DIR) && cmake --build . --target docs

# ============================================================================
# PLATFORM-SPECIFIC TARGETS
# ============================================================================

# WebAssembly build (if Emscripten is available)
wasm:
	@echo "🌐 Building for WebAssembly..."
	@mkdir -p build-wasm
	@cd build-wasm && emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
	@cd build-wasm && cmake --build .

# ============================================================================
# UTILITY TARGETS
# ============================================================================

# Show build status
status:
	@echo "📊 CGame Build Status:"
	@echo "====================="
	@echo "Platform: $(CMAKE_PLATFORM)"
	@echo "Build Type: $(CMAKE_BUILD_TYPE)"
	@echo "Build Directory: $(CMAKE_BUILD_DIR)"
	@echo "CMake Generator: $(CMAKE_GENERATOR)"
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

# Show available targets
help: test-help

# List all Makefile targets
list-targets:
	@echo "📋 Available Makefile Targets:"
	@$(MAKE) -qp | awk -F':' '/^[a-zA-Z0-9][^$$#\/\t=]*:([^=]|$$)/ {split($$1,A,/ /);for(i in A)print A[i]}' | sort -u

# ============================================================================
# DEFAULT TARGET DECLARATION
# ============================================================================

# Ensure test-help is not the default (maintain 'all' as default)
.DEFAULT_GOAL := all

# ============================================================================
# PHONY TARGET DECLARATIONS
# ============================================================================

.PHONY: all build clean run assets assets-force clean-assets
.PHONY: test test-all test-smoke test-core test-systems test-rendering test-ui test-integration
.PHONY: test-systems-physics test-systems-control test-systems-camera test-systems-performance
.PHONY: test-performance-benchmarks test-regression test-coverage test-dashboard test-discover
.PHONY: debug release performance profile docs wasm status help list-targets
