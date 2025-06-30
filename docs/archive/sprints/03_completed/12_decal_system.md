# Sprint 12: Decal System Implementation

**Duration**: 1.5 weeks  
**Priority**: Medium  
**Dependencies**: Sprint 11 (Advanced Lighting & Shadow Mapping)  

## Sprint Goal
Implement a decal system for dynamic surface details like bullet holes, scorch marks, projected textures, and environmental details that can be applied to any surface in the scene.

## Current State
- ✅ PBR materials with full texture support
- ✅ Advanced lighting and shadow mapping
- ✅ Mesh rendering with proper transforms
- ❌ No decal support or projection system
- ❌ No surface detail beyond base textures

## Target State
- ✅ Decal projection system working
- ✅ Multiple decal types (bullet holes, scorch marks, text/logos)
- ✅ Decals interact properly with lighting and shadows
- ✅ Decal management system (creation, destruction, pooling)
- ✅ Integration with ECS for gameplay decals

---

## Research Request

**For Research AI**: Please investigate the following areas to ensure best practices for decal system implementation:

### Decal Rendering Techniques
1. **Decal Projection Mathematics**: Detailed explanation of projective decal mapping and OBB intersection
2. **Screen-Space Decals**: Comparison of object-space vs screen-space decal techniques
3. **Decal Batching**: Strategies for efficiently rendering multiple decals with minimal draw calls
4. **Depth Buffer Usage**: Techniques for proper depth testing and avoiding z-fighting

### Decal-Surface Interaction
1. **Surface Normal Integration**: How to properly blend decal normals with underlying surface normals
2. **UV Coordinate Generation**: Algorithms for generating proper UV coordinates for projected decals
3. **Edge Fading**: Techniques for smoothly fading decal edges to avoid harsh boundaries
4. **Multi-layer Decals**: Handling overlapping decals and proper blending order

### Performance Optimization
1. **Frustum Culling**: Efficient culling of decals outside the view frustum
2. **Spatial Partitioning**: Data structures for fast decal-mesh intersection queries
3. **LOD Systems**: Level-of-detail techniques for decals based on distance and size
4. **Memory Management**: Efficient allocation and deallocation of decal resources

### ECS Integration
1. **Decal Components**: Best practices for integrating decals into an ECS architecture
2. **System Organization**: Optimal ordering of decal systems relative to rendering systems
3. **Event-Driven Decals**: Techniques for spawning decals in response to game events
4. **Decal Lifecycle**: Managing decal creation, updates, and cleanup within ECS

Please provide specific code examples for C99 and recommendations for integration with the existing PBR material system.

---

## Tasks

### Task 12.1: Decal Core System Architecture
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_decals.c`, `src/render_decals.h`

#### Acceptance Criteria
- [ ] Implement Decal structure with transform and material data
- [ ] Add DecalComponent for ECS integration
- [ ] Create decal pool for efficient memory management
- [ ] Implement decal lifecycle (spawn, update, destroy)
- [ ] Add decal type enumeration and template system

#### Implementation Details
```c
typedef enum DecalType {
    DECAL_BULLET_HOLE,
    DECAL_SCORCH_MARK,
    DECAL_BLOOD_SPLATTER,
    DECAL_TEXT_LABEL,
    DECAL_LOGO_PROJECTION,
    DECAL_ENVIRONMENTAL,
    DECAL_TYPE_COUNT
} DecalType;

typedef struct Decal {
    uint32_t id;
    DecalType type;
    
    // Transform data
    float position[3];
    float rotation[4];          // Quaternion
    float scale[3];
    
    // Projection data
    float projection_matrix[16];
    float normal[3];            // Surface normal
    float depth_fade_distance;  // Fade based on depth
    
    // Material data
    uint32_t albedo_texture_id;
    uint32_t normal_texture_id;
    float color[4];
    float intensity;
    float fade_start_time;
    float fade_duration;
    
    // Rendering state
    bool active;
    bool cast_shadows;
    bool receive_shadows;
    uint32_t affected_meshes[8]; // IDs of meshes this decal affects
    uint32_t affected_mesh_count;
} Decal;

typedef struct DecalComponent {
    uint32_t decal_id;
    DecalType type;
    float lifetime;
    float fade_time;
    bool auto_fade;
} DecalComponent;
```

### Task 12.2: Decal Projection Mathematics
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_decals.c`

#### Acceptance Criteria
- [ ] Implement world-to-decal space transformation
- [ ] Add decal bounding box calculation
- [ ] Implement surface normal-based projection
- [ ] Add angle-based projection rejection (prevent glancing angles)
- [ ] Optimize projection calculations for performance

