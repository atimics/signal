# ============================================================================
# BUILD TARGETS
# ============================================================================

# Primary targets
.PHONY: default all clean test help run

# Default target builds everything
default: assets test all
	@echo "✅ Build complete: assets compiled, tests passed, game built"

# Build just the game executable (depends on assets)
all: assets $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	@echo "Linking $@..."
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Pattern rule for building object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(COMPILE.c) $< -o $@

# Create build directory
$(BUILD_DIR):
	@mkdir -p $@

# Include platform-specific compilation rules
$(eval $(PLATFORM_COMPILE_RULES))

# Include dependency files
-include $(DEPS)

# Test target (delegate to test system)
.PHONY: test
test: test-sequential

# Run the game (builds everything first)
.PHONY: run
run: default
	@echo "Running CGame..."
	@$(TARGET)

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Help target
.PHONY: help
help:
	@echo "CGame Build System"
	@echo "=================="
	@echo "Available targets:"
	@echo "  make          - Build everything (assets, run tests, build game)"
	@echo "  make test     - Run all tests"
	@echo "  make all      - Build the game executable (with assets)"
	@echo "  make run      - Build everything and run the game"
	@echo "  make clean    - Remove all build artifacts"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Test targets:"
	@echo "  make test-parallel - Run tests in parallel"
	@echo "  make test-sequential - Run tests sequentially"
	@echo ""
	@echo "Build options:"
	@echo "  BUILD_TYPE=debug|release - Set build type (default: debug)"