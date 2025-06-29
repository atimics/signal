# Sprint 11: Advanced Lighting & Shadow Mapping

**Duration**: 2.5 weeks  
**Priority**: Medium  
**Dependencies**: Sprint 10 (PBR Materials & Advanced Lighting)  

## Sprint Goal
Implement advanced lighting techniques including shadow mapping, point lights, spot lights, and environmental lighting to create realistic and dynamic lighting scenarios.

## Current State
- ✅ PBR materials working with directional lighting
- ✅ Cook-Torrance BRDF implementation
- ✅ Multi-texture support active
- ❌ Only directional lights supported
- ❌ No shadow casting or receiving
- ❌ No environmental lighting (IBL)

## Target State
- ✅ Shadow mapping for directional lights working
- ✅ Point lights and spot lights implemented
- ✅ Image-based lighting (IBL) foundation
- ✅ Light attenuation and falloff realistic
- ✅ Dynamic light management system

---

## Research Request

**For Research AI**: Please investigate the following areas to ensure best practices for advanced lighting implementation with Sokol:

### Shadow Mapping Techniques
1. **Shadow Map Theory**: Comprehensive coverage of shadow mapping algorithms, including cascaded shadow maps
2. **Shadow Artifacts**: Common shadow artifacts and their solutions (shadow acne, peter panning, light bleeding)
3. **PCF and Advanced Filtering**: Implementation details for Percentage Closer Filtering and other soft shadow techniques
4. **Cascade Split Optimization**: Best practices for calculating optimal cascade split distances

### Light Types and Attenuation
1. **Point Light Mathematics**: Proper attenuation curves and light falloff calculations
2. **Spot Light Implementation**: Cone angle calculations and smooth falloff techniques
3. **Area Light Approximation**: Techniques for approximating area lights with point/spot lights
4. **Light Culling**: Efficient techniques for culling lights outside the view frustum

### Performance Optimization
1. **Light Batching**: Strategies for batching multiple lights in a single render pass
2. **Shadow Map Caching**: Techniques for avoiding unnecessary shadow map updates
3. **LOD for Shadows**: Level-of-detail systems for shadow quality based on distance
4. **GPU Performance**: Optimization techniques specific to mobile and desktop GPUs

### Sokol Integration
1. **Render Target Management**: Best practices for managing shadow map render targets in Sokol
2. **Multi-pass Rendering**: Efficient organization of shadow and lighting passes
3. **Uniform Buffer Organization**: Optimal layout for light data in uniform buffers
4. **Debug Visualization**: Techniques for visualizing shadow maps and light volumes

Please provide shader code examples and specific performance benchmarks for different shadow map resolutions and light counts.

---

## Tasks

### Task 11.1: Shadow Mapping System
**Estimated**: 4 days  
**Assignee**: Developer  
**Files**: `src/render_lighting.c`, `assets/shaders/shadow.vert.metal`, `assets/shaders/shadow.frag.metal`

#### Acceptance Criteria
- [ ] Implement shadow map render targets (1024x1024, 2048x2048)
- [ ] Add depth-only rendering pass for shadow generation
- [ ] Implement PCF (Percentage Closer Filtering) for soft shadows
- [ ] Add shadow bias and slope bias controls
- [ ] Support for 4 cascaded shadow maps for directional lights

#### Implementation Details
```c
typedef struct ShadowMapConfig {
    uint32_t resolution;        // 1024, 2048, 4096
    uint32_t cascade_count;     // 1-4 cascades
    float cascade_splits[4];    // Split distances
    float bias;                 // Shadow bias
    float slope_bias;           // Slope-based bias
    bool pcf_enabled;           // Soft shadows
    uint32_t pcf_samples;       // PCF sample count
} ShadowMapConfig;

typedef struct ShadowMap {
    sg_image depth_texture;
    sg_pass render_pass;
    sg_pipeline pipeline;
    Mat4 light_view_matrix;
    Mat4 light_projection_matrix;
    Mat4 bias_matrix;
} ShadowMap;
```

### Task 11.2: Point Light Implementation
**Estimated**: 3 days  
**Assignee**: Developer  
**Files**: `src/render_lighting.c`, `src/render_lighting.h`

#### Acceptance Criteria
- [ ] Implement point light structure with position and range
- [ ] Add quadratic light attenuation formula
- [ ] Support for up to 16 dynamic point lights
- [ ] Implement light culling based on camera frustum
- [ ] Add point light shadow mapping (cube maps)

#### Implementation Details
```c
typedef struct PointLight {
    float position[3];
    float color[3];
    float intensity;
    float range;                // Light attenuation range
    float constant_attenuation; // Usually 1.0
    float linear_attenuation;   // Distance factor
    float quadratic_attenuation;// Distance squared factor
    bool cast_shadows;
    sg_image shadow_cube_map;   // 6-face cube map
} PointLight;

// Light attenuation calculation
float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
```

