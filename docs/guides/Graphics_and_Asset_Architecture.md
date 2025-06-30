# CGame Engine: Graphics and Asset Architecture

**Status**: Authoritative | **Last Updated**: June 30, 2025
**Owner**: Gemini

## 1. Core Philosophy: A Data-Driven, Decoupled Pipeline

The CGame engine's graphics and asset architecture is built on two core principles: **Data-Oriented Design** and **Strict Decoupling**. Our goal is to create a high-performance rendering system that is easy to maintain, extend, and debug.

1.  **Data-Oriented Design**: We prioritize the data and its transformations. The pipeline is a one-way flow of data from source art to the GPU. Components are simple data containers, and systems perform transformations on that data.
2.  **Strict Decoupling**: High-level engine code should *never* depend on the implementation details of low-level libraries. This is achieved through a robust asset pipeline and a clean graphics abstraction layer.

This document serves as the single source of truth for this architecture.

## 2. The End-to-End Data Flow

The entire rendering process can be understood as a predictable, multi-stage data pipeline:

```
[1. Source Assets] -> [2. Asset Compiler] -> [3. Asset Index] -> [4. Runtime Loader] -> [5. GPU Abstraction] -> [6. Renderer]
```

Each stage has a single, well-defined responsibility.

### Stage 1: Source Assets (`/assets`)

*   **Description**: Artist-created, source-control-friendly files.
*   **Formats**: `.obj`, `.fbx` (meshes); `.png`, `.svg` (textures).
*   **Responsibility**: To be the human-editable source for all game assets.

### Stage 2: The Asset Compiler (`/tools/asset_compiler.py`)

*   **Description**: A Python-based tool that processes source assets into an optimized, engine-ready format.
*   **Responsibilities**:
    *   **Mesh Optimization**: Triangulating, optimizing vertex cache order, and generating normals and tangents.
    *   **UV Unwrapping**: Using libraries like `xatlas` to generate clean UV coordinates for texturing.
    *   **Texture Conversion**: Converting source images into an optimized format (e.g., `.ktx`, `.dds`) or our simple `.ctex`.
    *   **Material Generation**: Parsing source material data and generating a standardized `.mtl` file with PBR properties.
*   **Output**: Engine-consumable binary files (e.g., `.cobj`) and material definitions.

### Stage 3: The Asset Index (`/build/assets/index.json`)

*   **Description**: The **single source of truth** for the runtime engine. This file maps logical asset names to their physical, compiled data files.
*   **Structure**:
    ```json
    {
      "version": 1,
      "assets": {
        "wedge_ship": {
          "type": "static_mesh",
          "path": "props/wedge_ship/geometry.cobj",
          "material": "props/wedge_ship/material.mtl"
        }
      }
    }
    ```
*   **Key Principle**: The engine runtime does **not** scan directories. It only reads this index. This decouples the engine from the physical file layout, making loading fast and deterministic.

### Stage 4: The Runtime Asset Loader (`/src/assets.c`)

*   **Description**: The C module responsible for loading the asset index and bringing compiled assets into CPU memory.
*   **Responsibilities**:
    *   Parse `index.json` on startup.
    *   On request (e.g., `load_mesh("wedge_ship")`), look up the asset in the index.
    *   Construct the full, unambiguous file path by combining the asset root path with the relative path from the index.
    *   Read the compiled asset file (e.g., `.cobj`) into a CPU-side `Mesh` struct.
    *   **Memory Management**: The loader must correctly allocate and free memory for mesh data. It uses a two-pass approach: first, count vertices/indices to determine size, then allocate the exact memory required.

### Stage 5: The Graphics Abstraction Layer (`/src/gpu_resources.c`)

