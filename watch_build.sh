#!/bin/bash

# CGame Auto-Build Watcher
# Watches for file changes and automatically builds the project
# Updates .build_status file with current build state

BUILD_STATUS_FILE=".build_status"
WATCH_DIRS="src/ assets/ data/ Makefile"
WATCH_EXTENSIONS="c,h,json,txt,glsl,metal,frag,vert"

echo "CGame Auto-Build Watcher Started"
echo "Watching: $WATCH_DIRS"
echo "Extensions: $WATCH_EXTENSIONS"
echo "Build status file: $BUILD_STATUS_FILE"
echo "Press Ctrl+C to stop"
echo ""

# Function to perform build and update status
build_project() {
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] File change detected, building..."
    
    # Write building status
    echo "BUILDING|$timestamp|Building project..." > "$BUILD_STATUS_FILE"
    
    # Run make and capture output
    if make clean > /dev/null 2>&1 && make > build.log 2>&1; then
        echo "SUCCESS|$timestamp|Build completed successfully" > "$BUILD_STATUS_FILE"
        echo "[$timestamp] ✅ Build SUCCESS"
    else
        echo "FAILED|$timestamp|Build failed - check build.log for details" > "$BUILD_STATUS_FILE"
        echo "[$timestamp] ❌ Build FAILED"
        echo "Error details:"
        tail -10 build.log | sed 's/^/  /'
    fi
    echo ""
}

# Initial build
build_project

# Check if fswatch is available (macOS)
if command -v fswatch >/dev/null 2>&1; then
    echo "Using fswatch for file monitoring..."
    fswatch -o -r -e ".*" -i "\\.($WATCH_EXTENSIONS)$" $WATCH_DIRS | while read f; do
        build_project
    done
# Check if entr is available (cross-platform)
elif command -v entr >/dev/null 2>&1; then
    echo "Using entr for file monitoring..."
    find $WATCH_DIRS -name "*.c" -o -name "*.h" -o -name "*.json" -o -name "*.txt" -o -name "*.glsl" -o -name "*.metal" -o -name "*.frag" -o -name "*.vert" -o -name "Makefile" | entr -r sh -c 'build_project'
# Fallback to basic polling
else
    echo "Neither fswatch nor entr found. Installing fswatch is recommended for macOS."
    echo "You can install it with: brew install fswatch"
    echo "Or install entr with: brew install entr"
    echo ""
    echo "Using basic polling (less efficient)..."
    
    # Simple polling fallback
    last_modified=0
    while true; do
        current_modified=$(find $WATCH_DIRS -name "*.c" -o -name "*.h" -o -name "*.json" -o -name "*.txt" -o -name "*.glsl" -o -name "*.metal" -o -name "*.frag" -o -name "*.vert" -o -name "Makefile" -newer .build_check 2>/dev/null | wc -l)
        
        if [ "$current_modified" -gt 0 ]; then
            touch .build_check
            build_project
        fi
        
        sleep 2
    done
fi
