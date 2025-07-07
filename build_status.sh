#!/bin/bash

# SIGNAL Build Status Checker
# Quick utility to check current build status

BUILD_STATUS_FILE=".build_status"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔍 SIGNAL Build Status${NC}"
echo "===================="

# Check if build status file exists
if [ ! -f "$BUILD_STATUS_FILE" ]; then
    echo -e "${YELLOW}❓ No build status file found${NC}"
    echo "Run './watch_build.sh' to start monitoring builds"
    echo ""
    echo "Checking current build manually..."
    if make > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Manual build check: SUCCESS${NC}"
    else
        echo -e "${RED}❌ Manual build check: FAILED${NC}"
    fi
    exit 0
fi

# Read and parse build status
BUILD_STATUS=$(cat "$BUILD_STATUS_FILE")
STATUS_CODE=$(echo "$BUILD_STATUS" | cut -d'|' -f1)
TIMESTAMP=$(echo "$BUILD_STATUS" | cut -d'|' -f2)
MESSAGE=$(echo "$BUILD_STATUS" | cut -d'|' -f3)

echo "Timestamp: $TIMESTAMP"
echo -n "Status: "

case "$STATUS_CODE" in
    "SUCCESS")
        echo -e "${GREEN}✅ $MESSAGE${NC}"
        ;;
    "FAILED")
        echo -e "${RED}❌ $MESSAGE${NC}"
        if [ -f "build.log" ]; then
            echo ""
            echo "Recent build errors:"
            tail -5 build.log | sed 's/^/  /'
        fi
        ;;
    "BUILDING")
        echo -e "${YELLOW}⏳ $MESSAGE${NC}"
        ;;
    *)
        echo -e "${YELLOW}❓ Unknown status: $MESSAGE${NC}"
        ;;
esac

echo ""
echo "Commands:"
echo "  ./watch_build.sh    - Start auto-build monitoring"
echo "  ./build_status.sh   - Check this status"
echo "  make                - Manual build"
echo "  make run            - Build and run"
