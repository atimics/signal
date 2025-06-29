# Building the CGame ECS Engine

This document provides comprehensive instructions for building, running, and developing the CGame Entity-Component-System engine.

## Prerequisites

### Required Tools
- **GCC** compiler with C99 support
- **Make** build system (GNU Make recommended)
- **Git** for version control (optional but recommended)

### Required Libraries
- **SDL2** development library
- **Math library** (libm) - usually included with system

### Supported Platforms
- **macOS** (primary development platform)
- **Linux** (Ubuntu, Debian, Fedora tested)
- **Windows** (via WSL or MinGW - community supported)

## Quick Start

### 1. Clone and Setup
```bash
git clone <repository-url>
cd cgame
```

### 2. Install Dependencies

#### macOS (using Homebrew)
```bash
# Install SDL2
brew install sdl2

# Or use the automated installer (if available)
make install-deps
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev build-essential
```

#### Linux (Fedora/RedHat)
```bash
sudo dnf install SDL2-devel gcc make
```

#### Linux (Arch)
```bash
sudo pacman -S sdl2 gcc make
```

### 3. Build and Run
```bash
# Build the engine
make

# Run the test scene
make run

# Or run directly
./build/cgame
```

## Build System Details

### Makefile Targets

#### Primary Targets
- **`make`** or **`make all`**: Build the complete engine
- **`make run`**: Build and execute the test scene
- **`make clean`**: Remove all build artifacts
- **`make debug`**: Build with debug symbols and optimizations disabled

#### Advanced Targets
- **`make profile`**: Build and run with timing analysis
- **`make release`**: Optimized build for distribution
- **`make install`**: Install to system directories (if configured)

### Build Configuration

#### Compiler Flags
```makefile
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -I/opt/homebrew/include/SDL2 -Isrc
```
- **`-Wall -Wextra`**: Enable comprehensive warnings
- **`-std=c99`**: Enforce C99 standard compliance
- **`-O2`**: Optimization level 2 for performance
- **`-g`**: Include debug symbols
- **`-I...`**: Include paths for SDL2 and project headers

#### Linker Flags
```makefile
LIBS = -lm -L/opt/homebrew/lib -lSDL2
```
- **`-lm`**: Link math library
- **`-lSDL2`**: Link SDL2 library
- **`-L...`**: Library search paths

### Source File Organization
```
src/
├── core.c/h         → ECS foundation
├── systems.c/h      → Game systems
├── assets.c/h       → Asset management
├── render_3d.c      → 3D rendering
├── ui.c/h           → Debug interface
├── data.c/h         → Data loading
└── test.c           → Main entry point
```

## Development Builds

### Debug Build
```bash
make debug
```
- Disables optimizations (`-O0`)
- Enables debug macros (`-DDEBUG`)
- Includes extensive debug symbols
- Enables debug UI and performance counters

### Release Build
```bash
make release
```
- Maximum optimizations (`-O3`)
- Strips debug symbols
- Disables debug output
- Optimized for distribution

### Profile Build
```bash
make profile
```
- Enables timing measurements
- Runs with performance analysis
- Useful for identifying bottlenecks

## VS Code Integration

### Available Tasks
The project includes VS Code task definitions:

- **Build Game** (`Ctrl+Shift+P` → "Tasks: Run Task" → "Build Game")
- **Run Game** (builds and executes)
- **Clean Build** (removes build artifacts)

### Launch Configuration
Create `.vscode/launch.json` for debugging:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug CGame",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/cgame",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "Build Game",
            "miDebuggerPath": "/usr/bin/gdb"
        }
    ]
}
```

## Troubleshooting

### Common Build Issues

#### SDL2 Not Found
```
error: SDL.h: No such file or directory
```
**Solution**: Install SDL2 development packages
```bash
# macOS
brew install sdl2

# Ubuntu/Debian
sudo apt-get install libsdl2-dev

