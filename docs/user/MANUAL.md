# CGame User Manual

This document explains how to interact with the CGame Entity-Component-System space simulation engine.

## Overview

CGame is a 3D space simulation showcasing an advanced Entity-Component-System architecture. The current version demonstrates the engine's capabilities with multiple entities, physics simulation, AI behavior, and 3D rendering in a solar system environment.

## Controls

### Basic Navigation
- **Arrow Keys**: Ship movement (Up/Down/Left/Right)
- **Space**: Thrust/Boost
- **ESC**: Exit the game

### Debug and Development
- **F1**: Toggle debug performance overlay
- **F2**: Toggle wireframe rendering (if implemented)
- **F3**: Toggle collision visualization (if implemented)

### Camera Controls (Planned)
- **Mouse**: Look around
- **Mouse Wheel**: Zoom in/out
- **Middle Mouse**: Pan view

## Debug Interface

Press **F1** to access the debug overlay, which shows:

### Performance Metrics
- **FPS**: Current frames per second
- **Frame Time**: Time per frame in milliseconds
- **Entity Count**: Total entities in the world
- **System Timing**: Individual system update times

### Entity Information
- **Transform**: Position, rotation, scale data
- **Physics**: Velocity, acceleration, mass
- **AI**: Current state, update frequency, targets
- **Collision**: Collision sphere radius, layer masks

### System Status
- **Physics System**: 60 FPS - Movement and force simulation
- **Collision System**: 20 FPS - Intersection detection
- **AI System**: 2-10 FPS - Autonomous behavior (distance-based LOD)
- **Render System**: 60 FPS - 3D visual output

## Current Scene: Spaceport

The default scene demonstrates various entity types:

### Player Ship
- **Blue ship**: Player-controlled entity at origin (0, 0, 0)
- **Physics**: Responds to input with realistic momentum
- **Collision**: Sphere collision detection with other entities

### AI Ships
- **Patrol Ships**: Autonomous entities with basic AI behavior
- **Behavior States**: Idle, Patrolling, Reacting
- **LOD Optimization**: Update frequency decreases with distance

### Environment
- **Sun**: Large celestial body for scale reference
- **Scale**: Positioned at (0, 1000, 0) with 10x scale
- **Visual**: Demonstrates large-scale 3D rendering

## Engine Demonstration Features

### Entity-Component-System Architecture
- **Modular Design**: Each entity composed of reusable components
- **Performance**: Efficient processing of hundreds of entities
- **Extensibility**: Easy addition of new behaviors and features

### Data-Driven Configuration
- **Entity Templates**: Ships and objects defined in external files
- **Scene Loading**: Complete environments loaded from configuration
- **Rapid Iteration**: Modify behavior without recompilation

### Scheduled Systems
- **Optimal Performance**: Each system runs at appropriate frequency
- **Scalability**: Supports thousands of entities efficiently
- **LOD Support**: AI complexity scales with importance and distance

## Technical Showcase

### Physics Simulation
- **Newtonian Physics**: Realistic force-based movement
- **Drag Simulation**: Atmospheric-style drag for space flight feel
- **Mass and Inertia**: Different ship types have unique physics properties

### AI Behavior
- **Autonomous Entities**: AI ships operate independently
- **State Machines**: Idle, patrol, and reaction behaviors
- **Decision Making**: AI entities respond to environment and player

### 3D Rendering
- **Mesh Loading**: OBJ file format support with materials
- **Texture Mapping**: PNG texture support for visual detail
- **LOD Rendering**: Distance-based detail optimization
- **Matrix Transforms**: Full 3D positioning and rotation

## Development and Modding

### Asset Creation
- **Mesh Files**: Place OBJ files in `assets/meshes/`
- **Textures**: PNG files alongside mesh files
- **Materials**: MTL files for surface properties

### Entity Configuration
- **Templates**: Define new entity types in `data/templates/entities.txt`
- **Components**: Mix and match Transform, Physics, AI, Renderable components
- **Properties**: Configure mass, collision, AI behavior parameters

### Scene Creation
- **Scene Files**: Create environments in `data/scenes/`
- **Entity Placement**: Position entities with specific configurations
- **Dynamic Loading**: Load different scenes without engine restart

## Troubleshooting

### Performance Issues
- **High Entity Count**: Reduce entities or increase LOD distances
- **System Timing**: Use F1 debug overlay to identify bottlenecks
- **Memory Usage**: Monitor entity count and component allocation

### Visual Problems
- **Missing Textures**: Ensure PNG files are in correct asset directories
- **Black Models**: Check MTL material file configuration
- **Clipping**: Adjust camera near/far planes in render configuration

### Build Issues
- **SDL2 Missing**: Install SDL2 development libraries
- **Compilation Errors**: Ensure GCC with C99 support
- **Asset Loading**: Verify assets directory structure

## Future Features

### Planned Gameplay
- **Faction System**: Dynamic territorial control and conflict
- **Economic Simulation**: Trade routes, resource management
- **Mission Generation**: Procedural quests based on universe state
- **Multiplayer**: Shared universe with multiple players

### Technical Enhancements
- **Neural AI**: LLM-driven entity personalities and dialog
- **Audio System**: 3D positional sound with SDL2_mixer
- **Particle Effects**: Engine exhaust, weapons, environmental effects
- **Advanced Physics**: Collision response, orbital mechanics

### Content Creation
- **Visual Editors**: GUI tools for entity and scene creation
- **Scripting Support**: Lua integration for custom behaviors
- **Workshop Integration**: Community content sharing
- **Modding API**: Comprehensive interfaces for game modification

## Getting Started as a Developer

1. **Explore the Code**: Start with `src/test.c` to see entity creation patterns
2. **Modify Templates**: Edit `data/templates/entities.txt` to create new entity types
3. **Create Scenes**: Design environments in `data/scenes/`
4. **Add Components**: Follow patterns in `src/core.h` for new component types
5. **Implement Systems**: Add new behavior systems in `src/systems.c`

The engine is designed to be educational and extensible, demonstrating modern game architecture techniques while remaining approachable for learning and experimentation.
