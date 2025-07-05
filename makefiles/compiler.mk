# Compiler Configuration
# ============================================================================
# Compiler settings, warning flags, and build configurations

# Compiler selection
CC = clang

# Platform-specific warning suppressions (must be defined before CFLAGS)
PLATFORM_WARNING_FLAGS = 
ifeq ($(PLATFORM),macOS)
    PLATFORM_WARNING_FLAGS += -Wno-error=unused-but-set-variable -Wno-error=null-pointer-subtraction
else ifeq ($(PLATFORM),Linux)  
    PLATFORM_WARNING_FLAGS += -Wno-error=implicit-function-declaration -Wno-error=implicit-int
    PLATFORM_WARNING_FLAGS += -Wno-error=unused-but-set-variable -Wno-error=missing-field-initializers
endif

# Base compiler flags
BASE_CFLAGS = -Wall -Wextra -Werror -std=c99 -g
CFLAGS = $(BASE_CFLAGS) -Isrc $(YAML_INCLUDE) $(PLATFORM_GRAPHICS) $(PLATFORM_WARNING_FLAGS)

# Build type configurations
ifndef BUILD_TYPE
    BUILD_TYPE := debug
endif

ifeq ($(BUILD_TYPE),debug)
    CFLAGS += -DDEBUG -O0
else ifeq ($(BUILD_TYPE),release)
    CFLAGS += -DNDEBUG -O2
else ifeq ($(BUILD_TYPE),performance)
    CFLAGS += -DNDEBUG -O3
else
    $(error Invalid BUILD_TYPE: $(BUILD_TYPE). Use: debug, release, performance)
endif

# Automatic dependency generation
DEPFLAGS = -MMD -MP -MF $(@:.o=.d)

# Combined compilation command
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

# Math library (required for all platforms)
LIBS = -lm $(PLATFORM_ALL_LIBS)

# Test-specific flags (consolidated)
TEST_CFLAGS = $(CFLAGS) -Itests -Itests/vendor -Itests/stubs
TEST_CFLAGS += -DUNITY_TESTING -DTEST_MODE -DSOKOL_DUMMY_BACKEND
TEST_CFLAGS += -Wno-error=unused-function -Wno-error=unused-variable

# Platform-specific compilation rules
define PLATFORM_COMPILE_RULES
ifeq ($(PLATFORM),macOS)
# macOS-specific compilation with Objective-C for graphics files
$(BUILD_DIR)/graphics_api.o: $(SRC_DIR)/graphics_api.c | $(BUILD_DIR)
	@mkdir -p $$(dir $$@)
	$$(CC) $$(CFLAGS) $(OBJC_FLAGS) -c $$< -o $$@

$(BUILD_DIR)/render_3d.o: $(SRC_DIR)/render_3d.c | $(BUILD_DIR)
	@mkdir -p $$(dir $$@)
	$$(CC) $$(CFLAGS) $(OBJC_FLAGS) -c $$< -o $$@

$(BUILD_DIR)/ui.o: $(SRC_DIR)/ui.c | $(BUILD_DIR)
	@mkdir -p $$(dir $$@)
	$$(CC) $$(CFLAGS) $(OBJC_FLAGS) -c $$< -o $$@
endif
endef
