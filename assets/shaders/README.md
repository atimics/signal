# Shader Directory

This directory contains the shader files used by the CGame engine's Sokol-based rendering system.

## Structure

Shaders are organized by functionality and platform:

```
assets/shaders/
├── basic_3d.vert.metal    # Metal vertex shader (macOS)
├── basic_3d.frag.metal    # Metal fragment shader (macOS)
├── basic_3d.vert.glsl     # OpenGL vertex shader (Linux/Windows)
└── basic_3d.frag.glsl     # OpenGL fragment shader (Linux/Windows)
```

## Naming Convention

Shader files follow this naming pattern:
`<shader_name>.<stage>.<api>`

Where:
- `shader_name`: Descriptive name (e.g., "basic_3d", "pbr", "shadow")
- `stage`: Shader stage ("vert" for vertex, "frag" for fragment, "comp" for compute)
- `api`: Graphics API ("metal" for Metal, "glsl" for OpenGL)

## Usage

Shaders are loaded automatically by the rendering system based on the current platform:
- macOS: Uses `.metal` shaders
- Linux/Windows: Uses `.glsl` shaders

The engine automatically selects the correct shader files using the `get_shader_path()` function in the assets system.

## Adding New Shaders

1. Create both Metal and GLSL versions of your shader
2. Place them in this directory following the naming convention
3. Update the rendering code to load your shaders using `load_shader_source()`

## Current Shaders

### basic_3d
Basic 3D rendering with:
- MVP transformation
- Phong-style lighting
- Single diffuse texture
- Vertex attributes: position, normal, texture coordinates
