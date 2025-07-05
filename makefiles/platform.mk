# Platform Detection and Configuration
# ============================================================================
# Detects operating system and sets platform-specific flags and libraries

OS := $(shell uname)

# Platform-specific configurations
ifeq ($(OS),Darwin)
    # macOS Configuration
    PLATFORM := macOS
    
    # Sokol Graphics Backend
    PLATFORM_GRAPHICS = -DSOKOL_METAL
    
    # System Frameworks
    PLATFORM_LIBS = -framework Metal -framework MetalKit -framework AppKit -framework QuartzCore
    PLATFORM_LIBS += -framework IOKit -framework CoreFoundation
    
    # ODE Physics (Homebrew)
    ODE_INCLUDE = -I/opt/homebrew/include
    ODE_LIB = -L/opt/homebrew/lib -lode
    
    # YAML Support (Homebrew)
    YAML_INCLUDE = -I/opt/homebrew/include
    YAML_LIB = -L/opt/homebrew/lib -lyaml
    
    # Platform-specific compilation flags
    OBJC_FLAGS = -x objective-c
    
else ifeq ($(OS),Linux)
    # Linux Configuration
    PLATFORM := Linux
    
    # Sokol Graphics Backend
    PLATFORM_GRAPHICS = -DSOKOL_GLCORE -D_POSIX_C_SOURCE=199309L
    
    # System Libraries
    PLATFORM_LIBS = -lGL -lX11 -lXi -lXcursor -lXrandr -ludev -lrt -lpthread
    
    # ODE Physics (System)
    ODE_INCLUDE = 
    ODE_LIB = -lode
    
    # YAML Support (System)
    YAML_INCLUDE = 
    YAML_LIB = -lyaml
    
else
    $(error Unsupported platform: $(OS))
endif

# Apply platform-specific includes and libraries
CFLAGS += $(ODE_INCLUDE) $(YAML_INCLUDE) -DUSE_ODE_PHYSICS
PLATFORM_ALL_LIBS = $(ODE_LIB) $(YAML_LIB) $(PLATFORM_LIBS)

# Platform information
$(info Building for platform: $(PLATFORM))
