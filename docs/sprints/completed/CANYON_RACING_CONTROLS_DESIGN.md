# Canyon Racing Control System Design

## Vision
Create a visceral, responsive control system inspired by canyon racing games where the ship flies towards where the player is looking, with smooth camera control and intuitive input mapping.

## Control Scheme

### Gamepad Controls
- **Left Stick**: Direct ship pitch (Y) and yaw (X) control
- **Right Stick**: Camera/look target control (orbit around ship)
- **Right Trigger**: Accelerate towards look target
- **Left Trigger**: Decelerate and auto-level (reset orientation)
- **Bumpers**: Roll control (barrel rolls)
- **A Button**: Boost
- **B Button**: Brake

### Keyboard + Mouse Controls
- **W/S**: Pitch up/down
- **A/D**: Yaw left/right
- **Q/E**: Roll left/right
- **Space**: Accelerate towards look target
- **Shift**: Boost
- **Ctrl**: Brake
- **Mouse Movement**: Rotate look target around ship
- **Mouse Wheel**: Zoom camera in/out

## Technical Architecture

### 1. Look Target System
```c
struct LookTarget {
    Vector3 world_position;      // Where we're looking in world space
    float distance;              // Distance from ship
    float azimuth;              // Horizontal angle
    float elevation;            // Vertical angle
    float smoothing;            // Camera smoothing factor
};
```

### 2. Enhanced Control Authority
```c
struct ControlAuthority {
    // Existing fields...
    
    // New fields for look-based control
    LookTarget look_target;
    Vector3 desired_forward;     // Direction we want to fly
    float look_influence;        // How much look direction affects thrust
    bool auto_level_enabled;     // Auto-leveling on left trigger
    float auto_level_strength;   // How fast to auto-level
};
```

### 3. Control Flow
```
Input → Look Target Update → Thrust Direction Calculation → Physics Forces
                ↓
         Camera Update → Render
```

## Implementation Steps

### Phase 1: Simplify Input System
1. Bypass neural network processing in `input_processing.c`
2. Implement direct dead zone and sensitivity scaling
3. Remove adaptive controllers for now

### Phase 2: Look Target System
1. Add `LookTarget` to `ControlAuthority` component
2. Implement spherical coordinate system for camera orbit
3. Add mouse input handling to SDL event loop
4. Update look target based on right stick/mouse input

### Phase 3: Direction-Based Thrust
1. Calculate desired forward vector from ship to look target
2. Apply thrust in look direction when right trigger/space pressed
3. Implement smooth turning towards look direction
4. Add auto-leveling on left trigger

### Phase 4: Camera System Enhancement
1. Make camera follow look target, not just ship
2. Implement smooth camera transitions
3. Add zoom control with mouse wheel
4. Keep ship in frame while looking around

### Phase 5: Fine-Tuning
1. Adjust control sensitivity curves
2. Add control customization options
3. Implement acceleration/deceleration curves
4. Add visual indicators for look direction

## Key Algorithms

### Look Target Update
```c
void update_look_target(ControlAuthority* auth, float right_x, float right_y, float dt) {
    // Update spherical coordinates
    auth->look_target.azimuth += right_x * LOOK_SENSITIVITY * dt;
    auth->look_target.elevation += right_y * LOOK_SENSITIVITY * dt;
    
    // Clamp elevation to prevent gimbal lock
    auth->look_target.elevation = clamp(auth->look_target.elevation, -PI/3, PI/3);
    
    // Convert to world position
    Vector3 offset = spherical_to_cartesian(
        auth->look_target.distance,
        auth->look_target.azimuth,
        auth->look_target.elevation
    );
    
    auth->look_target.world_position = vector3_add(ship_position, offset);
}
```

### Direction-Based Thrust
```c
void apply_look_based_thrust(ThrusterSystem* thrusters, ControlAuthority* auth, float thrust_input) {
    // Calculate direction from ship to look target
    Vector3 to_target = vector3_normalize(
        vector3_subtract(auth->look_target.world_position, ship_position)
    );
    
    // Apply thrust in that direction
    thrusters->input_linear = vector3_multiply_scalar(to_target, thrust_input);
    
    // Smoothly rotate ship towards look direction
    Quaternion desired_rot = quaternion_look_at(to_target, world_up);
    Quaternion current_rot = transform->orientation;
    
    // Apply torque to turn towards desired orientation
    Vector3 rotation_error = quaternion_to_euler_difference(current_rot, desired_rot);
    thrusters->input_angular = vector3_multiply_scalar(rotation_error, TURN_SPEED);
}
```

### Auto-Level System
```c
void apply_auto_level(ThrusterSystem* thrusters, Transform* transform, float strength) {
    // Get current up vector
    Vector3 current_up = quaternion_rotate_vector(transform->orientation, VECTOR3_UP);
    
    // Calculate error from world up
    Vector3 up_error = vector3_cross(current_up, VECTOR3_UP);
    
    // Apply corrective torque
    thrusters->input_angular = vector3_add(
        thrusters->input_angular,
        vector3_multiply_scalar(up_error, strength)
    );
}
```

## Benefits

1. **Intuitive**: Fly where you look, like in racing games
2. **Visceral**: Direct connection between look and movement
3. **Smooth**: Camera and ship work together harmoniously
4. **Flexible**: Works great with gamepad or mouse+keyboard
5. **Cinematic**: Creates dynamic camera angles during flight

## Testing Plan

1. **Basic Functionality**:
   - Look target updates correctly with input
   - Ship flies towards look target
   - Auto-level works on left trigger
   
2. **Edge Cases**:
   - Looking straight up/down
   - Rapid look direction changes
   - Switching between input devices
   
3. **Performance**:
   - Smooth 60 FPS with new calculations
   - No input lag
   - Proper dead zone handling

## Success Metrics

- Players can navigate tight canyons intuitively
- Controls feel responsive and predictable
- Camera enhances rather than hinders gameplay
- Both gamepad and mouse+keyboard feel equally good
- New players can pick up controls quickly