#!/bin/bash
# Quick WASM build and test script for CGame

echo "🌐 CGame WebAssembly Build & Test Script"
echo "========================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo -e "${RED}❌ Error: Emscripten (emcc) not found!${NC}"
    echo ""
    echo "Please install Emscripten first:"
    echo "  Option 1: brew install emscripten"
    echo "  Option 2: Follow instructions at https://emscripten.org/docs/getting_started/downloads.html"
    echo ""
    echo "If already installed, activate it with:"
    echo "  source /path/to/emsdk/emsdk_env.sh"
    exit 1
fi

# Show Emscripten version
echo -e "${GREEN}✅ Emscripten found:${NC}"
emcc --version | head -n1

# Clean previous builds
echo ""
echo "🧹 Cleaning previous builds..."
make clean

# Build WASM
echo ""
echo "🔨 Building WebAssembly version..."
make wasm

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ Build successful!${NC}"
    
    # Check if Python 3 is available
    if command -v python3 &> /dev/null; then
        echo ""
        echo "🌐 Starting local web server..."
        echo -e "${YELLOW}➡️  Open your browser to: http://localhost:8080/cgame.html${NC}"
        echo ""
        echo "Press Ctrl+C to stop the server"
        cd build
        python3 -m http.server 8080
    elif command -v python &> /dev/null; then
        echo ""
        echo "🌐 Starting local web server (Python 2)..."
        echo -e "${YELLOW}➡️  Open your browser to: http://localhost:8080/cgame.html${NC}"
        echo ""
        echo "Press Ctrl+C to stop the server"
        cd build
        python -m SimpleHTTPServer 8080
    else
        echo -e "${YELLOW}⚠️  Warning: Python not found!${NC}"
        echo ""
        echo "To test the build, you'll need to serve the files from build/ directory:"
        echo "  - cgame.html"
        echo "  - cgame.js"
        echo "  - cgame.wasm"
        echo "  - cgame.data"
        echo ""
        echo "You can use any web server, for example:"
        echo "  npx http-server build -p 8080"
    fi
else
    echo -e "${RED}❌ Build failed!${NC}"
    echo "Check the error messages above for details."
    exit 1
fi