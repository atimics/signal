# R06: Research - C/C++ Header Architecture and Dependency Management

**ID**: `R06`  
**Author**: Gemini Engineering Analyst  
**Status**: New  
**JIRA**: `CG-215`

## 1. Research Goal

To investigate and define a robust, scalable, and industry-standard header inclusion and dependency management strategy for the CGame engine. The primary objective is to eliminate the recurring "type redefinition" errors caused by including low-level library headers (like `sokol_gfx.h`) in high-level engine headers (like `core.h`). This research will establish a clear architectural pattern for C/C++ development that improves compilation times, reduces coupling, and increases overall code quality.

## 2. Problem Statement

The CGame engine is experiencing persistent compilation failures due to "redefinition of type" errors for `sg_buffer`, `sg_image`, and other Sokol GFX types.

**Symptom**:
```sh
In file included from src/render_mesh.c:5:
In file included from ./src/render_mesh.h:5:
In file included from ./src/render.h:5:
./src/core.h:12:18: error: redefinition of 'sg_buffer'
typedef struct { uint32_t id; } sg_buffer;
                                 ^
/usr/local/include/sokol_gfx.h:989:16: note: previous definition is here
typedef struct sg_buffer { uint32_t id; } sg_buffer;
```

**Root Cause**:
This error occurs because `core.h`, a high-level header defining core engine data structures, directly includes or defines types from `sokol_gfx.h`, a low-level graphics library. When another file, like `render_mesh.c`, includes both `core.h` and `sokol_gfx.h` (often through a chain of other includes), the compiler sees the same type defined twice, leading to a compilation failure. This represents a fundamental violation of the **Dependency Inversion Principle**.

## 3. Candidate Solutions

There are several industry-standard patterns to solve this problem.

### **Candidate A: The Opaque Pointer (PIMPL) Idiom**

This pattern hides the implementation details entirely from the header file, achieving maximum decoupling.

*   **How it Works**: The header file only declares a pointer to an incomplete `struct`. The actual definition of the `struct` and the inclusion of the low-level library happen exclusively in the `.c` file.

*   **Example (`core.h`)**:
    ```c
    // In the header, we only have a forward declaration.
    // The size and members of GpuResources are unknown here.
    struct GpuResources; 

    struct Renderable {
        struct GpuResources* gpu_resources; // Pointer to the implementation
        uint32_t index_count;
        bool visible;
    };
    ```

*   **Example (`core.c`)**:
    ```c
    #include "core.h"
    #include "sokol_gfx.h" // The low-level library is ONLY included here.

    // The actual definition is hidden in the implementation file.
    struct GpuResources {
        sg_buffer vbuf;
        sg_buffer ibuf;
        sg_image tex;
    };
    ```

*   **Pros**:
    *   **Perfect Decoupling**: `core.h` has zero knowledge of Sokol GFX. The rendering backend could be swapped out with no changes to `core.h`.
    *   **Faster Compilation**: Changes to Sokol GFX or `GpuResources` do not require recompiling files that only include `core.h`.
*   **Cons**:
    *   Requires an extra layer of memory indirection (pointer lookup).
    *   Requires dynamic memory allocation (`malloc`) for the `GpuResources` struct, which adds complexity.

### **Candidate B: Strict Forward Declarations**

This is the most common and idiomatic C approach for breaking header dependencies for `struct` types.

*   **How it Works**: The header file forward-declares the `struct` type without defining it. This tells the compiler that the type exists, allowing it to be used in pointers or as an incomplete type. The full definition is only included in the `.c` files that need to know the `struct`'s size and members.

*   **Example (`core.h`)**:
    ```c
    // Forward-declare the Sokol types as structs.
    // We are not including sokol_gfx.h here.
    struct sg_buffer;
    struct sg_image;

    struct Renderable {
        // This is now valid because the compiler knows these are struct types,
        // even if it doesn't know their size or members yet.
        struct sg_buffer vbuf;
        struct sg_image  tex;
        uint32_t index_count;
        bool visible;
    };
    ```
    *Note: This approach fails if the type is a `typedef` to a primitive (e.g., `typedef uint32_t AssetID;`), but it works perfectly for `struct` types like Sokol's.*

