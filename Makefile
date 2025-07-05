# CGame Engine - Modular Makefile System
# ============================================================================
# Component-Based Game Engine Build System
# Supports: macOS, Linux, WebAssembly
# Architecture: Entity-Component-System (ECS) with scheduled systems

# ============================================================================
# CONFIGURATION
# ============================================================================

# Build configuration (can be overridden)
BUILD_TYPE ?= debug

# ============================================================================
# MODULAR INCLUDES
# ============================================================================

# Include modular makefiles in dependency order
include makefiles/platform.mk
include makefiles/compiler.mk
include makefiles/sources.mk
include makefiles/assets.mk
include makefiles/tests.mk
include makefiles/targets.mk

# ============================================================================
# BUILD INFORMATION
# ============================================================================

$(info )
$(info CGame Engine - Focused Test Build System)
$(info =========================================)
$(info Platform: $(PLATFORM))
$(info Build Type: $(BUILD_TYPE))
$(info Compiler: $(CC))
$(info Primary Target: test ($(words $(ALL_TEST_TARGETS)) tests))
$(info )
$(info Use 'make' or 'make test' to run tests)
$(info Use 'make help' for available targets)
$(info )
