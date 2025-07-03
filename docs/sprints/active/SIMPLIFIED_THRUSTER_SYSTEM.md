# Simplified Thruster System Design

## Core Concept
Single rigid body with thruster attachment points defined directly in mesh data. No complex multi-body physics - just force application at specific points.

## File Format

### Mesh File (.mesh)
Standard mesh data with vertices, normals, UVs, indices

### Thruster Definition File (.thrusters)
```
# thruster_name, position_x, position_y, position_z, direction_x, direction_y, direction_z, max_force
main_left    -2.0  -0.5  -3.0   0.0  0.0  -1.0   5000.0
main_right    2.0  -0.5  -3.0   0.0  0.0  -1.0   5000.0
rcs_forward   0.0   0.0   3.0   0.0  0.0   1.0    500.0
rcs_up        0.0   1.5   0.0   0.0  1.0   0.0    500.0
rcs_down      0.0  -1.5   0.0   0.0 -1.0   0.0    500.0
rcs_left     -2.5   0.0   0.0  -1.0  0.0   0.0    500.0
rcs_right     2.5   0.0   0.0   1.0  0.0   0.0    500.0
```

## Compilation Step
1. Parse .thrusters file
2. Load corresponding .mesh file
3. Validate each thruster position exists as a vertex in the mesh (with tolerance)
4. Generate compiled .cthrusters binary file with validated data

## Runtime System

### ThrusterPoint Structure
```c
typedef struct {
    Vector3 position;      // Position on mesh (local space)
    Vector3 direction;     // Thrust direction (normalized)
    float max_force;       // Maximum thrust force (N)
    float current_thrust;  // Current thrust level (0-1)
} ThrusterPoint;
```

### Ship Physics
```c
typedef struct {
    // Single rigid body
    Vector3 position;
    Quaternion rotation;
    Vector3 velocity;
    Vector3 angular_velocity;
    
    // Physical properties
    float mass;
    Matrix3 inertia_tensor;
    
    // Thrusters
    ThrusterPoint* thrusters;
    int num_thrusters;
} ShipPhysics;
```

## Physics Application
For each thruster:
1. Transform position to world space
2. Transform direction to world space
3. Apply force at position: `physics_add_force_at_point(force, world_position)`
4. This automatically creates appropriate torques

## Advantages
- Simple single-body physics
- Easy to author (just list positions)
- Compile-time validation
- Efficient runtime
- Works with existing physics system
- Visual debugging is straightforward

## Implementation Steps
1. Create thruster file parser
2. Add mesh vertex validation
3. Create binary compiler
4. Implement runtime loader
5. Update physics system to use thruster points
6. Add visual debugging