# Sprint 20: Advanced Lighting & Material System

**Sprint**: 20 - Advanced Lighting & Material System  
**Focus**: Proper lighting architecture, material properties, and visual effects  
**Date**: July 1, 2025  
**Priority**: High - Fix global glow effect issue from Sprint 19

## 🎯 Sprint Goals

Create a comprehensive lighting and material system that provides proper visual effects, fixes the global glow issue, and establishes a foundation for advanced rendering.

## 🚨 Critical Issue from Sprint 19

**Problem**: The glow effect implemented in Sprint 19 is being applied to ALL entities instead of just the logo cube.

**Root Cause**: The material detection logic in `render_3d.c` is incorrectly identifying all entities as the logo cube.

**Impact**: Every rendered object now has a golden glow effect, making the scene look unrealistic.

## 🏗️ Architecture Design

### 1. Material Property System
```c
// Enhanced material system with properties
struct Material {
    char name[64];
    char diffuse_texture[64];
    
    // Lighting properties
    Vector3 ambient_color;
    Vector3 diffuse_color;
    Vector3 specular_color;
    float shininess;
    
    // Special effects
    float emission_intensity;    // Glow/emission strength
    Vector3 emission_color;      // Glow color
    bool is_emissive;           // Enable emission effects
    
    // Rendering flags
    bool cast_shadows;
    bool receive_shadows;
    float transparency;
};
```

### 2. Lighting System Architecture
```c
// Advanced lighting system
struct LightingSystem {
    // Ambient lighting
    Vector3 ambient_color;
    float ambient_intensity;
    
    // Directional lights (sun/moon)
    struct DirectionalLight directional_lights[MAX_DIRECTIONAL_LIGHTS];
    uint32_t directional_light_count;
    
    // Point lights (lamps, fires)
    struct PointLight point_lights[MAX_POINT_LIGHTS];
    uint32_t point_light_count;
    
    // Spot lights (flashlights, spotlights)
    struct SpotLight spot_lights[MAX_SPOT_LIGHTS];
    uint32_t spot_light_count;
    
    // Global settings
    bool shadows_enabled;
    float shadow_bias;
    bool dynamic_lighting;
};
```

### 3. Shader Enhancement
```metal
// Enhanced fragment shader uniforms
struct fs_uniforms {
    float3 light_dir;
    float3 camera_pos;
    
    // Material properties
    float3 material_ambient;
    float3 material_diffuse;
    float3 material_specular;
    float material_shininess;
    
    // Emission/glow properties
    float3 emission_color;
    float emission_intensity;
    bool is_emissive;
    
    // Lighting settings
    float3 ambient_light;
    float ambient_intensity;
    
    float time; // For animated effects
};
```

## 📋 Sprint Tasks

### Phase 1: Material System Foundation
1. **Enhanced Material Structure**
   - [ ] Extend `Material` struct with lighting properties
   - [ ] Add emission properties for glow effects
   - [ ] Update material loading from files
   - [ ] Create material validation system

2. **Material Asset Pipeline**
   - [ ] Update `.mtl` file parser for new properties
   - [ ] Add material templates for common types
   - [ ] Create logo cube specific material
   - [ ] Update asset compilation pipeline

### Phase 2: Lighting System Redesign
1. **Core Lighting Framework**
   - [ ] Implement `LightingSystem` structure
   - [ ] Add directional light support
   - [ ] Add point light support
   - [ ] Add spot light support

2. **Lighting Integration**
   - [ ] Integrate lighting with ECS
   - [ ] Add light entities and components
   - [ ] Update scene loading for lights
   - [ ] Create lighting management API

### Phase 3: Shader Enhancement
1. **Advanced Fragment Shader**
   - [ ] Implement Phong/Blinn-Phong lighting model
   - [ ] Add proper emission/glow calculation
   - [ ] Support multiple light types
   - [ ] Add material property support

