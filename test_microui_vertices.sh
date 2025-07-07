#!/bin/bash
# Test script to debug MicroUI vertex generation
# Run cgame with navigation_menu as startup scene to see UI immediately

echo "🧪 Testing MicroUI vertex generation..."
echo "Setting navigation_menu as startup scene..."

# Create a temporary config that starts with navigation_menu
cp cgame_config.txt cgame_config.txt.backup
sed -i '' 's/startup_scene=.*/startup_scene=navigation_menu/' cgame_config.txt
sed -i '' 's/auto_start=.*/auto_start=true/' cgame_config.txt

echo "Running cgame with UI debug output..."
./build/cgame 2>&1 | grep -E "MicroUI|ui_render|vertices|commands|clip_stack" | head -100

# Restore original config
mv cgame_config.txt.backup cgame_config.txt

echo "✅ Test complete"