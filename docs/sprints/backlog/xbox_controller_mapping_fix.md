# Xbox Controller Mapping Fix

## Issue
Xbox Wireless Controller (PID:0x0B13) was causing the ship to spin when the right trigger was pressed, instead of providing forward thrust.

## Root Cause
The Xbox Wireless Controller uses a different HID report layout than standard Xbox controllers:
- Standard layout: Triggers at bytes 4-5, Sticks at bytes 6-13
- Xbox Wireless: Sticks at bytes 4-11, Triggers at bytes 12-13

## Solution
Added controller-specific mapping in `src/system/gamepad.c`:

```c
// Xbox Wireless Controller (PID:0x0B13) sometimes has triggers at end
if (gamepad->product_id == 0x0B13) {
    // Alternate mapping
    left_x = *((int16_t*)(data + 4));
    left_y = *((int16_t*)(data + 6));
    right_x = *((int16_t*)(data + 8));
    right_y = *((int16_t*)(data + 10));
    
    // Triggers at bytes 12-13 (0-255 range)
    left_trigger = data[12] / 255.0f;
    right_trigger = data[13] / 255.0f;
}
```

## Verification
When running with Xbox Wireless Controller, debug output will show:
```
🎮 ALT MAPPING: Triggers[12-13]=X,Y Sticks[4-11]=(X,Y),(X,Y)
```

This confirms the alternate mapping is active.

## Impact
- Right trigger now correctly provides forward thrust
- Left trigger provides reverse thrust
- No unwanted rotation from trigger input