# Fedora
sudo dnf install SDL2-devel
```

#### Linker Errors
```
undefined reference to `SDL_Init`
```
**Solution**: Check library paths in Makefile
- Verify SDL2 installation location
- Update `LIBS` path if needed
- On macOS, check Homebrew prefix: `brew --prefix sdl2`

#### Permission Denied
```
./build/cgame: Permission denied
```
**Solution**: Make executable
```bash
chmod +x build/cgame
```

#### Missing Build Directory
```
No such file or directory: build/
```
**Solution**: Makefile should create it automatically, but you can create manually:
```bash
mkdir -p build
```

### Platform-Specific Issues

#### macOS: Homebrew Paths
If SDL2 is installed but not found, check Homebrew installation:
```bash
# Check SDL2 installation
brew list sdl2

# Get proper include path
brew --prefix sdl2

# Update Makefile CFLAGS if needed
```

#### Linux: Missing Development Packages
Ensure you install `-dev` or `-devel` packages, not just runtime libraries:
```bash
# Wrong (runtime only)
sudo apt-get install libsdl2

# Correct (includes headers)
sudo apt-get install libsdl2-dev
```

#### Windows: WSL Setup
For Windows Subsystem for Linux:
```bash
# Install WSL development tools
sudo apt-get install build-essential

# Install SDL2 with X11 support
sudo apt-get install libsdl2-dev

# May need X server for graphics (VcXsrv, Xming)
export DISPLAY=:0.0
```

## Performance Testing

### Frame Rate Monitoring
The debug build includes performance counters:
```bash
make debug
./build/cgame
# Press F1 to toggle performance overlay
```

### Profiling Tools
```bash
# Time execution
time ./build/cgame

# Memory profiling (if available)
valgrind --tool=memcheck ./build/cgame

# CPU profiling
perf record ./build/cgame
perf report
```

### Benchmark Targets
- **Entity Creation**: 1000+ entities/second
- **Physics Updates**: 60 FPS with 500+ entities
- **Collision Detection**: 20 FPS with spatial partitioning
- **Rendering**: 60 FPS with LOD optimization

## Asset Verification

### Check Asset Loading
```bash
# Verify assets directory exists
ls -la assets/

# Check mesh files
find assets/meshes -name "*.mesh"

# Verify scene data
cat data/scenes/spaceport.txt
```

### Asset Requirements
- **Mesh Files**: OBJ format in `assets/meshes/`
- **Textures**: PNG format in mesh subdirectories
- **Scene Data**: Text format in `data/scenes/`
- **Entity Templates**: Text format in `data/templates/`

## Development Workflow

### Recommended Development Cycle
1. **Edit Code**: Modify source files in `src/`
2. **Build**: `make` to compile
3. **Test**: `make run` to execute
4. **Debug**: Use VS Code debugger or GDB
5. **Profile**: `make profile` for performance analysis
6. **Clean**: `make clean` before major changes

### Code Quality Checks
```bash
# Check for warnings
make 2>&1 | grep warning

# Validate code style (if clang-format available)
clang-format -style=file -i src/*.c src/*.h

# Static analysis (if available)
cppcheck src/
```

### Git Workflow
```bash
# Exclude build artifacts
echo "build/" >> .gitignore

# Commit source only
git add src/ data/ assets/ docs/ Makefile README.md
git commit -m "Feature: Added new component type"
```

## Distribution

### Creating Release Builds
```bash
# Clean previous builds
make clean

# Build optimized version
make release

# Package for distribution
tar -czf cgame-release.tar.gz build/cgame assets/ data/ README.md
```

### Binary Dependencies
Release builds require:
- SDL2 runtime library (not development package)
- System math library (usually available)
- Asset files (`assets/` and `data/` directories)

For distribution, consider static linking or bundling SDL2 runtime.

### Linux (Fedora)

```bash
sudo dnf install SDL2-devel gcc make
```

## Building and Running

1. **Build the game**:
   ```bash
   make
   ```

2. **Run the game**:
   ```bash
   make run
   ```

3. **Clean build files**:
   ```bash
   make clean
   ```

## Troubleshooting

If you encounter build errors:

1. Make sure SDL2 is properly installed.
2. Check that your compiler supports the C99 standard.
3. Verify that the SDL2 headers are in your system's include path.

For macOS users, if the SDL2 headers are not found, you may need to add the Homebrew include path to your environment variables:
```bash
export CFLAGS="-I/opt/homebrew/include"
export LDFLAGS="-L/opt/homebrew/lib"
```