### Task 11.3: Spot Light Implementation
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_lighting.c`

#### Acceptance Criteria
- [ ] Implement spot light with position, direction, and cone angles
- [ ] Add inner and outer cone angle controls
- [ ] Implement smooth falloff between inner and outer cones
- [ ] Support for up to 8 dynamic spot lights
- [ ] Add spot light shadow mapping

#### Implementation Details
```c
typedef struct SpotLight {
    float position[3];
    float direction[3];
    float color[3];
    float intensity;
    float range;
    float inner_cone_angle;     // Full intensity cone (radians)
    float outer_cone_angle;     // Zero intensity cone (radians)
    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
    bool cast_shadows;
    ShadowMap shadow_map;
} SpotLight;
```

### Task 11.4: Image-Based Lighting (IBL) Foundation
**Estimated**: 3 days  
**Assignee**: Developer  
**Files**: `src/render_lighting.c`, `assets/shaders/ibl.frag.metal`

#### Acceptance Criteria
- [ ] Implement environment cube map loading
- [ ] Add diffuse irradiance map generation
- [ ] Implement specular reflection map with mip levels
- [ ] Add BRDF integration lookup table
- [ ] Basic environment reflection on materials

#### Implementation Details
```c
typedef struct EnvironmentMap {
    sg_image environment_cube;      // HDR environment
    sg_image irradiance_cube;      // Diffuse irradiance
    sg_image specular_cube;        // Specular reflections
    sg_image brdf_lut;             // BRDF lookup table
    float intensity;               // Environment intensity
    float rotation;                // Environment rotation
} EnvironmentMap;
```

### Task 11.5: Dynamic Light Management
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/systems.c`, `src/core.h`

#### Acceptance Criteria
- [ ] Implement LightComponent for ECS integration
- [ ] Add light spawning and despawning system
- [ ] Implement light animation and movement
- [ ] Add light intensity and color animation
- [ ] Performance optimization for many lights (light culling)

#### Implementation Details
```c
// ECS Light Component
typedef struct LightComponent {
    LightType type;             // DIRECTIONAL, POINT, SPOT
    union {
        DirectionalLight directional;
        PointLight point;
        SpotLight spot;
    } light_data;
    bool enabled;
    float animation_time;       // For animated lights
} LightComponent;

// Light system update function
void lighting_system_update(World* world, float dt);
```

### Task 11.6: Advanced Shadow Techniques
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `assets/shaders/advanced_shadow.frag.metal`

#### Acceptance Criteria
- [ ] Implement Variance Shadow Maps (VSM) for soft shadows
- [ ] Add Contact Hardening Shadows (PCSS) foundation
- [ ] Implement shadow map resolution scaling based on distance
- [ ] Add temporal shadow map stability (reduce flickering)
- [ ] Shadow LOD system for performance

### Task 11.7: Light Debug Visualization
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/ui.c`, `src/render_3d.c`

#### Acceptance Criteria
- [ ] Add light gizmo rendering (wireframe spheres/cones)
- [ ] Implement light influence range visualization
- [ ] Add shadow map debugging view
- [ ] Light performance metrics in debug UI
- [ ] Real-time light property adjustment

---

## Acceptance Criteria

### Functional Requirements
- [ ] Shadow mapping working for directional lights
- [ ] Point lights illuminate scenes realistically
- [ ] Spot lights create proper cone-shaped illumination
- [ ] Multiple light types can be active simultaneously
- [ ] Environmental lighting provides realistic ambient illumination

### Performance Requirements
- [ ] Maintain 45+ FPS with 8 shadow-casting lights
- [ ] Shadow map generation under 2ms per light
- [ ] Light culling reduces rendering cost for distant lights
- [ ] Memory usage for all shadow maps under 50MB

### Quality Requirements
- [ ] Soft shadow edges without major artifacts
- [ ] Smooth light attenuation gradients
- [ ] No shadow acne or peter panning
- [ ] Realistic material response to different light types

---

## Technical Risks

### High Risk
1. **Shadow Map Performance**: Multiple shadow maps are expensive
   - *Mitigation*: Implement LOD and culling systems
   - *Fallback*: Reduce shadow map count and resolution

2. **Memory Usage**: Large shadow maps consume GPU memory
   - *Mitigation*: Dynamic shadow map allocation
   - *Fallback*: Lower resolution shadow maps

### Medium Risk
1. **Shadow Artifacts**: Acne, peter panning, edge artifacts
   - *Mitigation*: Proper bias settings and PCF implementation
   - *Fallback*: Simpler shadow techniques

2. **IBL Performance**: Environment mapping calculations are expensive
   - *Mitigation*: Pre-compute reflection probes
   - *Fallback*: Simplified environment lighting

### Low Risk
1. **Light Culling Complexity**: Managing many dynamic lights
   - *Mitigation*: Spatial partitioning for lights
   - *Fallback*: Limit maximum active lights

---

## Testing Strategy

### Unit Tests
- [ ] Light attenuation calculations
- [ ] Shadow map coordinate transformations
- [ ] Light culling algorithms
- [ ] IBL coefficient calculations

### Visual Tests
- [ ] Shadow accuracy comparisons
- [ ] Light falloff visual verification
- [ ] Cross-platform shadow consistency
- [ ] Performance profiling with many lights

### Integration Tests
- [ ] Multiple light types in single scene
- [ ] Dynamic light creation and destruction
- [ ] Shadow map memory management
- [ ] Light animation systems

---

## Success Metrics

- **Visual Quality**: Realistic lighting and shadows comparable to AAA games
- **Performance**: 45+ FPS with full lighting pipeline active
- **Flexibility**: Easy addition of new light types and configurations
- **Stability**: No memory leaks or crashes during dynamic light changes
- **Scalability**: Performance scales gracefully with light count

---

## Sprint Review

### Definition of Done
- [ ] All tasks completed with comprehensive testing
- [ ] Advanced lighting working on all target platforms
- [ ] Documentation includes lighting setup guidelines
- [ ] Performance benchmarks meet requirements
- [ ] Code reviewed and optimization passes completed

### Demo Preparation
- [ ] Scene showcasing all light types working together
- [ ] Real-time shadow map debugging visualization
- [ ] Performance comparison with and without advanced lighting
- [ ] Dynamic light animation demonstration
