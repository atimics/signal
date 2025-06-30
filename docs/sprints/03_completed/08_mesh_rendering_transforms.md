# Sprint 08: Mesh Rendering with Transforms

**Duration**: 1.5 weeks  
**Priority**: High  
**Dependencies**: Sprint 07 (3D Camera System)  

## Sprint Goal
Replace simple triangle rendering with actual mesh rendering, enabling the 4 loaded entities (Control Tower, Sun, Wedge Ships) to be visible in 3D space with proper transforms and materials.

## Current State
- ✅ 3D camera system working
- ✅ Assets loaded (4 meshes, materials, textures)
- ✅ Sokol buffers created for meshes
- ❌ `render_entity_3d()` function commented out
- ❌ Entities not visible (only test triangle renders)
- ❌ No transform matrix calculations

## Target State
- ✅ All 4 entities visible as 3D meshes
- ✅ Proper transform matrices (position, rotation, scale)
- ✅ Mesh vertex/index buffers rendering correctly
- ✅ Basic material colors applied
- ✅ Entities positioned at spawned locations (-15, 0, 15, 30 X coords)

---

## Research Request

**For Research AI**: Please investigate the following areas to ensure best practices for mesh rendering and transform system implementation:

### 3D Transform Mathematics
1. **Matrix Hierarchies**: Detailed explanation of parent-child transform relationships and matrix composition
2. **Transform Decomposition**: Techniques for extracting position, rotation, and scale from matrices
3. **Quaternion Integration**: Best practices for using quaternions in transform systems
4. **Transform Interpolation**: Smooth interpolation techniques for position, rotation, and scale

### Mesh Rendering Optimization
1. **Vertex Buffer Management**: Optimal strategies for managing static and dynamic vertex data
2. **Index Buffer Usage**: Best practices for triangle indexing and primitive restart
3. **Instanced Rendering**: Techniques for rendering multiple instances of the same mesh efficiently
4. **Mesh Batching**: Strategies for combining multiple meshes into single draw calls

### Culling and Performance
1. **Bounding Volume Hierarchies**: Implementation of AABBs and sphere culling for meshes
2. **Frustum Culling**: Efficient algorithms for culling meshes outside the camera view
3. **Occlusion Culling**: Basic occlusion testing techniques for hidden mesh removal
4. **Level of Detail**: Distance-based LOD systems for mesh complexity reduction

### ECS and Data Organization
1. **Transform Components**: Optimal data layout for transform components in ECS
2. **Mesh Components**: Best practices for storing mesh data and references
3. **System Ordering**: Proper ordering of transform and rendering systems
4. **Dirty Flagging**: Efficient techniques for tracking when transforms need matrix updates

Please provide specific code examples for matrix operations in C99 and recommendations for integration with Sokol's rendering commands and uniform buffers.

---

## Tasks

### Task 8.1: Transform Matrix Implementation
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Implement `mat4_translation()` function
- [ ] Implement `mat4_rotation()` functions (X, Y, Z axes)
- [ ] Implement `mat4_scale()` function
- [ ] Implement `mat4_compose_transform()` for TRS matrices
- [ ] Add transform utilities for common operations

#### Implementation Details
```c
// Transform matrix functions
void mat4_translation(float* m, Vector3 translation) {
    mat4_identity(m);
    m[12] = translation.x;
    m[13] = translation.y;
    m[14] = translation.z;
}

void mat4_rotation_y(float* m, float angle_radians) {
    mat4_identity(m);
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    m[0] = c;  m[2] = s;
    m[8] = -s; m[10] = c;
}

void mat4_scale(float* m, Vector3 scale) {
    mat4_identity(m);
    m[0] = scale.x;
    m[5] = scale.y;
    m[10] = scale.z;
}

void mat4_compose_transform(float* result, Vector3 position, Vector3 rotation, Vector3 scale) {
    float translation[16], rot_x[16], rot_y[16], rot_z[16], scaling[16];
    float temp1[16], temp2[16], temp3[16];
    
    // Create individual matrices
    mat4_translation(translation, position);
    mat4_rotation_x(rot_x, rotation.x);
    mat4_rotation_y(rot_y, rotation.y);  
    mat4_rotation_z(rot_z, rotation.z);
    mat4_scale(scaling, scale);
    
    // Combine: T * R * S
    mat4_multiply(temp1, rot_y, rot_x);
    mat4_multiply(temp2, rot_z, temp1);
    mat4_multiply(temp3, temp2, scaling);
    mat4_multiply(result, translation, temp3);
}
```

