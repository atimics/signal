# Simplified Thruster System Usage Guide

## Overview

The simplified thruster system provides a single-body physics approach to spacecraft propulsion. Thrusters are defined as attachment points on a mesh with force application for realistic 6DOF movement.

## Key Components

### 1. Thruster Definition Files (.thrusters)

Located in `data/thrusters/`, these files define thruster positions and properties:

```
# Format: thruster_name pos_x pos_y pos_z dir_x dir_y dir_z max_force
main_left      -0.40  -0.20  -2.00    0.0  0.0  -1.0   10000.0
main_right      0.40  -0.20  -2.00    0.0  0.0  -1.0   10000.0
```

- **Position**: Must match a vertex on the corresponding mesh (within tolerance)
- **Direction**: Normalized thrust vector in local space
- **Max Force**: Maximum thrust in Newtons

### 2. Validation Tool

The compile-time validation tool ensures thruster positions match mesh vertices:

```bash
# Validate all thruster files
make validate-thrusters

# Validate specific file
./build/validate_thrusters data/thrusters/ship.thrusters assets/meshes/
```

### 3. Component System

```c
// Create thruster component
ThrusterPointsComponent* thrusters = thruster_points_component_create("data/thrusters/wedge_ship_mk2.thrusters");

// Initialize with mesh validation
const Mesh* ship_mesh = assets_get_mesh(registry, "wedge_ship_mk2");
thruster_points_component_init(thrusters, ship_mesh);

// Update thrust commands
Vector3 linear_cmd = {0, 0, -1};  // Forward thrust
Vector3 angular_cmd = {0, 0, 0};  // No rotation
thruster_points_component_update(thrusters, &linear_cmd, &angular_cmd);

// Apply forces to physics
thruster_points_component_apply_forces(thrusters, transform, physics);
```

## Thruster Groups

The system automatically creates control groups based on thruster properties:

- **MAIN**: Large thrust engines (>2000N) pointing backward
- **RCS**: Smaller thrusters for maneuvering
- **DIRECTIONAL**: Forward, backward, left, right, up, down
- **ROTATIONAL**: Pitch, yaw, roll control

## Creating Thruster Files

1. **Identify Mesh Vertices**:
   ```bash
   grep "^v " assets/meshes/props/ship_name/geometry.obj
   ```

2. **Create .thrusters File**:
   - Place thrusters at exact vertex positions
   - Orient thrust directions appropriately
   - Balance placement for stable control

3. **Validate**:
   ```bash
   ./build/validate_thrusters data/thrusters/ship_name.thrusters assets/meshes/
   ```

## Physics Integration

The system calculates forces and torques based on:
- Thruster position relative to center of mass
- Thrust direction in world space
- Current throttle level (0-1)

Forces are applied using:
```c
physics_add_force(physics, total_force);
physics_add_torque(physics, total_torque);
```

## Visual Debugging

Enable visual feedback:
```c
thruster_component->visual_debug = true;
```

This displays thrust forces and positions in the console.

## Example: Wedge Ship Mk2

The wedge_ship_mk2.thrusters file demonstrates:
- 2 main engines for forward thrust
- 7 RCS thrusters for 6DOF control
- All positions validated against mesh vertices
- Balanced placement for stable flight

## Performance

- O(n) complexity for n thrusters
- No joint simulation overhead
- Efficient force accumulation
- Suitable for 50+ thrusters per ship

## Future Enhancements

- Visual thrust cone rendering
- Gimbal support for thrust vectoring
- Fuel consumption tracking
- Damage modeling per thruster