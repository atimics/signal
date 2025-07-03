# Unified Ship Control System

## Overview

The CGGame engine now features a unified ship control system that provides consistent configuration and behavior across all scenes. This system eliminates the fragmentation and duplication that occurred during experimentation and provides a clean API for configuring ships with different flight characteristics.

## Key Components

### 1. Ship Configuration Presets

The system provides four predefined ship configurations:

```c
typedef enum {
    SHIP_CONFIG_FIGHTER,      // Fast, agile fighter
    SHIP_CONFIG_RACER,        // Canyon racing optimized
    SHIP_CONFIG_FREIGHTER,    // Heavy cargo ship
    SHIP_CONFIG_RC_ROCKET     // RC model rocket (test)
} ShipConfigPreset;
```

### 2. Unified Configuration Function

```c
void control_configure_ship(struct World* world, EntityID ship_id, ShipConfigPreset preset);
```

This function automatically configures all ship components:
- **Physics**: Mass, drag, moment of inertia, 6DOF settings
- **Thrusters**: Max forces, torques, response time, efficiency
- **Control Authority**: Sensitivity, assist levels, control mode

### 3. Component Integration

The control system integrates three main components:

1. **Physics Component** - Handles 6DOF physics simulation
2. **ThrusterSystem Component** - Manages propulsion and force application
3. **ControlAuthority Component** - Processes input and generates commands

## Ship Presets

### SHIP_CONFIG_FIGHTER
- **Mass**: 50kg
- **Characteristics**: High agility, quick response
- **Use Case**: Combat, aerobatics
- **Control**: Assisted mode with light stability

### SHIP_CONFIG_RACER
- **Mass**: 80kg
- **Characteristics**: High drag for arcade feel, powerful thrust
- **Use Case**: Canyon racing, high-speed navigation
- **Control**: Assisted mode with moderate stability

### SHIP_CONFIG_FREIGHTER
- **Mass**: 500kg
- **Characteristics**: Heavy, stable, slower response
- **Use Case**: Cargo transport, steady flight
- **Control**: Assisted mode with high stability

### SHIP_CONFIG_RC_ROCKET
- **Mass**: 10kg
- **Characteristics**: Very light, minimal drag, quick response
- **Use Case**: Testing, demonstration
- **Control**: Manual mode, no assists

## Usage Example

```c
// In scene initialization
void my_scene_enter(struct World* world, SceneStateManager* state) {
    // Find or create player ship
    EntityID player_ship = find_player_ship(world);
    
    // Ensure ship has required components
    if (!entity_has_component(world, player_ship, COMPONENT_THRUSTER_SYSTEM)) {
        entity_add_component(world, player_ship, COMPONENT_THRUSTER_SYSTEM);
    }
    if (!entity_has_component(world, player_ship, COMPONENT_CONTROL_AUTHORITY)) {
        entity_add_component(world, player_ship, COMPONENT_CONTROL_AUTHORITY);
    }
    
    // Configure ship with preset
    control_configure_ship(world, player_ship, SHIP_CONFIG_RACER);
    
    // Set as player entity
    control_set_player_entity(world, player_ship);
    
    // Initialize input system
    input_init();
}

// In scene update
void my_scene_update(struct World* world, SceneStateManager* state, float delta_time) {
    // Update input system
    input_update();
    
    // Control system automatically processes input and applies thrust
}
```

## Control Flow

```
Input Device → Input System → Control Authority → Thruster System → Physics
     ↓              ↓                ↓                   ↓             ↓
  Gamepad      InputState    Control Commands    Force/Torque    Movement
```

## Key Features

1. **Automatic Component Configuration**: All required components are configured with appropriate values
2. **Self-Controlled Entities**: Ships are automatically set to control themselves (`controlled_by = ship_id`)
3. **6DOF Physics**: Always enabled for all ship configurations
4. **Consistent Behavior**: Same control scheme works across all scenes
5. **Easy Customization**: Scenes can still adjust specific values after preset application

## Migration Guide

### From Old System
```c
// Old way - manual configuration
physics->mass = 80.0f;
physics->drag_linear = 0.99f;
thrusters->max_linear_force = (Vector3){400, 400, 1200};
control->controlled_by = ship_id;
control->control_sensitivity = 2.0f;
// ... many more manual settings
```

### To New System
```c
// New way - unified configuration
control_configure_ship(world, ship_id, SHIP_CONFIG_RACER);
```

## Best Practices

1. **Always Initialize Input**: Call `input_init()` and `input_update()` in your scene
2. **Set Player Entity**: Call `control_set_player_entity()` after configuration
3. **Check Components**: Ensure entities have required components before configuration
4. **Scene-Specific Tweaks**: Apply any scene-specific adjustments after preset configuration

## Future Enhancements

- Additional presets for specialized ship types
- Dynamic preset switching during gameplay
- Per-component configuration overrides
- Integration with ODE physics engine for enhanced realism