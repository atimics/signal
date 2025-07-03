# Scripted Flight System Guide

The Scripted Flight System allows entities to follow predefined flight paths autonomously, perfect for cinematic sequences, AI behaviors, and racing demonstrations.

## Overview

The scripted flight system provides:
- Waypoint-based path following
- Smooth velocity and direction control
- Multiple predefined path patterns
- Manual override capability
- Performance-optimized updates

## Basic Usage

### Initializing Scripted Flight

```c
// Create a scripted flight component for an entity
ScriptedFlight* flight = scripted_flight_create_component(entity_id);

// Create a flight path (several patterns available)
FlightPath* path = scripted_flight_create_circuit_path();

// Start the scripted flight
scripted_flight_start(flight, path);
```

### Available Flight Patterns

#### 1. Circuit Path
A rectangular flight pattern, perfect for demonstrations.

```c
FlightPath* circuit = scripted_flight_create_circuit_path();
```

**Pattern**: Rectangle at 50m altitude
- Start: (0, 50, 0)
- Waypoints: Forms a 400x200 unit rectangle
- Speed: Variable (80-120 units/sec)
- Looping: Yes

#### 2. Figure-Eight Path
An infinity symbol pattern for dynamic demonstrations.

```c
FlightPath* figure8 = scripted_flight_create_figure_eight_path();
```

**Pattern**: Figure-8 at varying altitudes
- Center: (0, 60, 0)
- Size: 300 unit loops
- Speed: 60-100 units/sec
- Looping: Yes

#### 3. Landing Approach
A descending approach pattern for landing sequences.

```c
FlightPath* landing = scripted_flight_create_landing_approach();
```

**Pattern**: Gradual descent to ground
- Start: (-500, 100, 0)
- End: (0, 5, 0)
- Speed: Decreasing (100→20 units/sec)
- Looping: No

## Flight Control

### Starting a Flight
```c
scripted_flight_start(flight, path);
```

### Stopping a Flight
```c
scripted_flight_stop(flight);
```

### Pausing/Resuming
```c
scripted_flight_pause(flight);    // Temporarily stop
scripted_flight_resume(flight);   // Continue from pause
```

## Integration with Game Systems

### Required Components
Entities using scripted flight must have:
- `Transform` - Position and orientation
- `Physics` - Velocity and forces
- `ThrusterSystem` - Propulsion control

### Input Controls (Flight Test Scene)
- **1** - Start/stop circuit flight
- **2** - Start/stop figure-8 flight  
- **3** - Start/stop landing approach
- **TAB** - Cycle camera modes
- **ESC** - Return to menu

## Creating Custom Paths

### Path Structure
```c
typedef struct {
    Vector3 position;      // World position
    float target_speed;    // Desired speed at waypoint
    float tolerance;       // Distance to consider "reached"
} Waypoint;

typedef struct {
    Waypoint waypoints[MAX_WAYPOINTS];
    int waypoint_count;
    bool loop;            // Return to start after last waypoint
} FlightPath;
```

### Example: Custom Path
```c
FlightPath* create_custom_path() {
    FlightPath* path = malloc(sizeof(FlightPath));
    
    // Define waypoints
    path->waypoints[0] = (Waypoint){
        .position = {0, 50, 0},
        .target_speed = 50.0f,
        .tolerance = 10.0f
    };
    
    path->waypoints[1] = (Waypoint){
        .position = {100, 75, 50},
        .target_speed = 80.0f,
        .tolerance = 10.0f
    };
    
    path->waypoint_count = 2;
    path->loop = true;
    
    return path;
}
```

## Physics Integration

The system controls entities through the thruster system:

1. **Position Control**: Calculates desired velocity to reach waypoint
2. **Velocity Control**: Determines required acceleration
3. **Force Application**: Converts to thruster commands (-1 to 1)
4. **Physics Update**: Thruster system applies forces

### Control Parameters
- **Acceleration Limit**: 10 m/s² (prevents jerky movement)
- **Force Scaling**: Based on entity mass
- **Direction Smoothing**: Gradual rotation alignment

## Performance Considerations

### Update Frequency
- System updates every frame
- Debug output every 30 frames (0.5 seconds at 60 FPS)
- Minimal CPU overhead per entity

### Memory Usage
- Fixed array for flight components (MAX_SCRIPTED_FLIGHTS = 64)
- Each component: ~1KB
- Paths allocated separately

## Debugging

### Console Output
The system provides detailed debug information:

```
🛩️  Started scripted flight with 8 waypoints (loop: yes)
📍 Entity 1: waypoint 3/8, dist=45.2
📍 Position: [120.5,50.0,80.3] → Target: [200.0,50.0,100.0]
🚀 Thrust CMD: [0.68,0.00,0.25] (normalized -1 to 1)
📊 Speed: current=78.5, desired=80.0
```

### Common Issues

#### Entity Not Moving
- Verify all required components exist
- Check thruster system is enabled
- Ensure physics mass > 0
- Confirm waypoint positions are reachable

#### Erratic Movement
- Reduce acceleration limit for smoother motion
- Increase waypoint tolerance
- Check for conflicting manual input

#### Path Not Completing
- Verify waypoint count is correct
- Check tolerance values aren't too small
- Ensure loop flag matches intention

## Advanced Features

### Dynamic Path Modification
```c
// Change target speed for smoother sections
flight->path.waypoints[2].target_speed = 40.0f;

// Adjust tolerance for precision
flight->path.waypoints[5].tolerance = 5.0f;
```

### State Queries
```c
bool is_active = scripted_flight_is_active(flight);
int current_waypoint = scripted_flight_get_current_waypoint(flight);
float progress = scripted_flight_get_progress(flight);
```

### Manual Override
The system automatically disables when manual input is detected, allowing seamless transition between scripted and player control.

## Future Enhancements

Planned improvements:
- Spline interpolation between waypoints
- Dynamic obstacle avoidance
- Formation flying support
- Path recording from manual flight
- YAML path definitions

## Example Implementation

See `src/scripts/flight_test_scene.c` for a complete implementation example showing:
- Component setup
- Path selection
- Input handling
- Camera integration

The flight test scene demonstrates all three path types and provides a testing ground for the scripted flight system.