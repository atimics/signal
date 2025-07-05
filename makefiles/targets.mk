# ============================================================================
# BUILD TARGETS
# ============================================================================

# Primary targets
.PHONY: all clean test help run

# Default target is the main game executable
all: $(TARGET)

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

# Run the game
.PHONY: run
run: $(TARGET)
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
	@echo "  make          - Run all tests (default)"
	@echo "  make test     - Run all tests"
	@echo "  make all      - Build the game executable"
	@echo "  make run      - Build and run the game"
	@echo "  make clean    - Remove all build artifacts"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Test targets:"
	@echo "  make test-parallel - Run tests in parallel"
	@echo "  make test-sequential - Run tests sequentially"
	@echo ""
	@echo "Build options:"
	@echo "  BUILD_TYPE=debug|release - Set build type (default: debug)"