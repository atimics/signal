# R01: A Guide to Resilient Mesh Architecture and Rendering Pipelines

**ID**: `R01`  
**Author**: Gemini Engineering Analyst  
**Version**: 2.0 (Revised)  
**Status**: Active Reference

## 1. Overview

This document provides a deep, technical analysis of the architectural requirements for a robust and scalable mesh rendering pipeline in a modern game engine. It moves beyond high-level theory to offer concrete C-based implementation patterns, data structure designs, and data flow diagrams. This guide serves as the primary technical reference for **Sprint 10.5** and all future work on the rendering system.

## 2. The Ideal Mesh Data Flow

The core of a resilient system is a predictable, one-way data flow from source asset to GPU resource. The current system fails because this flow is broken and convoluted. The ideal flow should be:

```
[Source Asset (.obj, .fbx)]
       |
       v
[Asset Compiler (asset_compiler.py)] --> [Runtime Data (.cobj)]
       |
       v
[Asset Index (index.json)]
       |
       v
[Runtime Asset Loader (assets.c)]
       |
       v
[CPU Memory (Mesh struct)]
       |
       v
[GPU Resource Creation (Sokol GFX)] --> [sg_buffer, sg_image]
       |
       v
[Modular Renderer (render_mesh.c)] --> [Rendered Frame]
```

**Key Principle**: Each stage is responsible for a single, well-defined transformation. The runtime loader should **never** have to guess or reverse-engineer file paths.

## 3. Core Component Deep Dive

### 3.1. The Asset Index (`index.json`)

This file is the **single source of truth** for the runtime loader. It decouples the engine from the physical layout of the asset directory.

**Recommended Structure**:
```json
{
  "version": 1,
  "assets": {
    "wedge_ship": {
      "type": "static_mesh",
      "path": "props/wedge_ship/geometry.cobj",
      "material": "props/wedge_ship/material.mtl"
    },
    "control_tower": {
      "type": "static_mesh",
      "path": "props/control_tower/geometry.cobj",
      "material": "props/control_tower/material.mtl"
    }
  }
}
```
*   **`wedge_ship`**: A logical, engine-facing asset name.
*   **`path`**: The direct, relative path to the runtime data from the `assets/` root.

### 3.2. The Runtime Asset Loader (`assets.c`)

This module's only job is to read the asset index and load the data specified.

#### **Path Resolution (Task 1.1)**
The current pathing logic is the primary bug. It should be replaced with a simple, robust implementation.

**Incorrect (Current) Logic**:
```c
// Tries to build an absolute path, then make it relative again.
// This is complex and error-prone.
char absolute_path[512];
snprintf(absolute_path, ...);
char relative_path[512];
// ... complex logic to create relative_path ...
load_compiled_mesh(registry, relative_path, ...);
```

**Correct (Proposed) Logic**:
```c
// In the function that reads index.json:
const char* relative_path_from_index = "props/wedge_ship/geometry.cobj";
char full_path[512];

// The loader's ONLY job is to combine the asset root and the path from the index.
snprintf(full_path, sizeof(full_path), "%s/meshes/%s", registry->asset_root, relative_path_from_index);

// Pass the full, unambiguous path to the parser.
parse_and_upload_mesh(full_path, "wedge_ship");
```

#### **Memory Management (Task 1.2)**
The use of fixed-size static arrays in `parse_obj_file` is a critical stability risk.

**Incorrect (Current) `parse_obj_file` Structure**:
```c
bool parse_obj_file(...) {
    // DANGEROUS: Fixed-size buffers on the stack.
    Vertex final_vertices[8192];
    int final_indices[16384];
    // ... parsing logic ...
}
```

**Correct (Proposed) `parse_obj_file` Structure**:
```c
bool parse_obj_file(const char* filepath, Mesh* out_mesh) {
    FILE* file = fopen(filepath, "r");
    // ... error handling ...

    // 1. First Pass: Count elements
    int num_vertices = 0, num_indices = 0;
    // ... loop through file, incrementing counts ...

    // 2. Allocate exact memory needed
    out_mesh->vertices = malloc(num_vertices * sizeof(Vertex));
    out_mesh->indices = malloc(num_indices * sizeof(int));
    // ... error handling for malloc ...

    // 3. Second Pass: Populate allocated memory
    rewind(file);
    // ... loop through file again, filling the buffers ...
    
    fclose(file);
    return true;
}
```