*   **Description**: The critical layer that decouples the main engine from the underlying graphics API (Sokol GFX).
*   **Core Pattern**: We use the **Opaque Pointer (PIMPL) Idiom** to achieve this decoupling.
*   **How it Works**:
    1.  **Public Header (`assets.h`)**: Declares a *pointer* to an incomplete struct (e.g., `struct GpuResources*`). The header has **no knowledge** of Sokol types.
        ```c
        // In a high-level header like assets.h or core.h
        struct GpuResources; // Forward-declared, incomplete type

        typedef struct {
            // The component only stores a pointer. Its size is known.
            struct GpuResources* gpu;
            // ... other non-GPU data
        } Renderable;
        ```
    2.  **Private Implementation (`gpu_resources.c`)**: This `.c` file is the *only* place that includes `sokol_gfx.h` and provides the full definition of the struct.
        ```c
        // In a .c file
        #include "sokol_gfx.h"

        // The full definition is hidden from the rest of the engine.
        struct GpuResources {
            sg_buffer vbuf;
            sg_buffer ibuf;
            sg_image texture;
        };
        ```
*   **Benefit**: This is the cornerstone of a maintainable rendering architecture. If we ever change or upgrade the graphics library, the changes are confined to the `.c` files, and the public headers remain stable, preventing cascading recompiles.

### Stage 6: The Modular Renderer (`/src/render_mesh.c`)

*   **Description**: A self-contained module responsible for taking a `Renderable` component and issuing draw calls to the GPU.
*   **Responsibilities**:
    *   Initialize its own shaders and pipelines (`sg_shader`, `sg_pipeline`).
    *   Provide a simple drawing function (e.g., `mesh_renderer_draw()`).
    *   The main render loop iterates through entities and calls this function for each one, delegating the actual drawing work.
*   **Key Principle**: **Separation of Concerns**. The main renderer orchestrates, while modular renderers execute.

## 3. Material and Rendering System: PBR

To achieve modern, realistic visuals, the engine uses a Physically-Based Rendering (PBR) workflow.

### Material System

*   **Philosophy**: Our material system is data-driven and based on the metallic-roughness workflow.
*   **Core Properties**:
    *   **Albedo/BaseColor**: The base color of the material (texture or factor).
    *   **Metallic**: A value from 0 (dielectric) to 1 (metal).
    *   **Roughness**: Describes the microsurface of the material, affecting the blurriness of reflections.
    *   **Normal Map**: Provides high-frequency surface detail.
    *   **Ambient Occlusion (AO)**: Adds contact shadows and enhances detail.
*   **Asset Pipeline**: The `asset_compiler.py` is responsible for generating these texture maps and material files from source assets.

### Rendering Implementation

*   **Lighting**: All lighting calculations are performed per-pixel in the fragment shader for maximum quality.
*   **Shaders**: We use a dedicated PBR shader (`pbr.frag.glsl`/`.metal`) that implements the Cook-Torrance BRDF for realistic light interaction.
*   **Uniforms**: A `UniformBuffer` struct is used to pass PBR material properties, camera data, and light information to the GPU.
*   **Texture Binding**: The renderer supports binding multiple textures per draw call to supply the PBR shader with all necessary data (Albedo, Normal, Metallic/Roughness, etc.).

## 4. GPU Resource Management

*   **Validation First**: We employ a strict, validation-first approach to creating GPU resources.
    1.  **Validate CPU Data**: Ensure the `Mesh` struct loaded from disk is valid.
    2.  **Validate Buffer Sizes**: Check that the calculated vertex and index buffer sizes are non-zero before proceeding.
    3.  **Create GPU Resources**: Call `sg_make_buffer()` and `sg_make_image()`.
    4.  **Post-Creation Validation**: Use `sg_query_*_state()` to confirm that the resource was created successfully on the GPU.
*   **Memory Optimization**: After a mesh's data has been successfully uploaded to the GPU, the CPU-side copy of its vertex and index data is **freed**. This significantly reduces the engine's memory footprint, especially with large, high-poly meshes.

This consolidated guide provides the architectural foundation for all current and future graphics and asset-related development in the CGame engine.