2. **Rendering Pipeline Update**
   - [ ] Update uniform passing system
   - [ ] Fix material detection logic
   - [ ] Add per-material rendering
   - [ ] Implement selective glow effects

### Phase 4: Visual Effects
1. **Selective Glow System**
   - [ ] Fix logo cube glow detection
   - [ ] Add material-based glow properties
   - [ ] Create pulsing/animated glow effects
   - [ ] Add glow intensity controls

2. **Scene Enhancement**
   - [ ] Update logo scene with proper materials
   - [ ] Add atmospheric lighting
   - [ ] Create visual showcase scenes
   - [ ] Add lighting transitions

### Phase 5: Testing & Polish
1. **Material Testing**
   - [ ] Create material property tests
   - [ ] Test glow effect isolation
   - [ ] Validate lighting calculations
   - [ ] Performance testing for complex lighting

2. **Visual Validation**
   - [ ] Logo cube glow works correctly
   - [ ] Other objects don't glow
   - [ ] Lighting looks realistic
   - [ ] Performance remains stable

## 🔧 Implementation Strategy

### 1. Immediate Fix (Day 1)
**Priority**: Fix the global glow issue
```c
// Quick fix in render_3d.c
bool is_logo_cube = (entity->id == 1); // First entity in logo scene
// OR
bool is_logo_cube = false; // Disable glow until proper material system
```

### 2. Material Property Integration (Days 2-3)
- Add emission properties to materials
- Update material loading and storage
- Create logo cube specific material file

### 3. Lighting System Implementation (Days 4-5)
- Implement proper lighting calculations
- Add multiple light type support
- Update shader with advanced lighting

### 4. Effect Refinement (Days 6-7)
- Perfect the glow effect for logo cube only
- Add atmospheric lighting
- Polish visual presentation

## 📁 Files to Create/Modify

### Core Systems
- `src/lighting.h` / `src/lighting.c` - New lighting system
- `src/material.h` / `src/material.c` - Enhanced material system
- `src/render_3d.c` - Fix glow detection, update uniforms
- `assets/shaders/advanced_lighting.frag.metal` - New shader

### Assets
- `assets/materials/logo_cube.mtl` - Logo-specific material
- `assets/materials/templates/` - Material templates
- `data/scenes/lighting_demo.txt` - Lighting showcase scene

### Tests
- `tests/unit/test_lighting.c` - Lighting system tests
- `tests/unit/test_materials.c` - Material system tests
- `tests/visual/lighting_validation.c` - Visual testing

## 🎯 Success Criteria

### Primary Goals
1. **Glow Fix**: Only logo cube glows, other objects render normally
2. **Material System**: Proper material properties loaded from assets
3. **Lighting Quality**: Realistic lighting with multiple light types
4. **Performance**: No performance regression from advanced lighting

### Visual Quality
1. **Logo Scene**: Beautiful glowing logo cube with proper materials
2. **Other Scenes**: Realistic lighting without unwanted effects
3. **Atmospheric**: Proper ambient and directional lighting
4. **Consistency**: Uniform lighting behavior across scenes

### Technical Quality
1. **Architecture**: Clean, extensible lighting system
2. **Materials**: Data-driven material properties
3. **Shaders**: Efficient, feature-rich fragment shaders
4. **Integration**: Seamless ECS and asset pipeline integration

## 🚀 Expected Outcomes

### Immediate Benefits
- **Fixed Visuals**: Logo scene looks professional with selective glow
- **Realistic Lighting**: Proper Phong lighting for all scenes
- **Material Control**: Artists can control visual properties via files
- **Performance**: Optimized lighting calculations

### Long-term Foundation
- **Advanced Effects**: HDR, bloom, shadows in future sprints
- **Artist Tools**: Material editor and lighting tools
- **Diverse Scenes**: Support for indoor, outdoor, space environments
- **Visual Polish**: Professional-quality rendering pipeline

---

**Sprint 20 will transform CGame from a functional engine into a visually compelling platform with professional lighting and material systems.**
