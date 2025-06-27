# Building the Project

This document provides detailed instructions for building and running the CGame project.

## Prerequisites

- GCC compiler with C99 support
- SDL2 development library
- Make build system
- Math library (libm)

## Installation

### macOS (using Homebrew)

1. Install dependencies via the Makefile:
   ```bash
   make install-deps
   ```

2. Or, install manually:
   ```bash
   brew install sdl2
   ```

### Linux (Ubuntu/Debian)

```bash
sudo apt-get install libsdl2-dev build-essential
```

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
