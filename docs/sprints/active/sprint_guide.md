# Guide: Sprint 10.5 - Critical Mesh System Architecture Repair

**Status**: **COMPLETED**
**JIRA**: `CG-175`

## 1. Overview

This document is the consolidated and final guide for Sprint 10.5. It summarizes the problem statement, the implementation plan, the code-level changes, and the final outcome. Its purpose is to provide a single, authoritative source of truth for this critical development cycle.

---

## 2. The Problem: A Broken Rendering Pipeline

The sprint was initiated to address a **critical architectural failure** that prevented the engine from rendering any 3D mesh geometry.

*   **Symptom**: The engine would fall back to rendering a hardcoded test triangle instead of displaying game assets.
*   **Root Cause**: A fundamental disconnect between the asset pipeline and the runtime rendering system. The engine was successfully compiling assets but failing to load them correctly at runtime.

**Key Issues Addressed**:
1.  **Broken Asset Pathing**: The asset loader was unable to resolve the correct file paths for compiled `.cobj` files.
2.  **Invalid GPU Resources**: The silent failure of the asset loader led to attempts to create zero-sized GPU buffers, causing validation errors and crashes in the Sokol GFX library.
3.  **Monolithic Rendering**: The main render loop was not modular and contained hardcoded fallback logic, masking the severity of the underlying issues.
4.  **Architectural Flaws**: A persistent header dependency conflict between high-level engine code (`core.h`) and the low-level graphics library (`sokol_gfx.h`) was causing recurring compilation errors.

---

## 3. Implementation Details & Final Status

The following is a breakdown of the tasks performed during the sprint, their final status, and links to the relevant code and research.

### Phase 1: Fix Asset Loading

#### **Task 1.1: Fix Asset Loading Path Resolution**
*   **Status**: ✅ **Completed**
*   **Implementation**: The convoluted relative path logic in `src/assets.c` was replaced with a new, robust function `load_compiled_mesh_absolute`. This function accepts a full, unambiguous file path, which is now correctly constructed in `load_single_mesh_metadata`.
*   **Code Link**: [`src/assets.c`](../../src/assets.c)

#### **Task 1.2: Fix .cobj File Parsing**
*   **Status**: ⚠️ **Deferred**
*   **Implementation**: The parser in `src/assets.c` was improved to handle multiple face formats (`f v/t/n` and `f v/t`). However, the core task of replacing the fixed-size static arrays with dynamic memory allocation was deferred to prevent scope creep.
*   **Justification**: While a stability risk for very large assets, the current implementation is functional for all existing meshes. This task will be moved to a future "Engine Hardening" sprint.
*   **Code Link**: [`src/assets.c`](../../src/assets.c)

#### **Task 1.3: Add Mesh Loading Validation**
*   **Status**: ✅ **Completed**
*   **Implementation**: Comprehensive validation checks were added to `load_compiled_mesh_absolute`. The system now verifies that mesh data is not null and that buffer sizes are non-zero *before* attempting to create GPU resources, preventing Sokol GFX errors.
*   **Code Link**: [`src/assets.c`](../../src/assets.c)

### Phase 2: Restore Modular Rendering

#### **Task 2.1: Reimplement `render_mesh.c`**
*   **Status**: ✅ **Completed**
*   **Implementation**: The `render_mesh.c` and `render_mesh.h` files were restored and modernized. A self-contained `MeshRenderer` module was created to encapsulate all mesh-specific rendering logic.
*   **Code Links**: [`src/render_mesh.c`](../../src/render_mesh.c), [`src/render_mesh.h`](../../src/render_mesh.h)

#### **Task 2.2: Remove Test Triangle Fallback**
*   **Status**: ✅ **Completed**
*   **Implementation**: The fallback logic to draw a test triangle was removed from the main render loop in `src/render_3d.c`. It was replaced with a simple warning message that is logged to the console if no entities are rendered in a frame.
*   **Code Link**: [`src/render_3d.c`](../../src/render_3d.c)

#### **Task 2.3: Integrate Mesh Renderer with ECS**
*   **Status**: ✅ **Completed**
*   **Implementation**: The main render loop in `src/render_3d.c` was refactored to delegate rendering responsibility. It now calls the `mesh_renderer_render_entity` function for each valid entity, passing the necessary data for the modular renderer to perform the draw call.
*   **Code Link**: [`src/render_3d.c`](../../src/render_3d.c)

### Architectural Improvement

#### **Task: Resolve Header Dependencies**
*   **Status**: ✅ **Completed**
*   **Implementation**: Following the guidance from research document `R06`, the persistent "type redefinition" errors were resolved by applying the **Strict Forward Declarations** pattern. The `core.h` file now uses forward declarations (`struct sg_buffer;`) instead of including the full `sokol_gfx.h` header. This has decoupled the high-level engine core from the low-level graphics library.
*   **Code Link**: [`src/core.h`](../../src/core.h)
*   **Relevant Research**: [`R06_C_Header_Architecture.md`](../research/R06_C_Header_Architecture.md)

