# SIGNAL Flight Control Manual

**Document ID**: CONTROLS_V1  
**Date**: July 6, 2025  
**Author**: Development Team  
**Status**: Active  
**Version**: 1.0

---

## 🎮 Control Schemes Overview

SIGNAL features a sophisticated unified flight control system designed for intuitive 6-degree-of-freedom (6DOF) spacecraft control. The system supports multiple input devices with primary optimization for Xbox-style controllers.

### Primary Control Device: Xbox Controller

The game is optimized for Xbox Series X|S controllers with full Bluetooth HID support.

#### Flight Controls

| Control | Action | Range | Notes |
|---------|--------|-------|-------|
| **Right Trigger (RT)** | Thrust Forward | 0-100% | Variable thrust control |
| **Left Trigger (LT)** | Thrust Reverse/Brake | 0-100% | Deceleration and reverse |
| **Left Stick Y** | Pitch Up/Down | ±100% | Pull back to climb |
| **Left Stick X** | Yaw Left/Right | ±100% | Turn horizontally |
| **Right Stick X** | Roll Left/Right | ±100% | Bank/barrel roll |
| **Right Stick Y** | Vertical Thrust | ±100% | Up/down translation |
| **A Button** | Boost | On/Off | 3x thrust multiplier |
| **B Button** | Emergency Brake | On/Off | Full stop dampening |

#### Advanced Features

**Auto-Banking System**
- When yawing, the ship automatically banks into turns
- Banking Ratio: 1.8x roll per unit of yaw input
- Manual roll input overrides auto-banking
- Only active in Assisted flight mode

**Dead Zone Filtering**
- Default: 10% on all analog inputs
- Eliminates controller drift and accidental inputs
- Configurable per-axis (future feature)

### Secondary Control Device: Keyboard

Full keyboard support for players without controllers.

| Key | Action | Notes |
|-----|--------|-------|
| **W** | Thrust Forward | Digital on/off |
| **S** | Thrust Reverse/Brake | Digital on/off |
| **A** | Yaw Left | Turn left |
| **D** | Yaw Right | Turn right |
| **Q** | Roll Left | Bank left |
| **E** | Roll Right | Bank right |
| **↑** | Pitch Up | Climb |
| **↓** | Pitch Down | Dive |
| **Space** | Vertical Up | Translate up |
| **Ctrl** | Vertical Down | Translate down |
| **Shift** | Boost | Hold for 3x thrust |
| **Tab** | Emergency Brake | Activate dampeners |

---

## 🚀 Flight Modes

The unified control system supports multiple flight modes with different assistance levels:

### Manual Mode
- **Stability Assist**: 2% (minimal)
- **Inertia Dampening**: 0% (none)
- **Auto-Banking**: Disabled
- **Target Users**: Expert pilots, precision flying
- **Characteristics**: Raw, responsive, requires skill

### Assisted Mode (Default)
- **Stability Assist**: 15% (moderate)
- **Inertia Dampening**: 10% (gentle)
- **Auto-Banking**: Enabled (1.8x ratio)
- **Target Users**: Most players, balanced gameplay
- **Characteristics**: Intuitive, forgiving, coordinated turns

### Autonomous Mode
- **Stability Assist**: 100% (maximum)
- **Inertia Dampening**: 80% (heavy)
- **Auto-Banking**: Enabled
- **Target Users**: AI-controlled entities
- **Characteristics**: Stable, predictable, computer-controlled

---

## 🎯 Advanced Flight Techniques

### Banking Turns
Banking turns provide natural, aircraft-like flight characteristics:

1. **Initiate Turn**: Apply yaw input (A/D keys or Left Stick X)
2. **Automatic Banking**: Ship rolls into the turn automatically
3. **Tighten Turn**: Add manual roll input for sharper turns
4. **Level Out**: Release yaw input to auto-level

