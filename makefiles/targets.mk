# Build Targets and Rules
# ============================================================================
# Main build targets, rules, and special compilation cases

# ============================================================================
# MAIN BUILD TARGETS
# ============================================================================

# Default target
.DEFAULT_GOAL := all
all: test assets $(TARGET)

# Build with assets (explicit)
.PHONY: with-assets
with-assets: assets $(TARGET)

# Build without running tests (use with caution)
.PHONY: build-only
build-only: assets $(TARGET)

# ============================================================================
# COMPILATION RULES
# ============================================================================

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Generic object file compilation with dependency generation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(COMPILE.c) $< -o $@

# Include dependency files
-include $(DEPS)

# Main executable linking
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	@echo "🔗 Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $@ $(LIBS) $(PLATFORM_LIBS)
	@echo "✅ Build complete: $(TARGET)"

# ============================================================================
# PLATFORM-SPECIFIC COMPILATION RULES
# ============================================================================

# Apply platform-specific rules
$(eval $(PLATFORM_COMPILE_RULES))

# ============================================================================
# BUILD VARIANTS
# ============================================================================

# Debug build
.PHONY: debug
debug: BUILD_TYPE := debug
debug: clean $(TARGET)

# Release build
.PHONY: release
release: BUILD_TYPE := release
release: clean $(TARGET)

# Performance build
.PHONY: performance
performance: BUILD_TYPE := performance
performance: clean $(TARGET)

# ============================================================================
# WEBASSEMBLY BUILD
# ============================================================================

.PHONY: wasm
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

# ============================================================================
# EXECUTION TARGETS
# ============================================================================

# Run the game
.PHONY: run
run: $(TARGET)
	@echo "🎮 Running CGame..."
	./$(TARGET) $(SCENE)

# Run with performance timing
.PHONY: profile
profile: $(TARGET)
	@echo "📊 Running CGame with performance profiling..."
	time ./$(TARGET) $(SCENE)

# ============================================================================
# CLEANUP TARGETS
# ============================================================================

# Clean all build files
.PHONY: clean
clean:
	@echo "🧹 Cleaning build files..."
	rm -rf $(BUILD_DIR)
	@echo "✅ Clean complete."

# ============================================================================
# MEMORY TESTING TARGETS
# ============================================================================

# Memory leak detection with Valgrind (Linux/macOS with Valgrind installed)
.PHONY: memory-test
memory-test: $(ALL_TEST_TARGETS)
	@echo "🧠 Running memory leak detection with Valgrind..."
	@for test in $(ALL_TEST_TARGETS); do \
		echo "Testing $$test for memory leaks..."; \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
			--error-exitcode=1 --suppressions=.valgrind.supp ./$$test || exit 1; \
	done
	@echo "✅ All tests passed memory leak detection"

# Memory error detection with Valgrind
.PHONY: memory-check
memory-check: $(ALL_TEST_TARGETS)
	@echo "🧠 Running memory error detection with Valgrind..."
	@for test in $(ALL_TEST_TARGETS); do \
		echo "Checking $$test for memory errors..."; \
		valgrind --tool=memcheck --error-exitcode=1 --track-origins=yes \
			--suppressions=.valgrind.supp ./$$test || exit 1; \
	done
	@echo "✅ All tests passed memory error detection"

# Address Sanitizer builds (faster than Valgrind)
.PHONY: asan-test
asan-test: clean
	@echo "🧠 Building tests with AddressSanitizer..."
	@$(MAKE) test CFLAGS="$(CFLAGS) -fsanitize=address -fno-omit-frame-pointer"
	@echo "✅ AddressSanitizer tests completed"

# Thread Sanitizer builds
.PHONY: tsan-test
tsan-test: clean
	@echo "🧠 Building tests with ThreadSanitizer..."
	@$(MAKE) test CFLAGS="$(CFLAGS) -fsanitize=thread -fno-omit-frame-pointer"
	@echo "✅ ThreadSanitizer tests completed"

# Undefined Behavior Sanitizer builds
.PHONY: ubsan-test
ubsan-test: clean
	@echo "🧠 Building tests with UndefinedBehaviorSanitizer..."
	@$(MAKE) test CFLAGS="$(CFLAGS) -fsanitize=undefined -fno-omit-frame-pointer"
	@echo "✅ UndefinedBehaviorSanitizer tests completed"