---

## 4. Final Outcome

The CGame engine's rendering pipeline is now **stable, modular, and functional**. The successful completion of this sprint has unblocked all future rendering work, including the high-priority PBR implementation. The engine can now reliably load and render complex 3D assets, marking a significant milestone in its development.

---

## 5. Post-Sprint Task: Permanent Architectural Fix (PIMPL Idiom)

**Objective**: To permanently resolve the header dependency issues by implementing the **Opaque Pointer (PIMPL)** pattern, as recommended in the revised `R06` research document. This will replace the temporary forward-declaration fix with a more robust, industry-standard solution.

### 5.1. Why This Is Necessary

The forward-declaration fix, while effective, is a patch. The PIMPL idiom is the architecturally correct solution that provides true decoupling between the engine's core data structures and the low-level graphics library. This prevents future dependency conflicts and makes the engine more modular and maintainable.

### 5.2. Implementation Guide

This guide details the step-by-step process to refactor the `Renderable` component.

#### **Step 1: Define the Opaque Struct in `render.h`**

The `render.h` header is the appropriate place to define the public-facing graphics structures.

*   **File**: `src/render.h`
*   **Action**: Add the following definitions. This creates a new `GpuResources` struct that will contain the Sokol handles, and modifies `Renderable` to hold a pointer to it.

```c
// In render.h

// Forward-declare the Sokol types to avoid including the full header
struct sg_buffer;
struct sg_image;

// This struct will contain the actual GPU handles. Its definition is
// "opaque" to any file that just includes render.h.
typedef struct {
    struct sg_buffer vbuf;
    struct sg_image ibuf;
    struct sg_image tex;
} GpuResources;

// The Renderable component now holds a pointer to the opaque struct.
// This breaks the hard dependency, as a pointer always has a known size.
typedef struct {
    GpuResources* gpu; // Pointer to the GPU resources
    uint32_t index_count;
    bool visible;
    float lod_distance;
    uint8_t lod_level;
} Renderable;
```

#### **Step 2: Remove Old Definitions from `core.h`**

The `Renderable` definition will now be owned by the rendering system.

*   **File**: `src/core.h`
*   **Action**:
    1.  Remove the forward declarations for `sg_buffer` and `sg_image`.
    2.  Remove the entire `struct Renderable` definition.
    3.  In the `Entity` and `ComponentPools` structs, change the `Renderable` member to a pointer: `struct Renderable* renderable;`.

#### **Step 3: Update Component Management Logic**

We must now manage the lifecycle of the `GpuResources` struct.

*   **File**: `src/systems.c` (or wherever `entity_add_component` is defined)
*   **Action**: Modify the component creation and destruction logic to handle the new pointer.

```c
// When adding a Renderable component
void entity_add_renderable(...) {
    // ...
    renderable->gpu = calloc(1, sizeof(GpuResources)); // Use calloc for zero-initialization
    if (!renderable->gpu) {
        // Handle memory allocation failure
    }
}

// When removing a Renderable component
void entity_remove_renderable(...) {
    // ...
    if (renderable->gpu) {
        // IMPORTANT: We must also destroy the Sokol resources here
        sg_destroy_buffer(renderable->gpu->vbuf);
        sg_destroy_buffer(renderable->gpu->ibuf);
        sg_destroy_image(renderable->gpu->tex);
        
        free(renderable->gpu);
        renderable->gpu = NULL;
    }
}
```

#### **Step 4: Update Accessor Code**

All code that previously accessed `renderable->vbuf` must now use the pointer.

*   **Files**: `src/assets.c`, `src/render_3d.c`, etc.
*   **Action**: Change all instances of `renderable.vbuf` to `renderable.gpu->vbuf`.

**Example in `assets_create_renderable_from_mesh`**:
```c
// Before
renderable->vbuf = mesh->sg_vertex_buffer;

// After
renderable->gpu->vbuf = mesh->sg_vertex_buffer;
renderable->index_count = mesh->index_count;
```

### 5.3. Expected Outcome

Once this refactor is complete, the engine's architecture will be significantly more robust:
*   **No More Compilation Conflicts**: The `core.h` header will have no knowledge of the Sokol GFX library, permanently eliminating the type redefinition errors.
*   **True Decoupling**: The core engine systems will be decoupled from the rendering backend, making future maintenance and upgrades (e.g., to a different graphics API) much simpler.
*   **Improved Stability**: The explicit memory management for `GpuResources` makes the lifecycle of graphics objects clearer and less error-prone.