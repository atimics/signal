// R06 Implementation Plan: Asset System Refactoring
// This document outlines the complete refactoring needed for R06 compliance

# R06 Implementation Plan: Comprehensive Header Decoupling

# R06 Implementation Plan: Comprehensive Header Decoupling

## Current State Analysis (Updated)

**PARALLEL IMPLEMENTATION DISCOVERED**: A working implementation has been created that uses a "Central Graphics Header" approach rather than the R06-recommended PIMPL pattern.

### What Has Been Implemented:
1. **`graphics_api.h/c`** - Central hub including all Sokol headers (`sokol_gfx.h`, `sokol_app.h`, etc.)
2. **Updated `.c` files** - All implementation files now include `graphics_api.h` instead of direct Sokol includes
3. **Working build** - Compilation succeeds and engine runs

### R06 Compliance Assessment:

**❌ CRITICAL R06 VIOLATIONS REMAIN:**
- `assets.h` still exposes `sg_buffer`, `sg_image` types in public API (lines 33-34, 42)
- `render_mesh.h` includes `graphics_api.h` directly, violating dependency inversion principle
- Headers still contain Sokol types in struct definitions (`MeshRenderer` struct contains `sg_pipeline`)
- **Risk**: Future type redefinition errors when multiple modules include headers with conflicting Sokol type exposure

**✅ PARTIAL BENEFITS ACHIEVED:**
- Centralized Sokol include management
- Working compilation
- Reduced scattered includes

## Problem Summary
While the current approach solves immediate compilation issues, it **does not achieve R06's core goal** of header decoupling. Headers still expose low-level graphics API types, creating fragile dependencies.

## Critical Issues (Updated Priority)
1. **URGENT**: `assets.h` still directly exposes `sg_buffer`, `sg_image` types (lines 33-34, 42) - R06 violation
2. **HIGH**: `render_mesh.h` includes `graphics_api.h` and exposes `sg_pipeline` in struct - R06 violation  
3. **MEDIUM**: Other headers may have similar violations not yet discovered
4. **ARCHITECTURAL**: Current approach provides compilation success but not true R06 decoupling

## Recommended Path Forward

### Option A: Complete R06 Implementation (RECOMMENDED)
**Goal**: Achieve true header decoupling per R06 principles
**Approach**: Implement PIMPL pattern throughout codebase
**Benefits**: Future-proof architecture, true decoupling, eliminates all type conflicts
**Timeline**: 2-3 hours of focused refactoring

### Option B: Hybrid Approach (PRAGMATIC)
**Goal**: Fix remaining violations while keeping current working approach
**Approach**: Apply PIMPL only to problematic headers (`assets.h`, `render_mesh.h`)
**Benefits**: Minimal disruption, maintains working build, addresses critical violations
**Timeline**: 30-60 minutes

### Option C: Accept Current State (NOT RECOMMENDED)
**Goal**: Ship current working implementation
**Risk**: Future type conflicts, architectural debt, R06 goals not achieved

## Phase 1: Fix Immediate R06 Violations (CRITICAL)

### Step 1.1: Fix assets.h Sokol Type Exposure
**CURRENT ISSUE**: `assets.h` lines 33-34, 42 directly expose `sg_buffer`, `sg_image`
```c
// CURRENT (VIOLATES R06):
typedef struct {
    // ...
    sg_buffer sg_vertex_buffer;    // ❌ Direct Sokol exposure
    sg_buffer sg_index_buffer;     // ❌ Direct Sokol exposure
} Mesh;

typedef struct {
    // ...
    sg_image sg_image;             // ❌ Direct Sokol exposure
} Texture;
```

**SOLUTION**: Apply PIMPL pattern to Mesh and Texture structs
```c
// R06 COMPLIANT:
struct MeshGpuResources;  // Forward declaration
typedef struct {
    // ...
    struct MeshGpuResources* gpu_resources;  // ✅ Opaque pointer
} Mesh;

struct TextureGpuResources;  // Forward declaration  
typedef struct {
    // ...
    struct TextureGpuResources* gpu_resources;  // ✅ Opaque pointer
} Texture;
```

