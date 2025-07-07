# CGame Tests - CMake Wrapper Makefile
# ============================================================================
# This Makefile provides familiar test commands that delegate to CMake
# Run tests from the tests/ directory with familiar make commands

# ============================================================================
# CMAKE CONFIGURATION
# ============================================================================

CMAKE_BUILD_DIR = ../build
CTEST_DIR = $(CMAKE_BUILD_DIR)

# ============================================================================
# PRIMARY TEST TARGETS
# ============================================================================

.PHONY: all test clean help

# Default target - run all tests
all: test

# Run all tests
test: configure
	@echo "🧪 Running CGame test suite..."
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target build_all_tests --parallel
	@cd $(CTEST_DIR) && ctest --output-on-failure --progress

# Configure build (ensure CMake is set up)
.PHONY: configure
configure:
	@cd .. && if [ ! -d $(CMAKE_BUILD_DIR) ] || [ ! -f $(CMAKE_BUILD_DIR)/CMakeCache.txt ]; then \
		echo "🔧 Configuring CMake build..."; \
		cmake -B $(CMAKE_BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug; \
	fi

# Clean test artifacts
clean:
	@echo "🧹 Cleaning test build artifacts..."
	@cd .. && rm -rf $(CMAKE_BUILD_DIR)

# ============================================================================
# INDIVIDUAL TEST CATEGORIES
# ============================================================================

.PHONY: test-core test-rendering test-ui test-math test-components test-world

# Core tests
test-core: configure
	@echo "📐 Running Core tests..."
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target build_all_tests
	@cd $(CTEST_DIR) && ctest --output-on-failure -R "test_core"

test-math: configure
	@echo "🔢 Running Math tests..."
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_core_math
	@cd $(CTEST_DIR) && ctest --output-on-failure -R "test_core_math"

test-components: configure
	@echo "🧩 Running Components tests..."
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_core_components
	@cd $(CTEST_DIR) && ctest --output-on-failure -R "test_core_components"

test-world: configure
	@echo "🌍 Running World tests..."
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_core_world
	@cd $(CTEST_DIR) && ctest --output-on-failure -R "test_core_world"

# Rendering tests
test-rendering: configure
	@echo "🎨 Running Rendering tests..."
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target build_all_tests
	@cd $(CTEST_DIR) && ctest --output-on-failure -R "test_render"

# UI tests
test-ui: configure
	@echo "🖥️  Running UI tests..."
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target build_all_tests  
	@cd $(CTEST_DIR) && ctest --output-on-failure -R "test_ui"

# ============================================================================
# INDIVIDUAL TEST EXECUTABLES
# ============================================================================

.PHONY: test_core_math test_core_components test_core_world test_rendering test_render_layers test_ui_rendering_pipeline

test_core_math:
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_core_math
	@cd $(CTEST_DIR) && ./test_core_math

test_core_components:
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_core_components
	@cd $(CTEST_DIR) && ./test_core_components

test_core_world:
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_core_world
	@cd $(CTEST_DIR) && ./test_core_world

test_rendering:
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_rendering
	@cd $(CTEST_DIR) && ./test_rendering

test_render_layers:
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_render_layers
	@cd $(CTEST_DIR) && ./test_render_layers

test_ui_rendering_pipeline:
	@cd .. && cmake --build $(CMAKE_BUILD_DIR) --target test_ui_rendering_pipeline
	@cd $(CTEST_DIR) && ./test_ui_rendering_pipeline

# ============================================================================
# HELP
# ============================================================================

help:
	@echo "🧪 CGame Test Suite - Make Commands"
	@echo "==================================="
	@echo ""
	@echo "🎯 PRIMARY TARGETS:"
	@echo "  make test        - Run all tests (default)"
	@echo "  make clean       - Clean test artifacts"
	@echo ""
	@echo "📂 TEST CATEGORIES:"
	@echo "  make test-core   - Run all core tests"
	@echo "  make test-math   - Run math tests"
	@echo "  make test-components - Run component tests"
	@echo "  make test-world  - Run world tests"
	@echo "  make test-rendering - Run rendering tests"
	@echo "  make test-ui     - Run UI tests"
	@echo ""
	@echo "🔧 INDIVIDUAL TESTS:"
	@echo "  make test_core_math - Run test_core_math executable"
	@echo "  make test_core_components - Run test_core_components executable"
	@echo "  make test_core_world - Run test_core_world executable"
	@echo "  make test_rendering - Run test_rendering executable"
	@echo "  make test_render_layers - Run test_render_layers executable"
	@echo "  make test_ui_rendering_pipeline - Run test_ui_rendering_pipeline executable"
	@echo ""
	@echo "✨ All commands use CMake backend for modern, reliable builds"

# Set default goal
.DEFAULT_GOAL := test