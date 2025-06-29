# Sprint 10: PBR Materials & Advanced Lighting

**Duration**: 2 weeks  
**Priority**: Medium  
**Dependencies**: Sprint 09 (Texture System)  

## Sprint Goal
Implement Physically Based Rendering (PBR) materials with proper lighting calculations, enabling realistic material appearance with metallic, roughness, and normal mapping support.

## Current State
- ✅ Basic textures working (diffuse maps)
- ✅ Simple material colors applied
- ✅ UV mapping functional
- ❌ No PBR material properties
- ❌ Basic ambient lighting only
- ❌ No metallic/roughness workflow

## Target State
- ✅ Full PBR material system (albedo, metallic, roughness, normal, AO)
- ✅ Directional lighting with proper PBR calculations
- ✅ Multiple texture support per material
- ✅ Material property sliders in debug UI
- ✅ HDR tone mapping and gamma correction

---

## Research Request

**For Research AI**: Please investigate the following areas to ensure best practices for PBR material implementation with Sokol:

### PBR Theory and Implementation
1. **PBR Fundamentals**: Comprehensive overview of PBR theory, including the metallic-roughness workflow
2. **Shader Mathematics**: Detailed explanation of BRDF calculations, Fresnel equations, and proper PBR lighting formulas
3. **Material Properties**: Typical value ranges and best practices for albedo, metallic, roughness, and normal map usage

### Sokol-Specific Implementation
1. **Texture Management**: Optimal strategies for managing multiple textures per material in Sokol
2. **Uniform Buffer Layout**: Best practices for organizing PBR material properties in uniform buffers
3. **Shader Optimization**: Performance optimization techniques for PBR shaders on different GPU architectures

### Asset Pipeline
1. **Texture Formats**: Recommended texture formats and compression for PBR textures (sRGB vs linear, etc.)
2. **Normal Map Standards**: Proper normal map handling, including tangent space calculations
3. **Material Validation**: Techniques for validating material properties and detecting common errors

### Debug and Visualization
1. **PBR Debug Views**: Common debug visualization techniques for PBR materials (albedo-only, metallic-only, etc.)
2. **Material Editor UI**: Best practices for creating material property editors with Nuklear
3. **Real-time Preview**: Techniques for providing real-time material preview and feedback

Please provide shader code examples compatible with Sokol's shader system and specific recommendations for C99 implementation.

---

## Tasks

### Task 10.1: PBR Material Structure
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/assets.h`, `src/assets.c`

#### Acceptance Criteria
- [ ] Extend `Material` struct with PBR properties
- [ ] Add metallic, roughness, normal, AO texture slots
- [ ] Implement material property validation
- [ ] Add material constants (base color, metallic factor, roughness factor)
- [ ] Support for material templates and presets

#### Implementation Details
```c
// Extended Material structure
typedef struct Material {
    char name[64];
    
    // PBR Textures
    uint32_t albedo_texture_id;
    uint32_t metallic_texture_id;
    uint32_t roughness_texture_id;
    uint32_t normal_texture_id;
    uint32_t ao_texture_id;
    
    // PBR Constants
    float base_color[4];        // RGBA
    float metallic_factor;      // 0.0 = dielectric, 1.0 = metallic
    float roughness_factor;     // 0.0 = mirror, 1.0 = rough
    float normal_scale;         // Normal map intensity
    float ao_strength;          // Ambient occlusion strength
    
    // Rendering properties
    bool double_sided;
    float alpha_cutoff;
    MaterialBlendMode blend_mode;
} Material;
```

### Task 10.2: PBR Shader Implementation
**Estimated**: 3 days  
**Assignee**: Developer  
**Files**: `assets/shaders/pbr.vert.metal`, `assets/shaders/pbr.frag.metal`

#### Acceptance Criteria
- [ ] Implement Cook-Torrance BRDF model
- [ ] Add multiple texture sampling (albedo, metallic, roughness, normal)
- [ ] Implement normal mapping with tangent space calculations
- [ ] Add proper Fresnel calculations (Schlick approximation)
- [ ] Support for up to 4 directional lights

#### Implementation Details
```glsl
// PBR Fragment Shader Structure
struct PBRMaterial {
    float3 albedo;
    float metallic;
    float roughness;
    float3 normal;
    float ao;
};

// Cook-Torrance BRDF functions
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 FresnelSchlick(float cosTheta, float3 F0);
```

### Task 10.3: Lighting System Enhancement
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_lighting.c`, `src/render_lighting.h`

