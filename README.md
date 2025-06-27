# 3D Vector Graphics Space Game

A 3D vector graphics space game written in C using SDL2 for graphics and input handling. Features a spaceship that can move in 3D space with realistic physics, particle effects, and a dynamic star field.

## Features

- **3D Vector Graphics**: Wireframe spaceship rendered with pure vector graphics
- **Full 3D Movement**: Move in all directions with momentum and damping
- **Particle System**: Engine trail particles for visual feedback
- **Dynamic Star Field**: Parallax scrolling stars that react to movement
- **Physics**: Velocity-based movement with space friction
- **HUD**: Velocity indicators and crosshair
- **Smooth Controls**: WASD movement with mouse look support

## Prerequisites

- GCC compiler with C99 support
- SDL2 development library
- Make build system
- Math library (libm)

## Installation

### macOS (using Homebrew)

1. Install dependencies:
   ```bash
   make install-deps
   ```

   Or manually:
   ```bash
   brew install sdl2
   ```

### Linux (Ubuntu/Debian)

```bash
sudo apt-get install libsdl2-dev build-essential
```

### Linux (Fedora)

```bash
sudo dnf install SDL2-devel gcc make
```

## Building and Running

1. Build the game:
   ```bash
   make
   ```

2. Run the game:
   ```bash
   make run
   ```

3. Clean build files:
   ```bash
   make clean
   ```

## Controls

- **W/S**: Throttle up/down
- **A/D**: Strafe left/right  
- **Space/C**: Move up/down
- **Q/E**: Roll left/right
- **R/F**: Pitch up/down
- **T/G**: Yaw left/right
- **Shift**: Afterburner (drains energy)
- **Alt**: Fire laser cannons
- **M**: Toggle mouse capture for flight controls
- **Mouse**: Look around (when mouse captured)
- **Close Window**: Exit the game

## Project Structure

```
cgame/
├── main.c          # X-wing starfighter simulator
├── Makefile        # Build configuration with math library  
├── README.md       # This file
├── build/          # Generated build files
└── .vscode/        # VS Code configuration
```

## Game Architecture

The 3D space game features:

1. **3D Math System**: Vector operations and 3D to 2D projection
2. **Physics Engine**: Velocity-based movement with damping
3. **Particle System**: Engine trail effects
4. **Camera System**: 3D perspective projection
5. **Rendering Pipeline**: Vector graphics rendering
6. **Input System**: Multi-axis movement controls

### Technical Details

- **3D Coordinate System**: Right-handed coordinate system
- **Projection**: Simple perspective projection
- **Frame Rate**: 60 FPS with SDL_Delay timing
- **Particle Effects**: Dynamic engine trail system
- **Star Field**: 200 procedurally placed stars with parallax

## Next Steps

Extend the 3D space game with:

- **Asteroid Field**: 3D obstacles to navigate around
- **Enemy Ships**: AI-controlled opponents
- **Weapons System**: Laser cannons and projectiles
- **Sound Effects**: Engine sounds and weapon fire
- **Textures**: Replace wireframes with textured models
- **Lighting**: Dynamic lighting system
- **HUD Improvements**: Radar, health, energy systems
- **Multiple Levels**: Different space environments

## Troubleshooting

If you encounter build errors:

1. Make sure SDL2 is properly installed
2. Check that your compiler supports C99 standard
3. Verify that SDL2 headers are in your system's include path

For macOS users, if SDL2 headers aren't found, you may need to add the Homebrew include path:
```bash
export CFLAGS="-I/opt/homebrew/include"
export LDFLAGS="-L/opt/homebrew/lib"
```
