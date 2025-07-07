#!/bin/bash
# SIGNAL CMake Configuration Helper
# This script helps with the transition from Make to CMake

set -e

# Default values
PRESET="debug"
BUILD_DIR="build"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            PRESET="debug"
            shift
            ;;
        --release)
            PRESET="release"
            shift
            ;;
        --test)
            PRESET="test"
            shift
            ;;
        --asan)
            PRESET="asan"
            shift
            ;;
        --clean)
            echo "Cleaning build directory..."
            rm -rf "$BUILD_DIR"
            shift
            ;;
        --help)
            echo "SIGNAL CMake Configuration Helper"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --debug     Configure for debug build (default)"
            echo "  --release   Configure for release build"
            echo "  --test      Configure for testing"
            echo "  --asan      Configure with AddressSanitizer"
            echo "  --clean     Clean build directory first"
            echo "  --help      Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                  # Configure debug build"
            echo "  $0 --release        # Configure release build"
            echo "  $0 --clean --test   # Clean and configure for testing"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed"
    echo "Please install CMake 3.20 or later"
    exit 1
fi

# Check CMake version
CMAKE_VERSION=$(cmake --version | head -1 | awk '{print $3}')
CMAKE_MAJOR=$(echo "$CMAKE_VERSION" | cut -d. -f1)
CMAKE_MINOR=$(echo "$CMAKE_VERSION" | cut -d. -f2)

if [[ $CMAKE_MAJOR -lt 3 ]] || [[ $CMAKE_MAJOR -eq 3 && $CMAKE_MINOR -lt 20 ]]; then
    echo "Error: CMake 3.20 or later is required (found $CMAKE_VERSION)"
    exit 1
fi

# Configure with preset
echo "Configuring SIGNAL with preset: $PRESET"
echo "CMake version: $CMAKE_VERSION"
echo ""

cmake --preset="$PRESET"

echo ""
echo "Configuration complete!"
echo ""
echo "To build, run one of:"
echo "  cmake --build build"
echo "  cd build && make"
echo "  make -C build"
echo ""
echo "Common targets:"
echo "  make -C build          # Build everything"
echo "  make -C build test     # Run tests"
echo "  make -C build run      # Run the game"
echo "  make -C build help     # Show all targets"