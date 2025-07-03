# YAML Scene Definition Format

This guide documents the YAML format used for defining scenes and entities in CGame.

## Overview

CGame uses YAML files to define game scenes, entities, and their components. This format provides a human-readable way to create game content without writing C code.

## Basic Structure

### Scene File Structure

```yaml
# Scene metadata
name: "Scene Name"
description: "Scene description"

# Entity definitions
entities:
  - name: "Entity Name"
    components:
      transform:
        position: [x, y, z]
        rotation: [x, y, z, w]  # Quaternion
        scale: [x, y, z]
      
      physics:
        mass: 1.0
        enable_6dof: true
        drag: 0.1
        angular_drag: 0.1
        environment: "space"  # or "atmosphere"
      
      mesh:
        asset: "ship_fighter"  # References assets/meshes/index.json
        material: "default"
      
      # Additional components...
```

## Component Types

### Transform Component
Defines position, rotation, and scale in 3D space.

```yaml
transform:
  position: [0.0, 10.0, -50.0]     # X, Y, Z coordinates
  rotation: [0.0, 0.0, 0.0, 1.0]   # Quaternion (X, Y, Z, W)
  scale: [1.0, 1.0, 1.0]           # Scale factors
```

### Physics Component
Controls physics simulation properties.

```yaml
physics:
  mass: 10.0                # Mass in kg
  enable_6dof: true        # Enable 6 degrees of freedom
  drag: 0.1                # Linear drag coefficient
  angular_drag: 0.1        # Angular drag coefficient
  environment: "space"     # Environment type: "space" or "atmosphere"
```

### Mesh Component
Visual representation of the entity.

```yaml
mesh:
  asset: "ship_fighter"    # Asset name from meshes/index.json
  material: "metal"        # Material name
  visible: true           # Visibility flag (optional, default: true)
```

### Thruster System Component
Ship propulsion control.

```yaml
thruster_system:
  max_linear_force: [100.0, 80.0, 120.0]    # X, Y, Z force limits
  max_angular_torque: [50.0, 50.0, 50.0]   # Pitch, yaw, roll torque limits
  thrust_response_time: 0.2                 # Response time in seconds
  atmosphere_efficiency: 0.7                # Efficiency in atmosphere (0-1)
  vacuum_efficiency: 1.0                    # Efficiency in vacuum (0-1)
```

### Control Authority Component
Maps input to thruster commands.

```yaml
control_authority:
  linear_authority: [1.0, 1.0, 1.0]      # Linear control scaling
  angular_authority: [1.0, 1.0, 1.0]     # Angular control scaling
  input_dead_zone: 0.1                   # Dead zone threshold
  input_sensitivity: 1.0                 # Overall sensitivity
```

### Camera Component
Camera configuration for viewing.

```yaml
camera:
  mode: "third_person"           # Camera mode
  fov: 60.0                     # Field of view in degrees
  near_plane: 0.1               # Near clipping plane
  far_plane: 1000.0             # Far clipping plane
  offset: [0.0, 5.0, -10.0]     # Offset from target
```

### AI Component
Basic AI behavior control.

```yaml
ai:
  behavior: "patrol"            # AI behavior type
  aggression: 0.5              # Aggression level (0-1)
  detection_range: 100.0       # Detection radius
  patrol_radius: 200.0         # Patrol area radius
```

## Example Scenes

### Flight Test Scene

```yaml
name: "Flight Test"
description: "Basic flight testing environment"

entities:
  # Player ship
  - name: "Player Ship"
    components:
      transform:
        position: [0.0, 10.0, 0.0]
        rotation: [0.0, 0.0, 0.0, 1.0]
        scale: [1.0, 1.0, 1.0]
      
      physics:
        mass: 1000.0
        enable_6dof: true
        drag: 0.1
        angular_drag: 0.2
        environment: "atmosphere"
      
      mesh:
        asset: "ship_fighter"
      
      thruster_system:
        max_linear_force: [100.0, 80.0, 120.0]
        max_angular_torque: [50.0, 60.0, 40.0]
        thrust_response_time: 0.1
      
      control_authority:
        linear_authority: [1.0, 1.0, 1.0]
        angular_authority: [0.8, 1.0, 0.6]

  # Ground plane
  - name: "Ground"
    components:
      transform:
        position: [0.0, 0.0, 0.0]
        rotation: [0.0, 0.0, 0.0, 1.0]
        scale: [1000.0, 1.0, 1000.0]
      
      mesh:
        asset: "plane"
        material: "ground"
```

### Racing Scene

```yaml
name: "Canyon Race Track 1"
description: "Beginner canyon racing track"

# Global scene settings
settings:
  fog_density: 0.02
  fog_color: [0.8, 0.7, 0.6]
  ambient_light: [0.3, 0.3, 0.4]
  sun_direction: [0.5, -0.8, 0.3]

entities:
  # Starting gate
  - name: "Start Gate"
    components:
      transform:
        position: [0.0, 50.0, 0.0]
      
      mesh:
        asset: "gate_start"
      
      checkpoint:
        index: 0
        is_start: true
        width: 40.0
        height: 20.0

  # Checkpoints
  - name: "Checkpoint 1"
    components:
      transform:
        position: [200.0, 45.0, 100.0]
        rotation: [0.0, 0.707, 0.0, 0.707]  # 90 degree Y rotation
      
      checkpoint:
        index: 1
        width: 30.0
        height: 15.0
```

## Asset References

Assets referenced in YAML files must be defined in `assets/meshes/index.json`:

```json
{
  "meshes": [
    {
      "name": "ship_fighter",
      "path": "meshes/ship_fighter.obj",
      "tags": ["vehicle", "player"],
      "material": "ship_metal.mtl"
    },
    {
      "name": "plane",
      "path": "meshes/plane.obj",
      "tags": ["terrain", "static"]
    }
  ]
}
```

## Loading Scenes

Scenes are loaded using the scene loading API:

```c
// Load a scene from YAML
scene_state_load_yaml(&scene_manager, "assets/scenes/flight_test.yaml");

// Or specify by name in scene transitions
scene_state_request_transition(&scene_manager, "flight_test");
```

## Best Practices

1. **Use Descriptive Names**: Give entities meaningful names for debugging
2. **Group Related Entities**: Use comments to organize sections
3. **Validate References**: Ensure all asset references exist
4. **Test Incrementally**: Add entities one at a time when debugging
5. **Use Templates**: Create template YAML files for common entity types

## Validation

The YAML loader validates:
- Required components for entity types
- Asset references exist
- Numeric values are in valid ranges
- Component dependencies (e.g., physics requires transform)

## Troubleshooting

### Common Issues

1. **Entity Not Appearing**
   - Check mesh asset name matches index.json
   - Verify transform position is in view
   - Ensure mesh component is present

2. **Physics Not Working**
   - Confirm physics component has mass > 0
   - Check transform component exists
   - Verify 6DOF is enabled for rotation

3. **Controls Not Responding**
   - Ensure control_authority component exists
   - Check thruster_system is configured
   - Verify physics component is present

## File Locations

- Scene files: `assets/scenes/*.yaml`
- Test scenes: `assets/test/scenes/*.yaml`
- Templates: `docs/templates/scene_template.yaml`

## Future Extensions

Planned component types:
- `particle_system`: Visual effects
- `audio_source`: 3D positioned sounds
- `trigger_volume`: Gameplay triggers
- `spawn_point`: Entity spawning locations