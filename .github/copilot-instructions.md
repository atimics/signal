<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# C Game Development Instructions

This is a C game development project using SDL2 for graphics and input handling.

## Project Context

- **Language**: C (C99 standard)
- **Graphics Library**: SDL2
- **Build System**: Make
- **Target Platform**: Cross-platform (primary: macOS)

## Code Style Guidelines

- Use descriptive variable and function names
- Follow consistent indentation (4 spaces)
- Add comments for complex game logic
- Keep functions focused and modular
- Use structs to organize related data (e.g., Player, Game state)

## Game Development Patterns

- Follow the game loop pattern: input → update → render
- Use SDL2 best practices for resource management
- Implement proper cleanup to avoid memory leaks
- Keep game state in organized structures
- Separate concerns (rendering, input, game logic)

## SDL2 Specific Guidelines

- Always check return values from SDL functions
- Use SDL_GetError() for debugging
- Properly initialize and cleanup SDL subsystems
- Use SDL_Delay() or proper timing for frame rate control
- Handle SDL events in a dedicated function

## Performance Considerations

- Minimize dynamic memory allocation in game loop
- Use efficient rendering techniques
- Consider object pooling for frequently created/destroyed objects
- Profile performance-critical sections
