# R06: Research - C/C++ Header Architecture and Dependency Management

**ID**: `R06`  
**Author**: Gemini Engineering Analyst  
**Version**: 2.1 (Revised after implementation feedback)  
**Status**: Active Reference

## 1. Research Goal

To investigate and define a robust, scalable, and industry-standard header inclusion and dependency management strategy for the CGame engine. The primary objective is to eliminate the recurring "type redefinition" errors caused by including low-level library headers (like `sokol_gfx.h`) in high-level engine headers (like `core.h`). This research will establish a clear architectural pattern for C/C++ development that improves compilation times, reduces coupling, and increases overall code quality.

## 2. Problem Statement & Analysis

The CGame engine experiences persistent compilation failures due to "redefinition of type" errors for `sg_buffer`, `sg_image`, etc.

**Root Cause**: This error occurs because `core.h`, a high-level header, directly includes or defines types from `sokol_gfx.h`, a low-level graphics library. This violates the **Dependency Inversion Principle**, creating a fragile and tightly-coupled architecture.

**Implementation Finding**: An initial attempt to use simple forward declarations (`struct sg_buffer;`) failed. This is due to a fundamental C language rule: **an incomplete type (a forward-declared struct) cannot be used as a member variable by value**, because the compiler does not know the size of the type and cannot calculate the size of the containing struct (`Renderable`). It can only be used as a **pointer**.

## 3. Candidate Solutions (Revised Analysis)

### **Candidate A: The Opaque Pointer (PIMPL) Idiom**

This pattern is the **correct and most robust solution** for this problem. It hides implementation details entirely from the header file, achieving maximum decoupling.

*   **How it Works**: The header file only declares a pointer to an incomplete `struct`. The actual definition of the `struct` and the inclusion of the low-level library happen exclusively in the `.c` file. This works because a pointer always has a known, fixed size, regardless of the size of the type it points to.

*   **Example (`core.h`)**:
    ```c
    // In the header, we only have a forward declaration.
    // The size and members of GpuResources are unknown here.
    struct GpuResources; 

    struct Renderable {
        // This is the key: we store a POINTER to the graphics-specific data.
        struct GpuResources* gpu_resources; 
        uint32_t index_count;
        bool visible;
    };
    ```

*   **Example (`core.c` or `systems.c`)**:
    ```c
    #include "core.h"
    #include "sokol_gfx.h" // The low-level library is ONLY included here.

    // The actual definition is hidden in the implementation file.
    struct GpuResources {
        sg_buffer vbuf;
        sg_buffer ibuf;
        sg_image tex;
    };

    // Usage requires allocation and dereferencing
    void some_function(struct Renderable* r) {
        // We must allocate memory for the opaque struct
        r->gpu_resources = malloc(sizeof(struct GpuResources));
        // Now we can access the members
        r->gpu_resources->vbuf = sg_make_buffer(...);
    }
    ```

*   **Pros**:
    *   **Correctly Solves the Problem**: The only viable pattern that fully decouples the headers.
    *   **Perfect Decoupling**: `core.h` has zero knowledge of Sokol GFX.
    *   **Faster Compilation**: Changes to graphics libraries do not require recompiling the entire engine.
*   **Cons**:
    *   Requires dynamic memory allocation (`malloc`/`free`), which must be managed carefully.
    *   Adds a layer of memory indirection (a pointer lookup), though the performance impact is negligible in this context.

### **Candidate B: Strict Forward Declarations (Re-evaluated)**

*   **Status**: **NOT VIABLE** for this use case.
*   **Reason for Failure**: This pattern is only viable if the incomplete type is used as a *pointer* (`struct sg_buffer* vbuf;`). Because the `Renderable` component was designed to hold the graphics handles *by value*, this pattern is not applicable without changing the struct to use pointers, which effectively turns it into Candidate A.

### **Candidate C: The Handle System (Interface Segregation)**

*   **Status**: **VIABLE, but Over-engineered** for the current stage.
*   **Analysis**: This pattern, which involves creating engine-specific `GpuHandle` types, is an excellent architectural goal for the long term, especially if supporting multiple graphics backends (Vulkan, DirectX) becomes a requirement. However, it introduces a significant amount of abstraction and complexity that is not necessary to solve the immediate problem. It remains a good future-state architecture to aspire to.

## 4. Recommendation (Revised)

The **Opaque Pointer (PIMPL) Idiom (Candidate A)** is the correct, industry-standard, and most pragmatic solution for the CGame engine's current architectural needs. It provides the necessary decoupling to solve the compilation errors while introducing a manageable level of complexity.

## 5. Action Plan (Revised)

The implementation agent should take the following steps to refactor the `Renderable` component using the Opaque Pointer pattern:

1.  **Modify `core.h`**:
    *   Remove any manual `typedef` definitions of Sokol types.
    *   Add a forward declaration for a new, engine-specific struct: `struct GpuResources;`
    *   Change the `Renderable` struct to hold a *pointer* to this new struct:
        ```c
        struct Renderable {
            struct GpuResources* gpu_resources; // Pointer to graphics data
            uint32_t index_count;
            bool visible;
            // ... other non-graphics fields ...
        };
        ```

2.  **Create `GpuResources` Definition**:
    *   In a suitable `.c` file that already includes `sokol_gfx.h` (e.g., `assets.c` or `render_3d.c`), define the `GpuResources` struct:
        ```c
        #include "sokol_gfx.h"
        
        struct GpuResources {
            sg_buffer vbuf;
            sg_buffer ibuf;
            sg_image tex;
        };
        ```

3.  **Update Component Creation Logic**:
    *   In the `entity_add_component` function for `COMPONENT_RENDERABLE`, you must now dynamically allocate memory for `gpu_resources`:
        ```c
        // In entity_add_component for COMPONENT_RENDERABLE...
        struct Renderable* r = ...;
        r->gpu_resources = calloc(1, sizeof(struct GpuResources)); // Use calloc for zero-initialization
        if (!r->gpu_resources) { /* handle allocation failure */ }
        ```

4.  **Update Component Destruction Logic**:
    *   In `entity_remove_component` or `world_destroy`, you must now `free` the allocated memory to prevent leaks:
        ```c
        // When a renderable component is destroyed...
        struct Renderable* r = ...;
        if (r->gpu_resources) {
            free(r->gpu_resources);
            r->gpu_resources = NULL;
        }
        ```

5.  **Update Accessor Logic**:
    *   All code that previously accessed `renderable->vbuf` must now access it via the pointer: `renderable->gpu_resources->vbuf`.

This revised plan provides a clear and architecturally sound path forward, resolving the immediate compilation issues while improving the engine's overall design.