#### Testing
- Unit tests for individual matrix functions
- Visual verification with simple geometry
- Performance benchmarking for matrix operations

---

### Task 8.2: render_entity_3d Implementation
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Uncomment and fix `render_entity_3d()` function
- [ ] Implement proper component access (Transform, Renderable)
- [ ] Calculate model matrix from entity transform
- [ ] Combine model matrix with camera VP matrix
- [ ] Apply mesh buffers and draw individual entities

#### Implementation Details
```c
void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config) {
    if (!render_state.initialized) {
        return;
    }

    // Get entity components
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Renderable* renderable = entity_get_renderable(world, entity_id);

    if (!transform || !renderable) {
        return;
    }

    // Validate mesh and material IDs
    if (renderable->mesh_id >= config->assets->mesh_count || 
        renderable->material_id >= config->assets->material_count) {
        return;
    }
    
    Mesh* mesh = &config->assets->meshes[renderable->mesh_id];
    Material* material = &config->assets->materials[renderable->material_id];

    // Validate mesh buffers
    sg_resource_state vb_state = sg_query_buffer_state(mesh->sg_vertex_buffer);
    sg_resource_state ib_state = sg_query_buffer_state(mesh->sg_index_buffer);
    
    if (vb_state != SG_RESOURCESTATE_VALID || ib_state != SG_RESOURCESTATE_VALID) {
        static bool warned = false;
        if (!warned) {
            printf("⚠️ Invalid mesh buffers for %s (VB:%d IB:%d)\n", mesh->name, vb_state, ib_state);
            warned = true;
        }
        return;
    }

    // Get active camera
    EntityID active_camera = world_get_active_camera(world);
    struct Camera* camera = entity_get_camera(world, active_camera);
    
    // Create model matrix from transform
    float model_matrix[16];
    mat4_compose_transform(model_matrix, transform->position, transform->rotation, transform->scale);
    
    // Create MVP matrix
    float mvp_matrix[16];
    if (camera) {
        mat4_multiply(mvp_matrix, camera->view_projection_matrix, model_matrix);
    } else {
        // Fallback to simple positioning
        memcpy(mvp_matrix, model_matrix, sizeof(mvp_matrix));
    }

    // Apply mesh bindings
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = mesh->sg_vertex_buffer,
        .index_buffer = mesh->sg_index_buffer,
        .images[0] = render_state.default_texture,  // TODO: Use material texture
        .samplers[0] = render_state.sampler
    });

    // Upload uniforms
    vs_uniforms_t uniforms;
    memcpy(uniforms.mvp, mvp_matrix, sizeof(uniforms.mvp));
    
    sg_range uniform_data = SG_RANGE(uniforms);
    sg_apply_uniforms(0, &uniform_data);

    // Draw mesh
    sg_draw(0, mesh->index_count, 1);
}
```

#### Testing
- Individual entity rendering
- Multiple entities in scene
- Transform changes affect rendering
- Buffer validation and error handling

---

### Task 8.3: Multi-Entity Rendering Loop
**Estimated**: 0.5 days  
**Assignee**: Developer  
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Uncomment entity iteration loop in `render_frame()`
- [ ] Remove or conditionally disable triangle rendering
- [ ] Add entity rendering statistics and debugging
- [ ] Optimize rendering loop for performance