#### Implementation Details
```c
// Decal projection functions
Mat4 decal_calculate_projection_matrix(const Decal* decal);
bool decal_intersects_mesh(const Decal* decal, const Mesh* mesh);
void decal_project_onto_surface(Decal* decal, const float* surface_position, 
                               const float* surface_normal);
float decal_calculate_fade_factor(const Decal* decal, const float* world_position);

// Projection matrix calculation
Mat4 decal_calculate_projection_matrix(const Decal* decal) {
    Mat4 translation = mat4_translation(decal->position);
    Mat4 rotation = mat4_from_quaternion(decal->rotation);
    Mat4 scale = mat4_scale(decal->scale);
    
    return mat4_multiply(mat4_multiply(translation, rotation), scale);
}
```

### Task 12.3: Decal Shader Implementation
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `assets/shaders/decal.vert.metal`, `assets/shaders/decal.frag.metal`

#### Acceptance Criteria
- [ ] Implement deferred decal rendering approach
- [ ] Add decal texture sampling and blending
- [ ] Implement depth-based fading at edges
- [ ] Add normal map blending for decal normals
- [ ] Support alpha blending and multiplicative blending modes

#### Implementation Details
```metal
// Decal vertex shader
struct DecalVertexIn {
    float3 position [[attribute(0)]];
    float2 uv [[attribute(1)]];
};

struct DecalVertexOut {
    float4 position [[position]];
    float3 world_position;
    float4 decal_position;
    float2 uv;
};

vertex DecalVertexOut decal_vertex(DecalVertexIn in [[stage_in]],
                                 constant DecalUniforms& uniforms [[buffer(1)]]) {
    DecalVertexOut out;
    
    float4 world_pos = float4(in.position, 1.0);
    out.position = uniforms.view_projection_matrix * world_pos;
    out.world_position = world_pos.xyz;
    out.decal_position = uniforms.decal_projection_matrix * world_pos;
    out.uv = in.uv;
    
    return out;
}

// Decal fragment shader
fragment float4 decal_fragment(DecalVertexOut in [[stage_in]],
                              texture2d<float> albedo_texture [[texture(0)]],
                              texture2d<float> normal_texture [[texture(1)]],
                              texture2d<float> depth_texture [[texture(2)]],
                              sampler texture_sampler [[sampler(0)]],
                              constant DecalUniforms& uniforms [[buffer(1)]]) {
    
    // Project decal coordinates
    float3 decal_coords = in.decal_position.xyz / in.decal_position.w;
    decal_coords = decal_coords * 0.5 + 0.5; // [-1,1] to [0,1]
    
    // Clip pixels outside decal bounds
    if (any(decal_coords.xy < 0.0) || any(decal_coords.xy > 1.0)) {
        discard;
    }
    
    // Depth-based fading
    float scene_depth = depth_texture.sample(texture_sampler, decal_coords.xy).r;
    float decal_depth = decal_coords.z;
    float depth_fade = saturate((scene_depth - decal_depth) / uniforms.fade_distance);
    
    // Sample decal textures
    float4 decal_color = albedo_texture.sample(texture_sampler, decal_coords.xy);
    float3 decal_normal = normal_texture.sample(texture_sampler, decal_coords.xy).xyz;
    
    // Apply depth fade
    decal_color.a *= depth_fade;
    
    return decal_color;
}
```

### Task 12.4: Decal-Mesh Interaction System
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_decals.c`, `src/render_3d.c`

#### Acceptance Criteria
- [ ] Implement decal-to-mesh assignment system
- [ ] Add spatial culling for decal-mesh pairs
- [ ] Optimize rendering of meshes with multiple decals
- [ ] Implement decal batching for performance
- [ ] Add decal draw call sorting and optimization

#### Implementation Details
```c
typedef struct DecalMeshPair {
    uint32_t decal_id;
    uint32_t mesh_id;
    float distance_squared;     // For sorting
    bool visible;              // Frustum culling result
} DecalMeshPair;

typedef struct DecalRenderBatch {
    uint32_t decal_ids[32];     // Decals in this batch
    uint32_t decal_count;
    uint32_t mesh_id;           // Target mesh
    DecalType type;             // All decals same type for batching
    sg_pipeline pipeline;       // Shader pipeline
} DecalRenderBatch;

// Decal rendering functions
void decal_update_mesh_assignments(World* world);
void decal_cull_invisible_pairs(const Camera* camera);
void decal_create_render_batches(void);
void decal_render_batches(const RenderConfig* config);
```

### Task 12.5: Decal Asset System
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/assets.c`, `data/templates/decals.json`

#### Acceptance Criteria
- [ ] Create decal template system for common types
- [ ] Add decal texture loading and management
- [ ] Implement decal presets (bullet holes, scorch marks, etc.)
- [ ] Add decal animation support (texture atlases)
- [ ] Create decal spawning API for gameplay code

