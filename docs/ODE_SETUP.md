# ODE Physics Engine Setup Guide

## Quick Start

To use ODE physics in CGGame, you need to install the ODE library first.

### macOS Installation

```bash
# Using Homebrew
brew install ode

# Find installation path
brew list ode

# The library should be in:
# /opt/homebrew/lib/libode.dylib (Apple Silicon)
# /usr/local/lib/libode.dylib (Intel)
```

### Linux Installation

```bash
# Ubuntu/Debian
sudo apt-get install libode-dev

# Fedora/RedHat
sudo dnf install ode-devel

# Arch Linux
sudo pacman -S ode
```

### Building from Source

If you prefer to build ODE from source:

```bash
# Download ODE source
git clone https://bitbucket.org/odedevs/ode.git
cd ode

# Configure with double precision
./configure --enable-double-precision --prefix=/usr/local

# Build and install
make -j4
sudo make install
```

## Project Setup

### Option 1: System Installation (Recommended)

If ODE is installed system-wide, update the Makefile:

```makefile
# For system-wide installation
ODE_INCLUDE = 
ODE_LIB = -lode
```

### Option 2: Local Installation

To include ODE in the project:

1. Create directories:
```bash
mkdir -p src/third_party/ode/include
mkdir -p src/third_party/ode/lib
```

2. Copy ODE files:
```bash
# Copy headers
cp -r /path/to/ode/include/ode src/third_party/ode/include/

# Copy library
cp /path/to/libode.* src/third_party/ode/lib/
```

## Testing ODE Integration

1. Build the project:
```bash
make clean && make
```

2. Run the ODE test scene:
```bash
./build/cgame --scene ode_test
```

3. Test controls:
- **F1**: Toggle physics stats
- **F2**: Toggle collision debug
- **F3**: Switch between ODE and custom physics
- **F5**: Reset positions
- **F6**: Apply random impulses to obstacles

## Troubleshooting

### Link Errors

If you get link errors like "undefined reference to dWorldCreate":

1. Check that ODE is installed:
```bash
# macOS
ls -la /opt/homebrew/lib/libode*

# Linux
ldconfig -p | grep ode
```

2. Update library path if needed:
```bash
# macOS
export DYLD_LIBRARY_PATH=/opt/homebrew/lib:$DYLD_LIBRARY_PATH

# Linux
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### Header Errors

If you get "ode/ode.h not found":

1. Check include paths:
```bash
# Find ODE headers
find /usr -name "ode.h" 2>/dev/null
find /opt -name "ode.h" 2>/dev/null
```

2. Update Makefile with correct path.

## Performance Tuning

ODE configuration in `ode_physics.h`:

```c
#define ODE_STEP_SIZE 0.01f      // 100Hz physics
#define ODE_MAX_ITERATIONS 20    // Solver iterations
#define ODE_ERP 0.2f             // Error reduction
#define ODE_CFM 1e-5f            // Constraint softness
```

Adjust these values based on your needs:
- Lower `ODE_STEP_SIZE` for more accuracy
- Higher `ODE_MAX_ITERATIONS` for more stable stacking
- Tune `ODE_ERP` and `ODE_CFM` for constraint behavior

## Next Steps

1. Review the [ODE Integration Plan](ODE_PHYSICS_INTEGRATION_PLAN.md)
2. Study the ODE test scene implementation
3. Migrate existing physics entities to use ODE
4. Tune parameters for your specific gameplay needs