### Step 1.2: Fix render_mesh.h Header Dependencies
**CURRENT ISSUE**: `render_mesh.h` includes `graphics_api.h` and exposes `sg_pipeline`
```c
// CURRENT (VIOLATES R06):
#include "graphics_api.h"  // ❌ Header depends on graphics API
typedef struct {
    sg_pipeline pipeline;  // ❌ Direct Sokol exposure
    sg_shader shader;      // ❌ Direct Sokol exposure
    sg_sampler sampler;    // ❌ Direct Sokol exposure
} MeshRenderer;
```

**SOLUTION**: Apply PIMPL pattern to MeshRenderer
```c
// R06 COMPLIANT:
struct MeshRendererGpuResources;  // Forward declaration
typedef struct {
    struct MeshRendererGpuResources* gpu_resources;  // ✅ Opaque pointer
    // ... other non-graphics fields
} MeshRenderer;
```

## Phase 2: Complete PIMPL Implementation

### Step 2.1: Create GPU Resource Implementations
**Create opaque struct definitions in implementation files**
```c
// In assets.c:
struct MeshGpuResources {
    sg_buffer sg_vertex_buffer;
    sg_buffer sg_index_buffer;
};

struct TextureGpuResources {
    sg_image sg_image;
};

// In render_mesh.c:
struct MeshRendererGpuResources {
    sg_pipeline pipeline;
    sg_shader shader;
    sg_sampler sampler;
};
```

### Step 2.2: Create Accessor Functions
**Provide controlled access to GPU resources**
```c
// In assets.h (declarations only):
bool mesh_get_gpu_buffers(Mesh* mesh, void** vertex_buffer, void** index_buffer);
bool texture_get_gpu_image(Texture* texture, void** image);

// In render_mesh.h:
bool mesh_renderer_get_pipeline(MeshRenderer* renderer, void** pipeline);
```

### Step 2.3: Update All Usage Sites
**Replace direct member access with function calls**
- Update all `mesh->sg_vertex_buffer` to `mesh_get_vertex_buffer(mesh)`
- Update all `texture->sg_image` to `texture_get_gpu_image(texture)`
- Update render loop to use accessor functions

## Phase 3: Validation and Testing

### Step 3.1: Header Dependency Audit
**Verify R06 compliance**
```bash
# No header should contain sg_ types except in implementation files
grep -r "sg_" src/*.h  # Should return only forward declarations
grep -r "#include.*sokol" src/*.h  # Should return empty
```

### Step 3.2: Build and Runtime Testing
- Verify clean compilation
- Test asset loading functionality
- Verify rendering still works
- Check for memory leaks in GPU resource management

## Expected Outcome
- **✅ R06 Compliance**: Zero Sokol types in any `.h` file except forward declarations
- **✅ Maintainable Build**: Current working compilation maintained throughout refactoring
- **✅ Future-Proof**: True architectural decoupling prevents future type conflicts
- **✅ Performance**: Minimal runtime overhead (PIMPL adds one pointer indirection)
- **✅ Compatibility**: Existing `graphics_api.h` approach works alongside PIMPL improvements

## Implementation Priority (Updated)
1. **IMMEDIATE**: Fix `assets.h` Sokol type exposure (affects multiple modules)
2. **HIGH**: Fix `render_mesh.h` graphics API dependency  
3. **MEDIUM**: Complete PIMPL pattern throughout codebase
4. **FUTURE**: Consider full graphics abstraction layer for multi-API support

## Coding Agent Guidance
**RECOMMENDED APPROACH**: Implement **Option B (Hybrid)** - Fix the critical R06 violations in `assets.h` and `render_mesh.h` while preserving the working `graphics_api.h` infrastructure. This provides:
- ✅ Immediate R06 compliance for the most problematic areas
- ✅ Maintains current working build system
- ✅ Sets foundation for future complete PIMPL implementation
- ✅ Minimal disruption to parallel development work
