# Sprint 21: Control Improvements Research

## Current Control Issues (January 2025)

### Observed Problems
1. **Left stick adding thrust** - Should only control pitch/yaw, not thrust
2. **Right trigger not providing enough thrust** - Values seem dampened
3. **Right stick should control camera** - Currently controls strafe/vertical
4. **Ship-camera alignment missing** - Ship should align to camera direction over time
5. **Turning with left stick not working** - Banking code may be interfering

### Debug Output Analysis
From the test output:
```
🕹️ RAW: LS(0.187,0.000) RS(0.000,0.000) LT:0.490 RT:0.988
   MAPPED: thrust=0.498 pitch=0.000 yaw=0.052 roll=0.000 strafe=0.000
```

Issues identified:
- Left stick X (0.187) is somehow contributing to thrust
- Right trigger (0.988) only maps to thrust=0.498 (should be closer to 1.0)
- Yaw value (0.052) seems too low for stick input

## Level 4 Control System Design

### Concept: Target-Based Control
Instead of directly mapping inputs to forces, implement a control system where:
1. **Input Layer** - Controller/keyboard sets desired targets
2. **Flight Computer Layer** - Calculates required forces to achieve targets
3. **Engine Layer** - Applies forces with realistic constraints
4. **Feedback Layer** - Adjusts based on current state

### Proposed Architecture

```
Input → Target State → Flight Computer → Engine Commands → Physics
         ↑                                                      ↓
         └──────────────── Feedback Loop ←────────────────────┘
```

### Target State Components
```c
typedef struct {
    // Desired velocities
    Vector3 target_velocity;        // World-space velocity target
    Vector3 target_angular_velocity; // Rotation rates
    
    // Desired orientation
    Quaternion target_orientation;   // Where we want to face
    float orientation_priority;      // How quickly to align (0-1)
    
    // Control modes
    bool velocity_relative_to_camera; // Fly relative to view
    bool auto_level;                 // Return to level flight
    bool flight_assist;             // Dampening and stability
    
    // Limits
    float max_velocity;
    float max_angular_velocity;
    float max_acceleration;
} FlightTargetState;
```

### Flight Computer Components
```c
typedef struct {
    // PID controllers for each axis
    PIDController linear_pid[3];
    PIDController angular_pid[3];
    
    // State estimation
    Vector3 estimated_velocity;
    Vector3 estimated_angular_velocity;
    
    // Control authority
    float linear_authority;  // 0-1, how much control we have
    float angular_authority; // 0-1, affected by damage/power
    
    // Smart features
    bool collision_avoidance;
    bool auto_stabilization;
    float g_force_limit;
} FlightComputer;
```

## Improved Control Mapping

### Xbox Controller Layout
```
Left Stick:
  - X: Turn rate (yaw)
  - Y: Pitch rate
  
Right Stick:
  - X: Camera orbit horizontal
  - Y: Camera orbit vertical
  
Triggers:
  - RT: Forward thrust (0-100%)
  - LT: Reverse thrust (0-100%)
  
Bumpers:
  - RB: Boost modifier
  - LB: Precision mode
  
Face Buttons:
  - A: Toggle flight assist
  - B: Brake (zero velocity)
  - X: Match velocity to target
  - Y: Toggle camera mode
  
D-Pad:
  - Up/Down: Vertical thrust
  - Left/Right: Lateral thrust
```

### Camera-Ship Alignment System
```c
typedef struct {
    // Alignment parameters
    float alignment_rate;        // How fast ship turns to face camera
    float camera_follow_rate;    // How fast camera follows ship
    float alignment_deadzone;    // Angle before alignment kicks in
    
    // Mode flags
    bool ship_follows_camera;    // Ship turns to face where camera looks
    bool camera_follows_ship;    // Camera stays behind ship
    bool free_look_mode;        // Decouple camera from ship
    
    // Smoothing
    Quaternion smoothed_target;
    float smoothing_factor;
} CameraAlignmentSystem;
```

## Implementation Plan

### Phase 1: Fix Current Issues
1. **Separate thrust from left stick**
   - Review `input.c` mapping logic
   - Ensure left stick only affects pitch/yaw
   
2. **Fix right trigger scaling**
   - Check trigger dead zones and scaling
   - Ensure full 0-1 range is used
   
3. **Implement right stick camera control**
   - Add camera orbit controls
   - Separate from ship movement

### Phase 2: Implement Flight Computer
1. **Create PID controllers**
   - One for each linear axis
   - One for each angular axis
   - Tunable parameters
   
2. **Add target state system**
   - Calculate desired vs actual
   - Generate correction forces
   
3. **Implement constraints**
   - Max acceleration limits
   - G-force limits for pilot safety
   - Power/heat management

### Phase 3: Advanced Features
1. **Camera-ship alignment**
   - Smooth orientation blending
   - Multiple alignment modes
   
2. **Flight assist modes**
   - Velocity dampening
   - Auto-stabilization
   - Collision prediction
   
3. **Context-aware controls**
   - Different responses in combat vs cruise
   - Adaptive sensitivity based on speed

## Research References

### Games with Excellent 6DOF Controls
1. **Elite Dangerous** - Complex but learnable flight model
2. **Star Citizen** - Physics-based with flight assists
3. **Everspace** - Arcade-style 6DOF with good game feel
4. **Chorus** - Modern arcade space combat with drift mechanics
5. **House of the Dying Sun** - Simplified but satisfying controls

### Key Principles from Research
1. **Predictability** - Player can anticipate ship behavior
2. **Responsiveness** - Immediate feedback with realistic inertia
3. **Forgiveness** - Flight assist prevents loss of control
4. **Depth** - Simple to learn, hard to master
5. **Context** - Controls adapt to situation (combat/cruise/landing)

## Recommended Next Steps

1. **Immediate Fixes** (Sprint 21 completion)
   - Fix left stick thrust contamination
   - Fix right trigger scaling
   - Add right stick camera control
   
2. **Sprint 22 Planning**
   - Prototype PID-based flight computer
   - Implement camera-ship alignment
   - Add flight assist toggles
   
3. **Future Enhancements**
   - Context-aware control sensitivity
   - Advanced flight modes
   - Damage-based control degradation

## Code Investigation Points

### Files to Review
- `src/system/input.c:76-108` - Gamepad mapping logic
- `src/system/gamepad_real.c:142-147` - Trigger processing
- `src/system/control.c` - Banking model implementation
- `src/scripts/canyon_racer_scene.c` - Camera control code

### Specific Issues to Fix
1. Line 103-104 in `input.c` - Check why left stick affects thrust
2. Line 144-147 in `gamepad_real.c` - Verify trigger scaling
3. Banking model in `control.c` - May be interfering with direct control

## Testing Methodology

### Control Feel Metrics
1. **Responsiveness** - Time from input to visible change
2. **Precision** - Ability to make small adjustments
3. **Predictability** - Consistent behavior
4. **Recovery** - How easy to correct mistakes
5. **Flow** - Smooth transitions between maneuvers

### Test Scenarios
1. **Precision flying** - Navigate through obstacles
2. **Combat maneuvering** - Quick direction changes
3. **Long distance travel** - Sustained flight
4. **Docking** - Fine control at low speed
5. **Emergency stops** - Brake effectiveness

## Conclusion

The current control system needs evolution from direct input mapping to a more sophisticated target-based system. This will provide better game feel while maintaining the physics-based flight model. The key is balancing realism with fun, ensuring controls are intuitive for arcade players while offering depth for simulation enthusiasts.