# Scripted Flight Debug Summary

## Debug Output Added

I've added comprehensive debugging to trace why scripted flight isn't moving the ship. The debug output covers:

### 1. **Scripted Flight System** (`scripted_flight.c`)
- **System Update**: Logs every second showing active scripted flight count and entity status
- **Entity Update**: Logs when update_scripted_entity is called for each entity
- **Flight State**: Enhanced debug showing:
  - Current waypoint and distance to target
  - Ship position and target position
  - Thrust commands (normalized -1 to 1)
  - Linear forces being calculated
  - Current vs desired velocity
  - Thruster state and max forces
  - Current thrust values in thruster system

### 2. **Control System** (`control.c`)
- **Entity Processing**: Logs which entities are being processed for control
- **Player Detection**: Fixed the player entity detection logic to check `entity->id == g_player_entity`
- **Input State**: Logs when no player input is detected (allowing scripted flight to control)
- **Non-Player Entities**: Removed the code that was clearing thrust for non-player entities

### 3. **Thruster System** (`thrusters.c`)
- **Command Reception**: Logs when thrust commands are received
- **Command Changes**: Logs significant thrust changes
- **System Update**: Logs which entities are being processed
- **Force Application**: Enhanced logging showing:
  - Entity ID receiving thrust
  - Local and world space forces
  - Ship position and velocity

### 4. **Physics System** (`physics.c`)
- **Force Addition**: Logs when forces are added to physics accumulator
- **Force State**: Shows current accumulator state

### 5. **Flight Test Scene** (`flight_test_scene.c`)
- **Key Press Handler**: Enhanced '1' key press debug showing:
  - Player ship ID
  - Scripted flight component pointer
  - Flight active state
  - Circuit path creation
  - Control system state

## Key Issues Found and Fixed

1. **Control System Logic Bug**: The control system was checking `control->controlled_by == g_player_entity`, but `controlled_by` is set to the ship's own ID. Fixed by checking `entity->id == g_player_entity`.

2. **Thrust Clearing Bug**: The control system was clearing thrust commands for all non-player entities, preventing scripted flight from controlling the player ship.

## How to Test

1. Build and run: `make clean && make`
2. Run with test flight: `./build/signal --test-flight`
3. Press '1' to activate circuit flight
4. Watch the debug output for:
   - "=== '1' KEY PRESSED - CIRCUIT FLIGHT DEBUG ==="
   - "=== SCRIPTED FLIGHT DEBUG ==="
   - "THRUST CHANGED:" messages
   - "THRUST APPLIED TO ENTITY" messages

## What to Look For

The debug output will show:
- Whether scripted flight is active for the entity
- If thrust commands are being calculated and set
- If those commands are reaching the thruster system
- If forces are being applied in physics
- Whether the control system is interfering

This comprehensive debugging should reveal exactly where in the chain the issue occurs.