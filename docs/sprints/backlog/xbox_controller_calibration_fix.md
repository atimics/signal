# Xbox Controller Calibration Fix

## Issue Summary
Xbox Wireless Controller (PID:0x0B13) exhibited stick drift causing the ship to spin continuously when no input was being applied. The controller's analog sticks had non-standard rest positions that were not being properly calibrated.

## Root Cause Analysis

### HID Report Analysis
From the raw HID data, we identified the following rest positions:
- Left Stick X (byte 6): 126 (expected 128)
- Left Stick Y (byte 7): 71 (expected 128) 
- Right Stick X (byte 8): 126 (expected 128)
- Right Stick Y (byte 9): 0 (expected 128)

These significant deviations from the expected center value of 128 were causing constant input even when the sticks were at rest.

### Debug Output Example
```
🎮 HID Report (size=14): 01 00 3C F0 00 00 7E 47 7E 00 00 FF 03 00
   Bytes: [0]=1 [1]=0 [2]=60 [3]=240 [4]=0 [5]=0 [6]=126 [7]=71 [8]=126 [9]=0 [10]=0 [11]=255 [12]=3 [13]=0
   Interpreted: LT=0.01 RT=1.00 LS(-0.72,0.60) RS(0.01,-1.00)
```

## Solution Implementation

### 1. Created Calibrated Deadzone Function
Added a new function to handle stick positions with known rest offsets:

```c
static float apply_calibrated_deadzone(float value, float rest_position, float deadzone) {
    // First, recenter based on rest position
    float centered = value - rest_position;
    
    // Then apply regular deadzone
    if (fabsf(centered) < deadzone) {
        return 0.0f;
    }
    
    // Scale remaining range
    float sign = (centered < 0.0f) ? -1.0f : 1.0f;
    float abs_value = fabsf(centered);
    return sign * ((abs_value - deadzone) / (1.0f - deadzone));
}
```

### 2. Applied Calibration to Xbox Controller Parsing
Updated the Xbox Wireless Controller mapping to use calibrated deadzones:

```c
// Apply calibrated deadzone for known rest positions
// Left stick X appears to be at 126 when centered
float left_x_raw = (data[6] - 128) / 128.0f;
left_x = (int16_t)(apply_calibrated_deadzone(left_x_raw, -0.016f, 0.25f) * 32767);

// Left stick Y rests at 71 (not 128)
float left_y_raw = (data[7] - 128) / 128.0f;
left_y = (int16_t)(apply_calibrated_deadzone(left_y_raw, -0.445f, 0.25f) * 32767);

// Right stick X at byte 8, appears centered properly
float right_x_raw = (data[8] - 128) / 128.0f;
right_x = (int16_t)(apply_calibrated_deadzone(right_x_raw, -0.016f, 0.25f) * 32767);

// Right stick Y rests at 0 (not 128)
float right_y_raw = (data[9] - 128) / 128.0f;
right_y = (int16_t)(apply_calibrated_deadzone(right_y_raw, -1.0f, 0.25f) * 32767);
```

### 3. Increased Deadzone
Also increased the general deadzone from 20% to 25% to provide additional drift protection:
```c
static float axis_deadzone = 0.25f;  // Increased to handle stick drift
#define GAMEPAD_DEADZONE 0.25f          // 25% deadzone (increased for Xbox drift)
```

## Results
- Ship no longer spins when controller is idle
- Right trigger provides smooth thrust control without interference
- Stick inputs are properly centered and respond only to intentional movement
- Banking controls work as intended without drift

## Technical Details

### Rest Position Calculations
- Left X: 126/128 = -0.016 offset
- Left Y: 71/128 = -0.445 offset  
- Right X: 126/128 = -0.016 offset
- Right Y: 0/128 = -1.0 offset

### Future Improvements
1. Auto-calibration routine that detects rest positions on controller connect
2. Per-controller calibration profiles saved to disk
3. User-adjustable deadzone settings in game options
4. Motion-based deadzone detection as suggested by user

## Files Modified
- `/src/system/gamepad.c` - Added calibrated deadzone function and updated Xbox mapping
- `/src/system/input.c` - Increased GAMEPAD_DEADZONE constant

## Testing
Confirmed fix with Xbox Wireless Controller (PID:0x0B13) on macOS:
- No drift when idle
- Smooth control response
- Proper thrust on right trigger
- Banking turns work correctly