*   **Example (`render_3d.c` or `assets.c`)**:
    ```c
    #include "core.h"
    #include "sokol_gfx.h" // The full definition is included here.

    void some_function(struct Renderable* r) {
        // This code can now access the 'id' member because
        // sokol_gfx.h has provided the full definition.
        sg_draw(r->vbuf.id, ...);
    }
    ```

*   **Pros**:
    *   **Solves the Problem Directly**: Eliminates redefinition errors.
    *   **No Performance Overhead**: No extra pointers or allocations.
    *   **Less Intrusive**: Requires minimal changes to existing code structure compared to PIMPL.
*   **Cons**:
    *   Still creates a compile-time dependency, though much weaker than a full `#include`.

### **Candidate C: The Handle System (Interface Segregation)**

This is a more advanced architectural pattern that provides the cleanest separation.

*   **How it Works**: The core engine defines its own generic, opaque handles. A dedicated rendering layer is then responsible for mapping these engine-specific handles to the actual backend-specific resources.

*   **Example (`core.h`)**:
    ```c
    // Define our own, backend-agnostic handle type.
    typedef struct { uint32_t id; } GpuHandle;

    struct Renderable {
        GpuHandle vbuf_handle;
        GpuHandle ibuf_handle;
        GpuHandle texture_handle;
        uint32_t index_count;
        bool visible;
    };
    ```

*   **Example (`renderer.c`)**:
    ```c
    #include "renderer.h"
    #include "sokol_gfx.h"

    // The renderer maintains a mapping from our handle to the real one.
    sg_buffer get_sokol_buffer(GpuHandle handle) {
        // ... lookup logic ...
    }
    ```

*   **Pros**:
    *   **Architecturally Pure**: The core engine is completely independent of the rendering API.
    *   **Most Flexible**: Makes it easiest to support multiple rendering backends (Vulkan, DirectX, etc.) in the future.
*   **Cons**:
    *   **Highest Complexity**: Requires a dedicated abstraction layer to manage the handle mapping.
    *   Likely overkill for the current stage of the project.

## 4. Recommendation

For the CGame engine's current needs, **Candidate B: Strict Forward Declarations** is the most pragmatic and effective solution.

*   **Why?**: It directly solves the compilation error with the least amount of code modification. It is a standard, well-understood C idiom that introduces no performance overhead or memory management complexity. While the Handle System (Candidate C) is architecturally "purer," it represents a significant refactoring effort that is not justified at this stage. The PIMPL idiom (Candidate A) adds unnecessary complexity with dynamic allocation.

## 5. Action Plan

The implementation agent should take the following steps:

1.  **Modify `core.h`**:
    *   Remove any `#include "sokol_gfx.h"` or manual `typedef` definitions of Sokol types.
    *   Add forward declarations for the necessary structs at the top of the file:
        ```c
        struct sg_buffer;
        struct sg_image;
        ```
    *   Ensure the `Renderable` struct uses `struct sg_buffer` and `struct sg_image` to refer to these incomplete types.

2.  **Update Implementation Files (`.c`)**:
    *   Go through all `.c` files that use `Renderable` or other components with graphics types (e.g., `assets.c`, `render_3d.c`, `systems.c`).
    *   Ensure that each of these `.c` files includes the full `sokol_gfx.h` header, preferably near the top. This will provide the full definition needed to work with the structs.

3.  **Compile and Verify**:
    *   Perform a clean build of the entire project (`make clean && make`).
    *   Confirm that all redefinition errors are resolved and the project compiles successfully.

By adopting this strategy, the engine's architecture will be significantly more robust, maintainable, and aligned with professional C development standards.
