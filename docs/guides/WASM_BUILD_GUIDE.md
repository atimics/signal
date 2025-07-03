# WebAssembly (WASM) Build Guide

This guide helps you build and test CGame in a web browser using WebAssembly.

## Prerequisites

### 1. Install Emscripten SDK

Emscripten is required to compile C code to WebAssembly.

#### macOS Installation

```bash
# Clone the Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install latest SDK
./emsdk install latest
./emsdk activate latest

# Add to your shell (add to ~/.zshrc or ~/.bash_profile for persistence)
source ./emsdk_env.sh

# Verify installation
emcc --version
```

#### Alternative: Using Homebrew
```bash
brew install emscripten
```

### 2. Python 3 (for local web server)

Most systems have Python 3 installed. Verify with:
```bash
python3 --version
```

## Building for WebAssembly

### 1. Activate Emscripten (if installed manually)

```bash
# Navigate to emsdk directory
cd /path/to/emsdk
source ./emsdk_env.sh

# Return to cgame directory
cd /path/to/cgame
```

### 2. Build the WASM version

```bash
# Clean previous builds
make clean

# Build WASM
make wasm
```

This will create:
- `build/cgame.html` - HTML wrapper
- `build/cgame.js` - JavaScript glue code
- `build/cgame.wasm` - WebAssembly binary
- `build/cgame.data` - Asset data

### 3. Common Build Issues

#### Issue: "emcc not found"
**Solution**: Ensure Emscripten is activated:
```bash
source /path/to/emsdk/emsdk_env.sh
```

#### Issue: Asset loading errors
**Solution**: The WASM build needs assets packaged differently:
```bash
make assets-wasm
```

#### Issue: Memory errors
**Solution**: WASM has different memory constraints. The build uses `ALLOW_MEMORY_GROWTH=1` to handle this.

## Testing Locally

### 1. Start a Local Web Server

WebAssembly requires a web server (file:// protocol won't work due to CORS).

#### Option A: Python 3
```bash
cd build
python3 -m http.server 8080
```

#### Option B: Python 2
```bash
cd build
python -m SimpleHTTPServer 8080
```

#### Option C: Node.js http-server
```bash
npm install -g http-server
cd build
http-server -p 8080
```

### 2. Open in Browser

Navigate to: http://localhost:8080/cgame.html

**Recommended Browsers**:
- Chrome/Edge (best performance)
- Firefox (good support)
- Safari (may have issues)

### 3. Browser Console

Open Developer Tools (F12) to see:
- Loading progress
- Error messages
- Performance stats

## Controls in Browser

The same controls work in the browser:

### Keyboard
- **WASD** - Pitch/Roll
- **QE** - Yaw
- **RF** - Thrust forward/back
- **Shift/Ctrl** - Vertical thrust
- **Tab** - Camera mode
- **Esc** - Menu

### Gamepad
- Should work if connected before page load
- May need to press a button to activate

## Performance Tips

### 1. Browser Settings
- Disable browser extensions (they can impact performance)
- Close other tabs
- Use hardware acceleration

### 2. Build Optimizations

For release build with optimizations:
```bash
make clean
make wasm WASM_OPT="-O3"
```

### 3. Debugging

For debug build with better error messages:
```bash
make clean
make wasm WASM_OPT="-O0 -g"
```

## Deployment

To deploy on a web server:

1. Copy these files to your web server:
   - `cgame.html`
   - `cgame.js`
   - `cgame.wasm`
   - `cgame.data`

2. Ensure proper MIME types:
   - `.wasm` → `application/wasm`
   - `.data` → `application/octet-stream`

3. Enable CORS headers if loading from CDN

## Troubleshooting

### Black Screen
1. Check browser console for errors
2. Ensure WebGL 2 is supported: https://get.webgl.org/webgl2/
3. Try different browser

### Poor Performance
1. Check if hardware acceleration is enabled
2. Reduce quality settings (if available)
3. Try release build

### Audio Issues
- Browser may block audio until user interaction
- Click on the canvas to enable audio

### Gamepad Not Working
1. Connect gamepad before loading page
2. Press a gamepad button after page loads
3. Check gamepad API support: https://gamepad-tester.com/

## WASM-Specific Considerations

### Memory Management
- WASM has different memory constraints than native
- Stack size is limited
- Heap grows as needed with `ALLOW_MEMORY_GROWTH=1`

### File System
- Uses Emscripten's virtual file system
- Assets are preloaded into memory
- No real file I/O

### Threading
- Currently single-threaded
- SharedArrayBuffer requires special headers for threading

### Graphics
- Uses WebGL 2 (OpenGL ES 3.0)
- Some desktop OpenGL features may not work

## Quick Test Script

Create `test_wasm.sh`:

```bash
#!/bin/bash
# Quick WASM test script

echo "🔧 Building WASM..."
make clean
make wasm

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "🌐 Starting web server on http://localhost:8080/cgame.html"
    cd build
    python3 -m http.server 8080
else
    echo "❌ Build failed!"
fi
```

Make it executable:
```bash
chmod +x test_wasm.sh
./test_wasm.sh
```

---

## Summary

1. Install Emscripten SDK
2. Run `make wasm`
3. Start local web server in `build/`
4. Open http://localhost:8080/cgame.html
5. Enjoy CGame in your browser!

For issues, check the browser console and ensure WebGL 2 support.