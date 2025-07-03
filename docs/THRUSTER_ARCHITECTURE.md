# Thruster Architecture Design

## Vision
Gyroscopically stabilized ships controlled by realistic RCS (Reaction Control System) thrusters. Ships should feel like they have mass and require proper thrust application to move and rotate.

## Thruster Types

### 1. Main Engines
- **Purpose**: Forward/backward thrust
- **Visual**: Large glowing spheres with exhaust cones
- **Color**: Blue-white for plasma/ion engines
- **Attachment**: Rear of ship, typically 2-4 engines

### 2. RCS Thrusters  
- **Purpose**: Rotation and fine maneuvering
- **Visual**: Small glowing spheres
- **Color**: Orange-yellow for chemical thrusters
- **Attachment**: Distributed around ship hull for 6DOF control
- **Groups**:
  - Pitch: Top/bottom pairs at front and rear
  - Yaw: Left/right pairs at front and rear  
  - Roll: Diagonal placement for torque
  - Translation: Forward/backward/up/down/left/right

### 3. Maneuvering Thrusters
- **Purpose**: Lateral movement without rotation
- **Visual**: Medium glowing spheres
- **Color**: Green-white 
- **Attachment**: Sides, top, bottom of ship

## Attachment Point System

### Ship Model Requirements
```c
typedef struct {
    Vector3 position;      // Local position on ship
    Vector3 direction;     // Thrust direction (normalized)
    float max_thrust;      // Maximum thrust force
    ThrusterType type;     // MAIN, RCS, MANEUVERING
    uint32_t group_id;     // For coordinated firing
} ThrusterAttachmentPoint;
```

### Example Configuration (Fighter Ship)
```
Main Engines (2):
  - Position: (-1.5, -0.5, -3.0), Direction: (0, 0, 1), Force: 10000N
  - Position: ( 1.5, -0.5, -3.0), Direction: (0, 0, 1), Force: 10000N

RCS Thrusters (12):
  Pitch Control:
    - Pos: ( 0.0,  1.0,  2.5), Dir: (0, -1, 0), Force: 500N  // Front top
    - Pos: ( 0.0, -1.0,  2.5), Dir: (0,  1, 0), Force: 500N  // Front bottom
    - Pos: ( 0.0,  1.0, -2.5), Dir: (0, -1, 0), Force: 500N  // Rear top
    - Pos: ( 0.0, -1.0, -2.5), Dir: (0,  1, 0), Force: 500N  // Rear bottom
  
  Yaw Control:
    - Pos: ( 2.0, 0.0,  2.5), Dir: (-1, 0, 0), Force: 500N  // Front right
    - Pos: (-2.0, 0.0,  2.5), Dir: ( 1, 0, 0), Force: 500N  // Front left
    - Pos: ( 2.0, 0.0, -2.5), Dir: (-1, 0, 0), Force: 500N  // Rear right
    - Pos: (-2.0, 0.0, -2.5), Dir: ( 1, 0, 0), Force: 500N  // Rear left
  
  Roll Control:
    - Pos: ( 2.0,  0.5, 0.0), Dir: (0, -1, 0), Force: 300N  // Right top
    - Pos: ( 2.0, -0.5, 0.0), Dir: (0,  1, 0), Force: 300N  // Right bottom
    - Pos: (-2.0,  0.5, 0.0), Dir: (0,  1, 0), Force: 300N  // Left top
    - Pos: (-2.0, -0.5, 0.0), Dir: (0, -1, 0), Force: 300N  // Left bottom
```

## Visual Design

### Thruster Mesh (Low-Poly Sphere)
```
Vertices: 12 (icosahedron base)
Faces: 20 triangles
Material: Emissive with bloom effect
Glow Intensity: Proportional to thrust output
Size: Scales with thrust intensity
```

### Exhaust Effect
- Particle system or cone mesh behind thruster
- Length proportional to thrust
- Color matches thruster type
- Heat distortion shader effect

## Control System Integration

### Gyroscopic Stabilization
1. **No Input**: RCS fires to counter current rotation
2. **Input Applied**: RCS fires to achieve desired rotation
3. **Input Released**: RCS fires opposite thrust to stop rotation
4. **Result**: Ship stops rotating when stick returns to center

### Thrust Calculation
```c
// For each desired rotation axis
for each thruster in rcs_group {
    if (thruster.can_provide_torque(desired_axis)) {
        float efficiency = dot(thruster.direction, needed_direction);
        float thrust = desired_torque * efficiency / lever_arm;
        thruster.fire(thrust);
    }
}
```

## Implementation Plan

### Phase 1: Visual Thruster System
1. Create low-poly sphere mesh for thrusters
2. Add emissive material support
3. Replace cube placeholders with spheres
4. Add glow intensity based on thrust

### Phase 2: Attachment Point System  
1. Define attachment points in ship data
2. Create thrusters at attachment points
3. Link visual thrusters to physics thrusters

### Phase 3: Realistic RCS Control
1. Calculate which thrusters to fire for desired motion
2. Implement thrust vectoring
3. Add thruster response delay
4. Fine-tune gyroscopic stabilization

### Phase 4: Effects and Polish
1. Add particle exhaust effects
2. Implement heat distortion
3. Add thruster startup/shutdown animations
4. Sound effects for different thruster types

## Prototype Scene: "thruster_test"

A simple scene with:
- Single ship with full RCS configuration
- Debug visualization of thrust vectors
- UI to control individual thruster groups
- Performance metrics for thrust efficiency
- Visual feedback for all thruster types

This allows testing and tuning the thruster system before integrating into gameplay.