#### Acceptance Criteria
- [ ] Implement directional light structure and uniforms
- [ ] Add support for up to 8 dynamic lights
- [ ] Implement light culling for performance
- [ ] Add shadow mapping foundation (basic implementation)
- [ ] Light intensity and color controls

#### Implementation Details
```c
typedef struct DirectionalLight {
    float direction[3];
    float color[3];
    float intensity;
    bool cast_shadows;
    float shadow_bias;
} DirectionalLight;

typedef struct LightingUniforms {
    DirectionalLight directional_lights[8];
    uint32_t directional_light_count;
    float ambient_color[3];
    float ambient_intensity;
} LightingUniforms;
```

### Task 10.4: Multi-Texture Binding System
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Support binding up to 8 textures per material
- [ ] Implement texture slot management
- [ ] Add texture streaming for large materials
- [ ] Optimize texture binding performance
- [ ] Add texture format validation

### Task 10.5: HDR and Tone Mapping
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Implement HDR render target
- [ ] Add tone mapping pipeline (Reinhard, ACES, Filmic)
- [ ] Implement gamma correction
- [ ] Add exposure controls
- [ ] Performance optimization for mobile targets

### Task 10.6: Debug UI Integration
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/ui.c`

#### Acceptance Criteria
- [ ] Add material property editor panel
- [ ] Real-time PBR parameter adjustment
- [ ] Light direction and color controls
- [ ] Texture preview and swapping
- [ ] Performance metrics for lighting calculations

---

## Acceptance Criteria

### Functional Requirements
- [ ] All entities render with PBR materials
- [ ] Metallic and roughness maps display correctly
- [ ] Normal mapping provides surface detail
- [ ] Directional lighting responds properly to material properties
- [ ] Material switching works via debug UI

### Performance Requirements
- [ ] Maintain 60 FPS with 4 entities and 4 lights
- [ ] GPU memory usage stays under 100MB for materials
- [ ] Shader compilation time under 2 seconds
- [ ] Material switching has no visible frame drops

### Quality Requirements  
- [ ] Realistic metal and dielectric material appearance
- [ ] Smooth gradients in lighting transitions
- [ ] No visible texture streaming artifacts
- [ ] Proper gamma correction on all platforms

---

## Technical Risks

### High Risk
1. **Shader Complexity**: PBR shaders are computationally expensive
   - *Mitigation*: Implement LOD system for distant objects
   - *Fallback*: Simplified lighting model for low-end hardware

2. **Multi-texture Performance**: Binding many textures per material
   - *Mitigation*: Texture atlas creation for small textures
   - *Fallback*: Reduce texture count per material

### Medium Risk
1. **Normal Mapping Artifacts**: Tangent space calculation errors
   - *Mitigation*: Use pre-computed tangent vectors from mesh data
   - *Fallback*: World-space normal mapping

2. **HDR Performance**: HDR rendering overhead
   - *Mitigation*: Configurable HDR quality settings
   - *Fallback*: LDR rendering mode

### Low Risk
1. **Material Editor Complexity**: UI becoming overwhelming
   - *Mitigation*: Organize UI into collapsible sections
   - *Fallback*: Simplified parameter sets

---

## Testing Strategy

### Unit Tests
- [ ] Material property validation functions
- [ ] PBR calculation accuracy tests
- [ ] Texture binding and unbinding
- [ ] Light culling algorithms

### Visual Tests
- [ ] Material reference comparisons
- [ ] Lighting accuracy with known values
- [ ] Cross-platform rendering consistency
- [ ] Performance benchmarks on target hardware

### Integration Tests
- [ ] Scene loading with PBR materials
- [ ] Runtime material property changes
- [ ] Multiple light source interactions
- [ ] Memory usage under material streaming

---

## Success Metrics

- **Visual Quality**: Realistic material appearance comparable to reference PBR renderers
- **Performance**: 60 FPS maintained with full PBR pipeline active
- **Usability**: Material properties easily adjustable via debug UI
- **Stability**: No rendering artifacts or crashes during material changes
- **Memory**: Efficient texture and material memory management

---

## Sprint Review

### Definition of Done
- [ ] All tasks completed with passing tests
- [ ] PBR rendering working on all target platforms
- [ ] Documentation updated with PBR material guidelines
- [ ] Performance meets target requirements
- [ ] Code reviewed and merged to main branch

### Demo Preparation
- [ ] Scene with variety of PBR materials (metal, plastic, fabric)
- [ ] Real-time material property adjustment
- [ ] Side-by-side comparison with reference PBR renderer
- [ ] Performance metrics display during demo