#### Implementation Details
```c
void render_frame(struct World* world, RenderConfig* config, EntityID player_id, float delta_time) {
    (void)delta_time;
    
    if (!render_state.initialized) {
        printf("⚠️ Render state not initialized, skipping frame\n");
        return;
    }

    // Check pipeline validity
    sg_resource_state pipeline_state = sg_query_pipeline_state(render_state.pipeline);
    if (pipeline_state != SG_RESOURCESTATE_VALID) {
        printf("⚠️ Pipeline not valid (state=%d), skipping frame\n", pipeline_state);
        return;
    }

    // Apply the rendering pipeline
    sg_apply_pipeline(render_state.pipeline);
    
    // Render statistics
    int rendered_count = 0;
    int skipped_count = 0;

    // Iterate over all renderable entities
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        if (entity->component_mask & COMPONENT_RENDERABLE) {
            render_entity_3d(world, entity->id, config);
            rendered_count++;
        } else {
            skipped_count++;
        }
    }

    // Debug output (occasional)
    static int debug_counter = 0;
    if (++debug_counter % 300 == 0) { // Every 5 seconds at 60fps
        printf("🎨 Rendered %d entities, skipped %d\n", rendered_count, skipped_count);
    }

    // Optional: Keep triangle for debugging (remove later)
    // render_debug_triangle();
    
    // Render debug info if enabled
    if (config->show_debug_info) {
        render_debug_info(world, config);
    }
}
```

#### Testing
- All 4 entities render correctly
- Performance impact measurement
- Debug output shows correct counts
- No rendering artifacts

---

### Task 8.4: Material Color Integration
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/render_3d.c`, `assets/shaders/basic_3d_simple.frag.metal`

#### Acceptance Criteria
- [ ] Update fragment shader to use material colors
- [ ] Pass material diffuse color as uniform
- [ ] Implement basic material uniform system
- [ ] Different entities show different colors

#### Implementation Details

**Enhanced Uniform Structure**:
```c
typedef struct {
    float mvp[16];           // Model-View-Projection matrix
    float diffuse_color[4];  // Material diffuse color (RGBA)
} vs_uniforms_t;
```

**Updated Fragment Shader** (`assets/shaders/basic_3d_simple.frag.metal`):
```metal
#include <metal_stdlib>
using namespace metal;

struct fs_in {
    float3 normal;
    float2 texcoord;
};

struct fs_uniforms {
    float4 diffuse_color;
};

fragment float4 fs_main(fs_in in [[stage_in]], 
                       constant fs_uniforms& uniforms [[buffer(0)]],
                       texture2d<float> diffuse_texture [[texture(0)]],
                       sampler diffuse_sampler [[sampler(0)]]) {
    // Simple material color with basic lighting
    float3 normal = normalize(in.normal);
    float light = max(0.3, dot(normal, float3(0.0, -1.0, -0.5))); // Basic directional light
    
    return float4(uniforms.diffuse_color.rgb * light, uniforms.diffuse_color.a);
}
```

**Material Application**:
```c
// In render_entity_3d()
vs_uniforms_t uniforms;
memcpy(uniforms.mvp, mvp_matrix, sizeof(uniforms.mvp));

// Apply material color
uniforms.diffuse_color[0] = material->diffuse_color.x;
uniforms.diffuse_color[1] = material->diffuse_color.y; 
uniforms.diffuse_color[2] = material->diffuse_color.z;
uniforms.diffuse_color[3] = 1.0f; // Alpha

