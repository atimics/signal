# CGame Flight Controls Manual

**Sprint 25 Implementation** - Direct Keyboard Flight Controls

## Overview

CGame features a sophisticated 6-degrees-of-freedom (6DOF) flight control system that allows precise maneuvering in a 3D space. The controls are designed for intuitive flight with banking turns and responsive control authority.

## Flight Controls

### WASD + QE Layout (Primary)
- **W** - Forward Thrust
- **S** - Backward Thrust  
- **A** - Yaw Left (turn left)
- **D** - Yaw Right (turn right)
- **Q** - Roll Left
- **E** - Roll Right

### Arrow Keys (Secondary)
- **↑ Up Arrow** - Pitch Up (nose up)
- **↓ Down Arrow** - Pitch Down (nose down)

## Control Philosophy

### Banking Flight Model
The flight system emphasizes a "banking turn" approach similar to atmospheric flight:
- Use **A/D** for coordinated turns that feel natural
- Roll (**Q/E**) is available for aerobatic maneuvers and precise orientation control
- Pitch (**Arrow Keys**) provides fine altitude control

### 6DOF Freedom
While optimized for banking flight, the system provides full 6-degrees-of-freedom control:
- **3 Rotational Axes**: Pitch, Yaw, Roll
- **3 Translational Axes**: Forward/Back, Up/Down, Left/Right (via turning)

## Context-Aware Input

The control system uses context-aware input switching:
- **Menu Context**: WASD controls UI navigation
- **Gameplay Context**: WASD controls flight (automatic in flight scenes)
- **Seamless Transition**: No manual mode switching required

## Advanced Features

### Input Filtering
- **Dead Zone Compensation**: Eliminates controller drift
- **Noise Reduction**: Kalman filtering provides smooth control
- **Responsive Design**: Maintains quick response for rapid maneuvers

### Multi-Device Support
- **Keyboard**: Full digital control with precise timing
- **Gamepad**: Analog control with enhanced sensitivity (when implemented)
- **Hot-Plug**: Seamless device switching during gameplay

## Technical Notes

### Coordinate System
- **Forward**: +Z axis (nose direction)
- **Up**: +Y axis (dorsal direction)  
- **Right**: +X axis (starboard direction)

### Force Application
All control inputs are processed through the physics system:
- Controls generate force/torque commands
- Physics system applies realistic flight dynamics
- Thruster system provides visual feedback

## Getting Started

1. **Launch Flight Test Scene**: Navigate to "Flight Test" from the main menu
2. **Basic Movement**: Use **W** to accelerate forward
3. **Turning**: Use **A/D** for natural banking turns
4. **Orientation**: Use **Q/E** for roll adjustments
5. **Altitude**: Use **Arrow Keys** for pitch control

## Troubleshooting

### Controls Not Responsive
- Ensure you're in a gameplay scene (not menu)
- Check that the flight test scene has properly initialized
- Verify no conflicting input devices

### Unexpected Behavior
- The input system automatically switches contexts
- Menu controls (WASD for navigation) are different from flight controls
- Scene transitions handle context switching automatically

---

**Note**: This control scheme was implemented in Sprint 25 as part of the direct keyboard flight handling initiative. For technical implementation details, see `src/services/input_service.c` and `src/input_state.c`.