#### Implementation Details
```json
{
  "decal_templates": {
    "bullet_hole_small": {
      "type": "DECAL_BULLET_HOLE",
      "textures": {
        "albedo": "textures/decals/bullet_hole_albedo.png",
        "normal": "textures/decals/bullet_hole_normal.png"
      },
      "properties": {
        "scale": [0.1, 0.1, 0.1],
        "fade_distance": 0.05,
        "lifetime": 30.0,
        "fade_duration": 5.0
      }
    },
    "scorch_mark": {
      "type": "DECAL_SCORCH_MARK",
      "textures": {
        "albedo": "textures/decals/scorch_albedo.png",
        "normal": "textures/decals/scorch_normal.png"
      },
      "properties": {
        "scale": [0.5, 0.5, 0.5],
        "fade_distance": 0.1,
        "lifetime": 60.0,
        "fade_duration": 10.0
      }
    }
  }
}
```

### Task 12.6: ECS Integration & Gameplay API
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/systems.c`, `src/core.h`

#### Acceptance Criteria
- [ ] Add DecalComponent to ECS component system
- [ ] Implement decal spawning from gameplay events
- [ ] Add decal system update function
- [ ] Create high-level decal API for easy use
- [ ] Add decal cleanup and lifetime management

#### Implementation Details
```c
// High-level decal API
uint32_t spawn_decal(World* world, DecalType type, const float* position, 
                    const float* normal, float scale);
void spawn_bullet_hole(World* world, const float* position, const float* normal);
void spawn_scorch_mark(World* world, const float* position, float radius);
void spawn_text_decal(World* world, const char* text, const float* position, 
                     const float* direction);

// ECS system integration
void decal_system_update(World* world, float dt);
void decal_system_render(World* world, const RenderConfig* config);
```

### Task 12.7: Debug UI & Visualization
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/ui.c`

#### Acceptance Criteria
- [ ] Add decal debug panel showing active decals
- [ ] Implement decal bounding box visualization
- [ ] Add decal spawning controls in debug UI
- [ ] Show decal performance metrics
- [ ] Add decal texture preview in debug panel

---

## Acceptance Criteria

### Functional Requirements
- [ ] Decals project correctly onto mesh surfaces
- [ ] Multiple decal types render with proper materials
- [ ] Decals fade in/out smoothly over time
- [ ] Decal-mesh interaction detection works accurately
- [ ] ECS integration allows easy decal spawning

### Performance Requirements
- [ ] Maintain 60 FPS with 50+ active decals
- [ ] Decal rendering adds less than 1ms to frame time
- [ ] Memory usage for decal system under 10MB
- [ ] No performance spikes during decal spawning/cleanup

### Quality Requirements
- [ ] Decals blend naturally with surface materials
- [ ] No visible artifacts at decal edges
- [ ] Proper depth-based fading
- [ ] Realistic lighting interaction with decals

---

## Technical Risks

### High Risk
1. **Depth Buffer Precision**: Z-fighting between decals and surfaces
   - *Mitigation*: Proper depth bias and projection matrix setup
   - *Fallback*: Polygon offset for decal rendering

2. **Performance with Many Decals**: Overdraw and batch complexity
   - *Mitigation*: Aggressive culling and batching system
   - *Fallback*: Decal LOD and maximum count limits

### Medium Risk
1. **Decal-Mesh Assignment**: Complex spatial queries
   - *Mitigation*: Spatial partitioning for decal-mesh pairs
   - *Fallback*: Simplified sphere-based intersection

2. **Texture Memory**: Many decal textures
   - *Mitigation*: Texture atlas and streaming system
   - *Fallback*: Reduce decal texture resolution

### Low Risk
1. **Shader Complexity**: Complex blending modes
   - *Mitigation*: Simplified blending for performance
   - *Fallback*: Basic alpha blending only

---

## Testing Strategy

### Unit Tests
- [ ] Decal projection matrix calculations
- [ ] Mesh-decal intersection algorithms
- [ ] Decal lifecycle management
- [ ] Memory pool allocation/deallocation

### Visual Tests
- [ ] Decal projection accuracy on various surfaces
- [ ] Fade transition smoothness
- [ ] Depth-based edge fading
- [ ] Multi-decal rendering correctness

### Integration Tests
- [ ] ECS decal component integration
- [ ] Decal spawning from gameplay events
- [ ] Performance under heavy decal load
- [ ] Memory usage during decal lifetime cycles

---

## Success Metrics

- **Visual Quality**: Decals integrate seamlessly with scene geometry
- **Performance**: No noticeable performance impact with typical decal counts
- **Usability**: Easy decal spawning API for gameplay programmers
- **Stability**: No memory leaks or crashes during decal operations
- **Flexibility**: Support for various decal types and use cases

---

## Sprint Review

### Definition of Done
- [ ] All tasks completed with comprehensive testing
- [ ] Decal system working on all target platforms
- [ ] Documentation includes decal usage guidelines
- [ ] Performance benchmarks meet requirements
- [ ] Integration tests pass with ECS system

### Demo Preparation
- [ ] Scene with various decal types (bullet holes, scorch marks, text)
- [ ] Real-time decal spawning demonstration
- [ ] Performance comparison with/without decal system
- [ ] Debug visualization of decal bounding boxes and projections