sg_range uniform_data = SG_RANGE(uniforms);
sg_apply_uniforms(0, &uniform_data);
```

#### Testing
- Different entities show different colors
- Material colors match asset definitions
- Basic lighting affects color intensity
- No shader compilation errors

---

### Task 8.5: Debugging and Validation
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: Various

#### Acceptance Criteria
- [ ] Add comprehensive entity rendering debugging
- [ ] Validate mesh data integrity
- [ ] Add performance monitoring for mesh rendering
- [ ] Create debug camera positions for testing

#### Implementation Details
```c
// Debug functions
void debug_entity_rendering(struct World* world, EntityID entity_id) {
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Renderable* renderable = entity_get_renderable(world, entity_id);
    
    printf("Entity %d: pos(%.1f,%.1f,%.1f) mesh_id=%d material_id=%d\n",
           entity_id, transform->position.x, transform->position.y, transform->position.z,
           renderable->mesh_id, renderable->material_id);
}

void validate_mesh_data(Mesh* mesh) {
    printf("Mesh '%s': %d vertices, %d indices, buffers(VB:%d IB:%d)\n",
           mesh->name, mesh->vertex_count, mesh->index_count,
           sg_query_buffer_state(mesh->sg_vertex_buffer),
           sg_query_buffer_state(mesh->sg_index_buffer));
}
```

#### Testing
- Debug output helps identify rendering issues
- Performance metrics show acceptable frame rates
- Mesh validation catches data corruption
- Debug cameras provide good testing views

---

## Definition of Done

### Technical Requirements
- [ ] All 4 entities visible as 3D meshes
- [ ] Transform matrices working correctly
- [ ] `render_entity_3d()` function fully functional
- [ ] Material colors applied correctly
- [ ] No Sokol validation errors

### Visual Requirements
- [ ] Control Tower visible at (-15, 0, 0)
- [ ] Sun visible at (0, 0, 0)  
- [ ] Wedge Ship visible at (15, 0, 0)
- [ ] Wedge Ship Mk2 visible at (30, 0, 0)
- [ ] Different entities show different colors
- [ ] Entities respond to camera movement

### Performance Requirements
- [ ] Frame rate maintains 60+ FPS with 4 entities
- [ ] Mesh rendering optimized for multiple objects
- [ ] Memory usage stable during entity rendering
- [ ] No excessive GPU state changes

### Documentation Requirements
- [ ] Entity rendering pipeline documented
- [ ] Transform matrix calculations explained
- [ ] Debugging procedures documented
- [ ] Sprint retrospective completed

---

## Risk Assessment

### High Risk
- **Mesh buffer corruption**: Invalid buffers could crash rendering
  - *Mitigation*: Comprehensive validation, error handling
- **Performance degradation**: Multiple entities could impact frame rate
  - *Mitigation*: Performance monitoring, optimization

### Medium Risk
- **Transform matrix bugs**: Incorrect positioning or scaling
  - *Mitigation*: Unit tests, visual verification
- **Shader compilation issues**: Material shader changes could break
  - *Mitigation*: Incremental shader changes, fallbacks

### Low Risk
- **Material color accuracy**: Minor color differences from expected
  - *Mitigation*: Color validation, asset verification

---

## Testing Strategy

### Unit Tests
- Transform matrix calculations
- Component access validation
- Buffer state checking

### Integration Tests
- Multi-entity rendering
- Camera integration with entity rendering
- Material system integration

### Visual Tests
- All entities visible and correctly positioned
- Camera movement affects all entities
- Material colors match expectations

### Performance Tests
- Frame rate with 4 entities vs triangle
- Memory usage monitoring
- GPU profiling for optimization opportunities

---

## Sprint Retrospective

### What Went Well
- [ ] Transform matrix implementation
- [ ] Entity rendering integration
- [ ] Material color system
- [ ] Debugging and validation

### What Could Be Improved
- [ ] Performance optimization
- [ ] Error handling coverage
- [ ] Code organization
- [ ] Testing completeness

### Action Items for Next Sprint
- [ ] Performance improvements identified
- [ ] Additional debugging tools needed
- [ ] Code refactoring opportunities
- [ ] Foundation for texture system

---

**Previous Sprint**: [Sprint 07: 3D Camera System](07_camera_system_3d.md)  
**Next Sprint**: [Sprint 09: Texture System Implementation](09_texture_system.md)
