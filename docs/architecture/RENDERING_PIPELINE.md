# CGame Rendering Pipeline Architecture

This document provides a comprehensive overview of the CGame engine's rendering pipeline, from frame initialization through final presentation.

## Overview

The CGame rendering pipeline is a sophisticated multi-pass system built on top of Sokol GFX, featuring:
- Layered rendering with offscreen render targets
- Separate 3D scene and UI rendering passes
- Compositor-based final frame assembly
- Thread-safe render pass management

## Pipeline Flow

### 1. Frame Initialization (`render_frame()`)

The rendering process begins when the main loop calls `render_frame()`:

```
render_frame()
├── Begin Sokol pass
├── Update render layers
├── Render 3D scene layers
├── Render UI layer
├── Composite all layers
└── End Sokol pass → Present
```

### 2. Render Layers System

The engine uses a flexible layer system (`render_layers.h`) to organize rendering:

#### Layer Types:
- **Background Layer**: Skybox, environment
- **3D Scene Layer**: Main game world (with depth buffer)
- **Effects Layer**: Particles, post-processing
- **UI Layer**: HUD and menus (no depth buffer)

Each layer maintains its own:
- Offscreen render target
- Optional depth buffer
- Update frequency (static/dynamic/on-demand)
- Visibility state

### 3. 3D Scene Rendering

The 3D scene rendering (`render_3d.h`) follows this sequence:

```
render_3d_begin()
├── Set viewport and projection
├── Update camera matrices
├── Bind 3D pipeline
└── Clear color/depth buffers

render_3d_scene()
├── Frustum culling
├── Sort by material/distance
├── For each visible entity:
│   ├── Bind mesh resources
│   ├── Update uniforms (MVP matrix)
│   └── Draw indexed geometry
└── Render debug overlays

render_3d_end()
└── Restore previous state
```

### 4. UI Rendering

The UI system (`ui_microui.h`) renders to a separate layer:

```
ui_render()
├── Begin UI frame (MicroUI)
├── Process UI modules
├── Generate draw commands
├── Upload vertex data
└── Render with UI pipeline
```

Key features:
- Immediate mode GUI via MicroUI
- Batched draw calls
- No depth testing
- Pixel-perfect rendering

### 5. Compositor Pass

The final compositor (`render_layers.c`) blends all layers:

```
layer_manager_composite()
├── Bind compositor pipeline
├── For each visible layer (sorted):
│   ├── Bind layer texture
│   ├── Apply blend mode
│   └── Render fullscreen quad
└── Present final frame
```

## Render State Management

### Pipeline Manager (`pipeline_manager.h`)

Manages GPU pipeline states:
- Caches compiled pipelines
- Switches between 3D/UI/compositor pipelines
- Handles shader uniform blocks

### Render Pass Guard (`render_pass_guard.h`)

Ensures thread-safe rendering:
- Validates pass state before operations
- Prevents nested/invalid pass sequences
- Provides debug assertions in development

## Resource Management

### GPU Resources (`gpu_resources.h`)

Centralized GPU resource tracking:
- Vertex/index buffer allocation
- Texture loading and binding
- Shader compilation
- Resource lifetime management

### Mesh Rendering (`render_mesh.h`)

Optimized mesh handling:
- Vertex format: position, normal, UV, color
- Indexed rendering for efficiency
- Material-based batching
- LOD support (future)

## Performance Considerations

1. **Frustum Culling**: Only visible entities are rendered
2. **Draw Call Batching**: Entities sorted by material to minimize state changes
3. **Layer Caching**: Static layers only re-render when marked dirty
4. **GPU Buffer Management**: Dynamic buffers for frequently updated data

## Debug Features

The rendering system includes extensive debug capabilities:
- Wireframe mode toggle
- Performance counters (draw calls, vertices)
- Layer visualization
- GPU state validation

## Integration Points

### Adding a New Render Layer

```c
RenderLayer* layer = layer_manager_add_layer(
    manager,
    "my_layer",
    width, height,
    true,  // needs depth buffer
    LAYER_UPDATE_DYNAMIC
);
layer_set_order(layer, 50);  // rendering order
```

### Custom Rendering Pass

```c
// In your system's render function
if (render_pass_begin()) {
    // Your custom rendering code
    render_pass_end();
}
```

## Future Enhancements

Planned improvements to the rendering pipeline:
- Shadow mapping support
- Post-processing effects framework
- Instanced rendering for vegetation/debris
- Temporal upsampling (FSR/DLSS style)
- HDR rendering with tone mapping

## See Also

- [Render Layers API](../api/render_layers.md)
- [Pipeline Manager API](../api/pipeline_manager.md)
- [GPU Resources Guide](../guides/gpu_resources.md)