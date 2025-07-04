# Gamepad Hotplug Test Instructions

## Testing Mid-Game Gamepad Connection

The gamepad hotplug detection system is now fully integrated and will:

1. **Check for new gamepad connections every second** while the game is running
2. **Automatically update the UI** to show gamepad control hints when a controller is connected
3. **Switch back to keyboard hints** when all controllers are disconnected
4. **Recalibrate the input processor** when a new gamepad is connected

### How to Test:

1. Start the game without a gamepad connected:
   ```bash
   ./build/cgame --scene navigation_menu
   ```

2. The UI should show keyboard controls (Arrow keys/WASD, Enter, Escape)

3. While the game is running, connect your Xbox controller or compatible gamepad

4. Within 1 second, you should see:
   - Console message: "🎮 HOTPLUG: Gamepad 0 connected!"
   - UI control hints change to gamepad buttons
   - The navigation menu will show "Gamepad Connected"

5. Use the gamepad to navigate the menu:
   - D-pad or left stick to move selection
   - A button to select
   - B button to go back

6. Disconnect the gamepad while the game is running

7. Within 1 second, you should see:
   - Console message: "🎮 HOTPLUG: Gamepad 0 disconnected"
   - UI control hints change back to keyboard
   - Console: "No gamepads remaining - switching to keyboard/mouse"

### What's Happening Behind the Scenes:

- `gamepad_update_hotplug()` is called every frame in `input_update()`
- Every second, it scans for new HID devices
- When a supported gamepad is detected, it:
  - Opens the device handle
  - Initializes the gamepad state
  - Calls the connected callback
  - Updates the UI to show gamepad controls
- When a gamepad is disconnected:
  - The disconnected callback is called
  - If no gamepads remain, switches to keyboard mode
  - Updates the UI to show keyboard controls

### Supported Gamepads:
- Xbox One Controller (USB)
- Xbox Series X/S Controller (USB)
- Xbox 360 Controller
- Other XInput-compatible controllers

The system will automatically detect and configure these controllers when plugged in via USB.