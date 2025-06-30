
# Guide: Rendering and Camera Systems

**Status**: Active
**Last Updated**: June 29, 2025
**Owner**: System Analysis

This guide provides a comprehensive overview of the rendering and camera systems in the CGame engine. It covers the rendering pipeline, the camera system, and how they work together to create the final 3D scene.

## 1. Rendering Pipeline

The rendering pipeline is responsible for taking the game world's data and turning it into a 2D image on the screen. It's designed for high performance and flexibility, using the `sokol_gfx` API for cross-platform graphics.

### 1.1. Core Rendering Logic (`src/render_mesh.c`)

The `render_mesh.c` file is the heart of the rendering system. It contains the `render_mesh_system_update` function, which is called every frame. This function is responsible for:

1.  **Iterating through all renderable entities**: It loops through all entities that have both a `Transform` and a `Renderable` component.
2.  **Calculating the MVP matrix**: For each entity, it calculates the Model-View-Projection (MVP) matrix. This matrix transforms the entity's vertices from model space to screen space. It's calculated by multiplying the entity's model matrix by the camera's view-projection matrix.
3.  **Applying materials**: It gets the entity's `Material` component and passes its color to the shader.
4.  **Drawing the mesh**: It calls `sg_draw` to render the entity's mesh.

### 1.2. Shaders (`assets/shaders/`)

The shaders are small programs that run on the GPU and are responsible for the actual drawing. We use a simple shader that takes the MVP matrix and a color as input and outputs the final pixel color.

## 2. Camera System

The camera system is responsible for controlling the viewpoint in the 3D world. It's implemented as a standard ECS system that runs every frame.

### 2.1. Camera Component (`src/core.h`)

The `Camera` component can be attached to any entity to turn it into a camera. This component stores the camera's properties, such as its field of view (FOV), near and far clipping planes, and its view and projection matrices.

To improve performance, the camera's matrices are cached. A `dirty` flag is used to indicate when the matrices need to be recalculated. The matrices are only recalculated when the camera moves or its properties change.

### 2.2. Camera System Logic (`src/render_camera.c`)

The `camera_system_update` function is responsible for updating the active camera. It performs the following steps:

1.  **Gets the active camera**: It gets the active camera entity from the `World`.
2.  **Updates camera position**: It updates the camera's position and orientation based on its behavior (e.g., following a target).
3.  **Updates matrices**: If the camera's `dirty` flag is set, it recalculates the view and projection matrices and caches them in the `Camera` component.

### 2.3. Matrix Math (`src/matrix_math.c`)

The `matrix_math.c` file contains all the necessary functions for 3D matrix and vector math. This includes functions for creating perspective and look-at matrices, as well as functions for matrix multiplication, vector normalization, etc.

## 3. Integration

The rendering and camera systems are tightly integrated. The `render_mesh_system_update` function gets the active camera from the `World` and uses its cached `view_projection_matrix` to calculate the final MVP matrix for each entity. This ensures that all objects are rendered from the correct perspective.