# Memory testing suite (runs all memory tests)
.PHONY: memory-suite
memory-suite: asan-test memory-test memory-check
	@echo "✅ Full memory testing suite completed"

# Memory leak test for macOS using the logo scene
.PHONY: test-leaks
test-leaks: $(TARGET)
	@echo "🧠 Running memory leak detection on logo scene..."
	@echo "#!/bin/bash" > $(BUILD_DIR)/test_logo.sh
	@echo "timeout 3 ./$(TARGET) > /dev/null 2>&1 || true" >> $(BUILD_DIR)/test_logo.sh
	@chmod +x $(BUILD_DIR)/test_logo.sh
	@if command -v leaks >/dev/null 2>&1; then \
		OUTPUT=$$(leaks --atExit -- $(BUILD_DIR)/test_logo.sh 2>&1); \
		if echo "$$OUTPUT" | grep -q "0 leaks for 0 total leaked bytes"; then \
			echo "✅ No memory leaks detected"; \
		else \
			echo "$$OUTPUT" | grep -E "Process.*[0-9]+ leak|total leaked"; \
			echo "❌ Memory leaks detected!"; \
			exit 1; \
		fi \
	else \
		echo "⚠️  'leaks' command not available (macOS only)"; \
	fi

# ============================================================================
# DOCUMENTATION TARGETS
# ============================================================================

# Generate API documentation using Doxygen
.PHONY: docs
docs: Doxyfile
	@echo "📚 Generating API documentation..."
	doxygen Doxyfile
	@echo "✅ API documentation generated in docs/api_docs/html"

# ============================================================================
# UTILITY TARGETS
# ============================================================================

# Show build information
.PHONY: info
info:
	@echo "CGame Build System Information"
	@echo "=============================="
	@echo "Platform: $(PLATFORM)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Compiler: $(CC)"
	@echo "Source Directory: $(SRC_DIR)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "Target: $(TARGET)"
	@echo "Test Targets: $(words $(ALL_TEST_TARGETS)) tests"
	@echo ""

# Help target
.PHONY: help
help:
	@echo "🎮 CGame Engine - Available Make Targets"
	@echo "========================================"
	@echo ""
	@echo "🏗️  BUILD TARGETS:"
	@echo "  all              - Run tests, then build game with assets (default)"
	@echo "  build-only       - Build without running tests (use with caution)"
	@echo "  with-assets      - Build game with assets (explicit)"
	@echo "  debug            - Build with debug flags (-DDEBUG -O0)"
	@echo "  release          - Build optimized release version (-O2)"
	@echo "  performance      - Build high-performance version (-O3)"
	@echo "  wasm             - Build WebAssembly version"
	@echo ""
	@echo "🧪 TEST TARGETS:"
	@echo "  test             - Run test suite (defaults to parallel)"
	@echo "  test-sequential  - Run tests sequentially"
	@echo "  test-parallel    - Run tests in parallel"
	@echo "  test-core        - Run core engine tests only"
	@echo "  test-rendering   - Run rendering tests only"
	@echo "  test-systems     - Run system tests only"
	@echo "  test-ui          - Run UI tests only"
	@echo "  test-integration - Run integration tests only"
	@echo "  test-leaks       - Run memory leak detection (macOS)"
	@echo "  memory-test      - Run Valgrind memory leak detection"
	@echo "  memory-check     - Run Valgrind memory error detection"
	@echo "  asan-test        - Run tests with AddressSanitizer"
	@echo "  tsan-test        - Run tests with ThreadSanitizer"
	@echo "  ubsan-test       - Run tests with UndefinedBehaviorSanitizer"
	@echo "  memory-suite     - Run comprehensive memory testing"
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
	@echo "  info             - Show build configuration information"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "💡 Examples:"
	@echo "  make run SCENE=flight_test    - Run specific scene"
	@echo "  make generate-asset MESH=cube - Generate specific mesh"
	@echo "  make test-parallel            - Run tests in parallel"
	@echo "  make BUILD_TYPE=release all   - Build optimized version"

# Declare all phony targets
.PHONY: all with-assets build-only debug release performance wasm run profile clean \
        info help test test-sequential test-parallel test-core test-rendering \
        test-systems test-ui test-integration memory-suite asan-test tsan-test \
        ubsan-test memory-test memory-check test-leaks docs assets assets-force \
        generate-assets regenerate-assets clean-assets clean-source-assets \
        assets-wasm view-meshes validate-thrusters