**Banking Calculation**:
```
banking_roll = -yaw_input * 1.8
final_roll = manual_roll + banking_roll (clamped to ±1.0)
```

### Boost Management
The boost system provides emergency acceleration:

- **Boost Multiplier**: 3x thrust on all axes
- **Activation**: A button (controller) or Shift key (keyboard)
- **Combined Effects**: Works with directional thrust
- **Tactical Use**: Escape maneuvers, high-speed turns

### 6DOF Movement Patterns

**Standard Flight**:
- Primary: Thrust + Pitch/Yaw (traditional aircraft-style)
- Secondary: Roll for coordinated turns

**Advanced Maneuvers**:
- **Vertical Thrust**: Right Stick Y for up/down translation
- **Strafing**: Combined yaw + vertical for lateral movement
- **Corkscrew**: Roll + vertical thrust while turning

---

## 📊 Technical Specifications

### Input Processing Pipeline

```
Hardware Input → HAL Layer → Input Service → Flight Control → Thrusters → Physics
```

### Response Characteristics

| Parameter | Manual Mode | Assisted Mode | Notes |
|-----------|-------------|---------------|-------|
| **Dead Zone** | 10% | 10% | Uniform across all axes |
| **Sensitivity** | 1.0x | 1.0x | Linear response curve |
| **Banking Ratio** | 0.0 | 1.8x | Auto-roll per yaw input |
| **Stability Assist** | 2% | 15% | Counter-rotation force |
| **Inertia Dampening** | 0% | 10% | Velocity dampening |

### Performance Targets

| Metric | Target | Status |
|--------|--------|--------|
| **Input Latency** | <16ms | ✅ Achieved |
| **Control Update Rate** | 60Hz | ✅ Locked |
| **Banking Response Time** | <100ms | ✅ Measured |
| **Dead Zone Accuracy** | ±1% | ✅ Tested |

---

## 🛠️ Configuration Options

### Sensitivity Adjustment
Future versions will support per-axis sensitivity tuning:

```c
// Linear sensitivity (thrust, vertical, strafe)
control->input_config.linear_sensitivity = 1.5f;

// Angular sensitivity (pitch, yaw, roll)  
control->input_config.angular_sensitivity = 0.8f;
```

### Dead Zone Customization
```c
// Global dead zone (0.0 - 0.5 range)
control->input_config.dead_zone = 0.15f;
```

### Response Curves
```c
// Quadratic response for fine control
control->input_config.use_quadratic_curve = true;
```

---

## 🎯 Best Practices

### For New Players (Assisted Mode)
1. Start with default settings
2. Use triggers for primary thrust control
3. Let auto-banking handle coordinated turns
4. Practice vertical thrust for landing/docking

### For Experienced Players (Manual Mode)
1. Disable banking for maximum control
2. Master manual roll for complex maneuvers
3. Use minimal assistance for precision flying
4. Combine boost with directional thrust for advanced techniques

### For Development
1. Test all control combinations
2. Validate input ranges and clamping
3. Monitor performance metrics
4. Ensure consistent behavior across modes

---

## 🐛 Troubleshooting

### Controller Not Detected
- Verify Xbox controller is properly connected
- Check HID driver installation
- Restart game if controller was connected after launch

### Poor Responsiveness
- Check dead zone settings (may be too high)
- Verify controller batteries
- Test with keyboard controls to isolate issue

### Unexpected Banking
- Ensure you're in Assisted mode if banking is desired
- Switch to Manual mode to disable auto-banking
- Check for conflicting manual roll input

---

## 📚 Related Documentation

- **Architecture Guide**: Control System Architecture (CONTROL_ARCH_V1)
- **Test Plan**: Control Scheme Test Plan (TEST_CONTROL_V1)
- **API Reference**: Unified Flight Control API
- **Input Mapping**: Hardware Input Abstraction Layer

---

*This document is part of the SIGNAL technical documentation suite. For updates and revisions, see the project repository.*