### 3.3. The Modular Renderer (`render_mesh.c`)

This module should be entirely self-contained. It should not rely on global state from `render_3d.c`.

**Proposed Header (`render_mesh.h`)**:
```c
#pragma once
#include "core.h" // For Entity, Transform, Renderable
#include "sokol_gfx.h"

// A self-contained renderer module
typedef struct {
    sg_pipeline pipeline;
    sg_shader shader;
    // Add other resources like a default sampler if needed
} MeshRenderer;

// Public API
bool mesh_renderer_init(MeshRenderer* renderer);
void mesh_renderer_shutdown(MeshRenderer* renderer);

// Renders a single entity
void mesh_renderer_draw(MeshRenderer* renderer, struct Transform* transform, 
                        struct Renderable* renderable, const float* view_projection_matrix);
```

**Implementation (`render_mesh.c`)**:
The `mesh_renderer_init` function will be responsible for creating its own `sg_pipeline` and `sg_shader`. The main render loop in `render_3d.c` will simply call `mesh_renderer_draw` for each renderable entity, delegating the actual drawing work. This enforces **separation of concerns**.

## 4. GPU Resource Management and Validation

A primary cause of failure is creating GPU resources from invalid or incomplete CPU-side data.

**Validation-First Approach (Task 1.3)**:
The `sg_make_buffer` call should be the very last step, guarded by comprehensive checks.

```c
// In assets.c, after a mesh has been parsed into memory
bool upload_mesh_to_gpu(Mesh* mesh) {
    // 1. Validate CPU-side data
    if (!mesh || !mesh->vertices || !mesh->indices) return false;
    if (mesh->vertex_count == 0 || mesh->index_count == 0) return false;

    // 2. Validate buffer sizes BEFORE creating them
    const size_t vb_size = mesh->vertex_count * sizeof(Vertex);
    const size_t ib_size = mesh->index_count * sizeof(int);
    if (vb_size == 0 || ib_size == 0) {
        printf("ERROR: Mesh '%s' would create a zero-sized GPU buffer.\n", mesh->name);
        return false;
    }

    // 3. Create the GPU resources
    sg_buffer_desc vbuf_desc = { .data = { .ptr = mesh->vertices, .size = vb_size }, ... };
    mesh->sg_vertex_buffer = sg_make_buffer(&vbuf_desc);

    sg_buffer_desc ibuf_desc = { .data = { .ptr = mesh->indices, .size = ib_size }, ... };
    mesh->sg_index_buffer = sg_make_buffer(&ibuf_desc);

    // 4. Post-creation validation
    if (sg_query_buffer_state(mesh->sg_vertex_buffer) != SG_RESOURCESTATE_VALID ||
        sg_query_buffer_state(mesh->sg_index_buffer) != SG_RESOURCESTATE_VALID) {
        printf("ERROR: Sokol failed to create buffers for mesh '%s'.\n", mesh->name);
        // Destroy any partially created resources
        sg_destroy_buffer(mesh->sg_vertex_buffer);
        sg_destroy_buffer(mesh->sg_index_buffer);
        return false;
    }

    // 5. Free CPU-side memory after successful upload
    // This is a critical optimization to reduce memory footprint.
    free(mesh->vertices);
    mesh->vertices = NULL;
    free(mesh->indices);
    mesh->indices = NULL;

    return true;
}
```
**Key Optimization**: Once data is on the GPU, the CPU-side copy can and should be freed to conserve memory, especially for large meshes.

## 5. Conclusion and Recommendations

The engine's current rendering failure is a classic symptom of a brittle, tightly-coupled pipeline. By implementing the recommendations in this guide, the system will be transformed into a robust, data-driven, and modular architecture.

*   **Priority 1**: Fix the data flow by implementing the `index.json`-driven loading strategy.
*   **Priority 2**: Harden the `parse_obj_file` function with dynamic memory allocation.
*   **Priority 3**: Implement the modular `MeshRenderer` and delegate all drawing from the main loop.
*   **Priority 4**: Implement strict pre- and post-creation validation for all GPU resources.

Adhering to these principles will not only solve the immediate crisis but will also provide a stable and scalable foundation for future rendering features like PBR, instancing, and skeletal animation.