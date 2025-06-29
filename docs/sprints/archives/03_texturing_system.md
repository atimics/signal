# Sprint 03: Texturing System Overhaul

**Goal:** Transform the current procedural gradient texture system into a professional, artist-friendly texturing pipeline that supports proper UV mapping, material reuse, and high-quality visual assets.

**Period:** June-July 2025

## Current State Analysis:

**Working:** Basic texture loading, texture repository, UV generation, asset compilation pipeline
**Problems:** Random gradient textures, per-face UV mapping, asset pipeline overwrites source materials, no artistic control

## Key Objectives:

### 1. **Asset Pipeline Reorganization (P0)**
*   [ ] **Preserve Source Materials:** Modify asset compiler to copy existing MTL files instead of overwriting
*   [ ] **Clean Directory Structure:** Separate source art assets from generated build assets
*   [ ] **Source Asset Protection:** Prevent asset compiler from modifying files in `/assets/` directory
*   [ ] **Material Name Consistency:** Fix material name extraction from OBJ files vs generated names

**User Story:** As an artist, I want my hand-crafted materials to persist through asset compilation so I can iterate on textures without losing my work.

### 2. **UV Mapping System Redesign (P0)**
*   [ ] **Replace Per-Face UVs:** Eliminate the current spritesheet-per-face approach
*   [ ] **Implement Proper UV Unwrapping:** Use planar/cylindrical projection or integrate Blender Python API
*   [ ] **Coherent Surface Mapping:** Ensure textures appear as unified surfaces, not disconnected patches
*   [ ] **UV Coordinate Validation:** Add bounds checking and error detection for UV coordinates

**User Story:** As a player, I want ship textures to look like realistic metal hulls and surface details, not random colored patches.

### 3. **Semantic Texture Templates (P1)**
*   [ ] **Material-Based Color Coding:** Generate SVG templates with semantic colors (metal=gray, lights=blue, engines=orange)
*   [ ] **Part-Aware Generation:** Different colors for ship hulls, cockpits, engines, weapons, etc.
*   [ ] **Architectural Templates:** Separate color schemes for buildings, structures, natural objects
*   [ ] **Artist Guidance System:** Include UV seam guides and editing instructions in SVG templates

**User Story:** As an artist, I want texture templates that make sense for the object type so I can quickly create believable materials.

### 4. **Material Repository System (P1)**
*   [ ] **Decouple Materials from Meshes:** Create shared material library independent of specific meshes
*   [ ] **Material Reuse:** Allow multiple meshes to reference the same material
*   [ ] **Runtime Material Swapping:** Support changing materials without recompiling assets
*   [ ] **Material Inheritance:** Base materials with variant overrides (e.g., metal_hull_clean, metal_hull_damaged)

**User Story:** As a developer, I want to create material variants (clean/damaged, different factions) without duplicating texture work.

### 5. **Multi-Texture Material Support (P2)**
*   [ ] **Diffuse + Normal Maps:** Support base color and surface detail textures
*   [ ] **Specular/Roughness Maps:** Add reflectivity and material property control
*   [ ] **Emission Maps:** Support glowing elements (engines, lights, screens)
*   [ ] **Texture Channel Management:** Proper loading and binding of multiple texture types

**User Story:** As a player, I want to see detailed surface textures with proper lighting that make ships feel solid and realistic.

### 6. **Artist Workflow Tools (P2)**
*   [ ] **Real-Time Texture Reload:** Mesh viewer automatically updates when texture files change
*   [ ] **UV Layout Visualization:** Show UV unwrapping overlays for debugging texture placement
*   [ ] **Material Preview System:** Side-by-side comparison of material variants
*   [ ] **Texture Quality Controls:** Resolution settings, filtering options, compression choices

**User Story:** As an artist, I want immediate feedback when editing textures so I can see results without restarting the game.

## Technical Implementation Plan:

### Phase 1: Asset Pipeline Fixes (Week 1)
```python
# Asset Compiler Changes:
def compile_mesh(source_path, build_path):
    # 1. Copy existing MTL if present, don't generate
    if source_mtl_exists():
        copy_preserve_mtl()
    else:
        generate_semantic_template()
    
    # 2. Extract material name from OBJ usemtl directive
    material_name = extract_usemtl_from_obj()
    
    # 3. Use proper UV unwrapping instead of per-face cells
    uvs = unwrap_mesh_uvs(mesh)
```

### Phase 2: UV System Overhaul (Week 2)
```c
// UV Unwrapping Integration:
typedef struct {
    Vector2* uv_coordinates;    // Single UV layout for entire mesh
    int* uv_indices;           // UV indices matching vertex indices
    bool has_valid_uvs;        // UV validation flag
} UVData;

// Replace current spritesheet generation with proper unwrapping
UVData generate_mesh_uvs(Mesh* mesh);
```

### Phase 3: Material System Enhancement (Week 3)
```c
// Enhanced Material Structure:
typedef struct {
    char name[64];
    char diffuse_texture[64];   // Base color map
    char normal_texture[64];    // Surface detail map  
    char specular_texture[64];  // Reflectivity map
    char emission_texture[64];  // Glow/emission map
    Vector3 base_color;         // Tint color
    float roughness;            // Surface roughness
    float metallic;             // Metallic factor
    bool loaded;
} Material;

// Material Repository:
Material* materials_get_by_name(const char* name);
bool materials_load_library(const char* materials_dir);
```

### Phase 4: Rendering Integration (Week 4)
```c
// Multi-texture rendering support:
void render_mesh_multitex(Mesh* mesh, Transform* transform, 
                         Material* material, RenderConfig* config) {
    // Bind multiple textures for different material properties
    bind_texture_unit(0, material->diffuse_texture);
    bind_texture_unit(1, material->normal_texture);
    bind_texture_unit(2, material->specular_texture);
    
    // Enhanced shader support for multiple texture channels
    render_with_material_properties(mesh, transform, material);
}
```

## Success Criteria:

### Visual Quality Targets:
- [ ] Textures appear as coherent surfaces without visible seams
- [ ] Ship hulls look metallic, engines glow appropriately, cockpits appear glass-like
- [ ] Materials visually distinguish between object types and factions
- [ ] Texture resolution scales appropriately with object complexity

### Technical Performance Targets:
- [ ] Asset compilation completes in under 30 seconds
- [ ] Texture memory usage scales with visual detail (not fixed per mesh)
- [ ] Rendering maintains 60 FPS with textured geometry
- [ ] Material swapping works without recompiling assets

### Workflow Quality Targets:
- [ ] Artists can edit textures without programmer intervention
- [ ] Material changes persist through asset recompilation
- [ ] New texture creation takes under 10 minutes (template → final texture)
- [ ] UV layout errors are caught and reported clearly

## Risk Mitigation:

**Risk:** UV unwrapping complexity breaks existing meshes
**Mitigation:** Implement fallback to current system, gradual mesh-by-mesh conversion

**Risk:** Multi-texture system impacts performance
**Mitigation:** Implement as optional feature, measure performance impact before full rollout

**Risk:** Artist workflow tools increase development time
**Mitigation:** Prioritize asset pipeline fixes first, tools are nice-to-have

## Dependencies:

- **Blender Python API** (for advanced UV unwrapping) - Optional, fallback to planar projection
- **Enhanced SDL texture management** - Required for multi-texture support
- **File watching system** - For real-time texture reload tools

## Definition of Done:

1. Artists can create and edit textures that persist through asset compilation
2. Ship textures look like realistic materials (metal, glass, energy) instead of random gradients  
3. Materials can be reused across multiple meshes
4. UV mapping produces coherent surface textures without visible seams
5. Asset compilation preserves hand-crafted materials and generates semantic templates for missing textures
6. Texture quality visually distinguishes the game from a programming demo to a game prototype

This sprint transforms the texturing system from a technical proof-of-concept into a production-ready asset pipeline that enables high-quality visual content creation.
