# Sprint 10: PBR Materials & Advanced Lighting

**Duration**: 2 weeks
**Priority**: High
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

## Tasks

### Task 10.1: PBR Material Structure
**Estimated**: 2 days
**Files**: `src/assets.h`, `src/assets.c`

#### Acceptance Criteria
- [ ] Extend `Material` struct with PBR properties
- [ ] Add metallic, roughness, normal, AO texture slots
- [ ] Implement material property validation
- [ ] Add material constants (base color, metallic factor, roughness factor)
- [ ] Support for material templates and presets

### Task 10.2: PBR Shader Implementation
**Estimated**: 3 days
**Files**: `assets/shaders/pbr.vert.metal`, `assets/shaders/pbr.frag.metal`

#### Acceptance Criteria
- [ ] Implement Cook-Torrance BRDF model
- [ ] Add multiple texture sampling (albedo, metallic, roughness, normal)
- [ ] Implement normal mapping with tangent space calculations
- [ ] Add proper Fresnel calculations (Schlick approximation)
- [ ] Support for up to 4 directional lights

### Task 10.3: Lighting System Enhancement
**Estimated**: 2 days
**Files**: `src/render_lighting.c`, `src/render_lighting.h`

#### Acceptance Criteria
- [ ] Implement directional light structure and uniforms
- [ ] Add support for up to 8 dynamic lights
- [ ] Implement light culling for performance
- [ ] Add shadow mapping foundation (basic implementation)
- [ ] Light intensity and color controls

### Task 10.4: Multi-Texture Binding System
**Estimated**: 2 days
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Support binding up to 8 textures per material
- [ ] Implement texture slot management
- [ ] Add texture streaming for large materials
- [ ] Optimize texture binding performance
- [ ] Add texture format validation

### Task 10.5: HDR and Tone Mapping
**Estimated**: 2 days
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Implement HDR render target
- [ ] Add tone mapping pipeline (Reinhard, ACES, Filmic)
- [ ] Implement gamma correction
- [ ] Add exposure controls
- [ ] Performance optimization for mobile targets

### Task 10.6: Debug UI Integration
**Estimated**: 1 day
**Files**: `src/ui.c`

#### Acceptance Criteria
- [ ] Add material property editor panel
- [ ] Real-time PBR parameter adjustment
- [ ] Light direction and color controls
- [ ] Texture preview and swapping
- [ ] Performance metrics for lighting calculations
