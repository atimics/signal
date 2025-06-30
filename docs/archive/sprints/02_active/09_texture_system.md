# Sprint 09: Texture System Implementation

**Duration**: 2 weeks  
**Priority**: High  
**Dependencies**: Sprint 08 (Mesh Rendering with Transforms)  

## Sprint Goal
Implement complete texture loading and GPU binding system, enabling meshes to display with proper textures instead of solid colors, bringing visual fidelity closer to production quality.

## Current State
- ✅ Meshes rendering with solid material colors
- ✅ Texture loading to CPU (stb_image integration)
- ✅ Texture metadata and asset system
- ❌ Textures not loaded to GPU (`sg_image` creation)
- ❌ Texture binding in render pipeline
- ❌ UV coordinate mapping in shaders

## Target State
- ✅ All entity textures loaded to GPU as `sg_image`
- ✅ Texture binding in render pipeline working
- ✅ UV mapping displaying textures correctly on meshes
- ✅ Texture filtering and sampling configured
- ✅ Multi-texture support foundation (diffuse + normal maps)

---

## Research Request

**For Research AI**: Please investigate the following areas to ensure best practices for texture system implementation with Sokol:

### Texture Management
1. **Texture Formats**: Comprehensive overview of modern texture formats (BC1-7, ASTC, ETC2) and their use cases
2. **Mipmap Generation**: Best practices for mipmap generation and filtering techniques
3. **Texture Compression**: Platform-specific texture compression strategies and quality trade-offs
4. **Texture Arrays**: Implementation of texture arrays and texture atlases for batching

### Asset Pipeline Integration
1. **Asset Loading**: Efficient techniques for loading and processing texture data
2. **Format Conversion**: Runtime and offline texture format conversion strategies
3. **Streaming**: Techniques for streaming large textures and handling memory constraints
4. **Validation**: Methods for validating texture data and detecting corruption

### Performance Optimization
1. **Texture Caching**: Cache-friendly strategies for texture access and management
2. **GPU Memory Management**: Optimal allocation patterns for GPU texture memory
3. **Binding Optimization**: Minimizing texture binding state changes during rendering
4. **Batch Rendering**: Techniques for batching draw calls with different textures

### Sokol-Specific Implementation
1. **Sokol Image API**: Best practices for using Sokol's image creation and management APIs
2. **Sampler Objects**: Proper setup and management of texture samplers
3. **Resource Lifecycle**: Handling texture creation, updates, and destruction in Sokol
4. **Debug Visualization**: Techniques for debugging texture issues and visualizing texture data

Please provide code examples for efficient texture loading and management in C99, with specific focus on integration with the existing asset system and ECS architecture.

---

## Tasks

### Task 9.1: GPU Texture Loading System
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/assets.c`

#### Acceptance Criteria
- [ ] Implement `load_texture_to_gpu()` function
- [ ] Integrate with existing texture loading pipeline
- [ ] Add texture validation and error handling
- [ ] Support common formats (PNG, JPG, TGA)
- [ ] Implement texture streaming for large textures

#### Implementation Details
```c
bool load_texture_to_gpu(Texture* texture) {
    if (texture->loaded && texture->sg_image.id != SG_INVALID_ID) {
        return true; // Already loaded
    }

    // Load image data using stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(texture->filepath, &width, &height, &channels, 4);
    
    if (!data) {
        printf("❌ Failed to load texture: %s (%s)\n", texture->filepath, stbi_failure_reason());
        return false;
    }

    // Create Sokol image
    texture->sg_image = sg_make_image(&(sg_image_desc){
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
        .data.subimage[0][0] = {
            .ptr = data,
            .size = width * height * 4
        },
        .label = texture->name
    });

    // Store dimensions
    texture->width = width;
    texture->height = height;
    
    // Cleanup
    stbi_image_free(data);
    
    // Validate creation
    sg_resource_state state = sg_query_image_state(texture->sg_image);
    texture->loaded = (state == SG_RESOURCESTATE_VALID);
    
    if (texture->loaded) {
        printf("✅ Loaded texture to GPU: %s (%dx%d)\n", texture->name, width, height);
    } else {
        printf("❌ Failed to create GPU texture: %s (state=%d)\n", texture->name, state);
    }
    
    return texture->loaded;
}

// Batch load all textures to GPU
bool load_all_textures_to_gpu(AssetRegistry* registry) {
    int success_count = 0;
    int total_count = registry->texture_count;
    
    for (uint32_t i = 0; i < registry->texture_count; i++) {
        if (load_texture_to_gpu(&registry->textures[i])) {
            success_count++;
        }
    }
    
    printf("🎨 Loaded %d/%d textures to GPU\n", success_count, total_count);
    return success_count == total_count;
}
```

#### Testing
- Individual texture loading verification
- Batch loading of all scene textures
- Error handling for missing/corrupt files
- Memory usage monitoring

---

### Task 9.2: Enhanced Shader System
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `assets/shaders/basic_3d_textured.vert.metal`, `assets/shaders/basic_3d_textured.frag.metal`

#### Acceptance Criteria
- [ ] Create textured version of shaders
- [ ] Implement proper UV coordinate handling
- [ ] Add texture sampling in fragment shader
- [ ] Support material color + texture blending
- [ ] Optimize shader performance

#### Implementation Details

**Vertex Shader** (`assets/shaders/basic_3d_textured.vert.metal`):
```metal
#include <metal_stdlib>
using namespace metal;

struct vs_uniforms {
    float4x4 mvp;
    float4x4 model;
    float3 view_pos;
};

struct vs_in {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texcoord [[attribute(2)]];
};

struct vs_out {
    float4 position [[position]];
    float3 world_pos;
    float3 normal;
    float2 texcoord;
};

vertex vs_out vs_main(vs_in in [[stage_in]], 
                      constant vs_uniforms& uniforms [[buffer(0)]]) {
    vs_out out;
    
    // Transform position
    float4 world_pos = uniforms.model * float4(in.position, 1.0);
    out.position = uniforms.mvp * float4(in.position, 1.0);
    out.world_pos = world_pos.xyz;
    
    // Transform normal (should use normal matrix, simplified for now)
    out.normal = normalize((uniforms.model * float4(in.normal, 0.0)).xyz);
    
    // Pass through texture coordinates
    out.texcoord = in.texcoord;
    
    return out;
}
```

**Fragment Shader** (`assets/shaders/basic_3d_textured.frag.metal`):
```metal
#include <metal_stdlib>
using namespace metal;

struct fs_in {
    float3 world_pos;
    float3 normal;
    float2 texcoord;
};

struct fs_uniforms {
    float4 diffuse_color;
    float3 light_direction;
    float3 light_color;
    float ambient_strength;
};

fragment float4 fs_main(fs_in in [[stage_in]], 
                       constant fs_uniforms& uniforms [[buffer(0)]],
                       texture2d<float> diffuse_texture [[texture(0)]],
                       sampler texture_sampler [[sampler(0)]]) {
    
    // Sample diffuse texture
    float4 texture_color = diffuse_texture.sample(texture_sampler, in.texcoord);
    
    // Combine with material color
    float4 base_color = texture_color * uniforms.diffuse_color;
    
    // Basic lighting calculation
    float3 normal = normalize(in.normal);
    float3 light_dir = normalize(-uniforms.light_direction);
    
    // Ambient component
    float3 ambient = uniforms.ambient_strength * uniforms.light_color;
    
    // Diffuse component
    float diff = max(dot(normal, light_dir), 0.0);
    float3 diffuse = diff * uniforms.light_color;
    
    // Combine lighting
    float3 result = (ambient + diffuse) * base_color.rgb;
    
    return float4(result, base_color.a);
}
```

#### Testing
- Shader compilation without errors
- UV coordinates display correctly
- Texture sampling produces expected results
- Material color blending works

---

### Task 9.3: Render Pipeline Texture Integration
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Update render pipeline to use textured shaders
- [ ] Implement texture binding in `render_entity_3d()`
- [ ] Add material-texture association
- [ ] Support fallback to default texture for missing textures
- [ ] Optimize texture state changes

#### Implementation Details
```c
// Enhanced uniform structure
typedef struct {
    float mvp[16];          // Model-View-Projection matrix
    float model[16];        // Model matrix for normal transformation
    float view_pos[3];      // Camera position
    float _pad1;
    
    // Material properties
    float diffuse_color[4]; // Base material color
    float light_direction[3]; // Directional light
    float _pad2;
    float light_color[3];   // Light color
    float ambient_strength; // Ambient light strength
} enhanced_uniforms_t;

void render_entity_3d(struct World* world, EntityID entity_id, RenderConfig* config) {
    // ... existing component retrieval ...

    // Get material texture
    Texture* texture = assets_get_texture(config->assets, material->texture_name);
    sg_image texture_image = render_state.default_texture;
    
    if (texture && texture->loaded && texture->sg_image.id != SG_INVALID_ID) {
        texture_image = texture->sg_image;
    } else {
        // Log missing texture once per material
        static char warned_materials[32][64] = {0};
        static int warn_count = 0;
        
        bool already_warned = false;
        for (int i = 0; i < warn_count; i++) {
            if (strcmp(warned_materials[i], material->name) == 0) {
                already_warned = true;
                break;
            }
        }
        
        if (!already_warned && warn_count < 32) {
            strncpy(warned_materials[warn_count], material->name, 63);
            warn_count++;
            printf("⚠️ Using default texture for material '%s' (texture '%s' not found)\n", 
                   material->name, material->texture_name);
        }
    }

    // ... existing matrix calculations ...

    // Apply enhanced bindings
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = mesh->sg_vertex_buffer,
        .index_buffer = mesh->sg_index_buffer,
        .images[0] = texture_image,
        .samplers[0] = render_state.sampler
    });

    // Upload enhanced uniforms
    enhanced_uniforms_t uniforms = {0};
    memcpy(uniforms.mvp, mvp_matrix, sizeof(uniforms.mvp));
    memcpy(uniforms.model, model_matrix, sizeof(uniforms.model));
    
    // Camera position (if available)
    if (camera) {
        uniforms.view_pos[0] = camera->position.x;
        uniforms.view_pos[1] = camera->position.y;
        uniforms.view_pos[2] = camera->position.z;
    }
    
    // Material properties
    uniforms.diffuse_color[0] = material->diffuse_color.x;
    uniforms.diffuse_color[1] = material->diffuse_color.y;
    uniforms.diffuse_color[2] = material->diffuse_color.z;
    uniforms.diffuse_color[3] = 1.0f;
    
    // Basic lighting setup
    uniforms.light_direction[0] = 0.3f;
    uniforms.light_direction[1] = -0.7f;
    uniforms.light_direction[2] = 0.2f;
    uniforms.light_color[0] = 1.0f;
    uniforms.light_color[1] = 1.0f;
    uniforms.light_color[2] = 1.0f;
    uniforms.ambient_strength = 0.3f;
    
    sg_range uniform_data = SG_RANGE(uniforms);
    sg_apply_uniforms(0, &uniform_data);

    // Draw mesh
    sg_draw(0, mesh->index_count, 1);
}
```

#### Testing
- Textures display correctly on meshes
- Material-texture associations work
- Default texture fallback functions
- Performance impact acceptable

---

### Task 9.4: Asset Loading Integration
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/assets.c`, `src/main.c`

#### Acceptance Criteria
- [ ] Integrate GPU texture loading into asset initialization
- [ ] Update asset loading pipeline to include GPU upload
- [ ] Add texture loading progress reporting
- [ ] Handle texture loading failures gracefully

#### Implementation Details
```c
// In assets_init() or after mesh loading
bool assets_initialize_gpu_resources(AssetRegistry* registry) {
    printf("🎨 Initializing GPU resources...\n");
    
    // Load all textures to GPU
    bool textures_ok = load_all_textures_to_gpu(registry);
    
    // Validate all mesh buffers are ready
    bool meshes_ok = true;
    for (uint32_t i = 0; i < registry->mesh_count; i++) {
        Mesh* mesh = &registry->meshes[i];
        sg_resource_state vb_state = sg_query_buffer_state(mesh->sg_vertex_buffer);
        sg_resource_state ib_state = sg_query_buffer_state(mesh->sg_index_buffer);
        
        if (vb_state != SG_RESOURCESTATE_VALID || ib_state != SG_RESOURCESTATE_VALID) {
            printf("❌ Invalid mesh buffers for %s\n", mesh->name);
            meshes_ok = false;
        }
    }
    
    printf("📦 GPU Resources: Textures=%s, Meshes=%s\n", 
           textures_ok ? "✅" : "❌", meshes_ok ? "✅" : "❌");
    
    return textures_ok && meshes_ok;
}

// Call during initialization
// In main.c after asset loading:
if (!assets_initialize_gpu_resources(&app_state.render_config.assets)) {
    printf("❌ Failed to initialize GPU resources\n");
    // Handle error appropriately
}
```

#### Testing
- All textures load successfully during startup
- Progress reporting provides useful information
- Error handling prevents crashes
- Resource validation catches issues early

---

### Task 9.5: Multi-Texture Foundation
**Estimated**: 2 days  
**Assignee**: Developer  
**Files**: `src/render_3d.c`, shader files

#### Acceptance Criteria
- [ ] Design multi-texture binding system
- [ ] Implement normal map support foundation
- [ ] Add texture slot management
- [ ] Prepare for PBR texture workflow
- [ ] Document texture binding architecture

#### Implementation Details
```c
// Enhanced texture binding system
typedef struct {
    sg_image diffuse;      // Base color texture
    sg_image normal;       // Normal map
    sg_image specular;     // Specular/metallic map
    sg_image emission;     // Emission map
} material_textures_t;

bool bind_material_textures(Material* material, AssetRegistry* assets, material_textures_t* out_textures) {
    // Default to white/default textures
    out_textures->diffuse = render_state.default_texture;
    out_textures->normal = render_state.default_normal_texture;
    out_textures->specular = render_state.default_white_texture;
    out_textures->emission = render_state.default_black_texture;
    
    // Bind diffuse texture
    if (strlen(material->diffuse_texture) > 0) {
        Texture* tex = assets_get_texture(assets, material->diffuse_texture);
        if (tex && tex->loaded) {
            out_textures->diffuse = tex->sg_image;
        }
    }
    
    // Bind normal map
    if (strlen(material->normal_texture) > 0) {
        Texture* tex = assets_get_texture(assets, material->normal_texture);
        if (tex && tex->loaded) {
            out_textures->normal = tex->sg_image;
        }
    }
    
    // Add specular and emission support similarly...
    
    return true;
}

// Usage in render_entity_3d():
material_textures_t material_textures;
bind_material_textures(material, config->assets, &material_textures);

sg_apply_bindings(&(sg_bindings){
    .vertex_buffers[0] = mesh->sg_vertex_buffer,
    .index_buffer = mesh->sg_index_buffer,
    .images = {
        [0] = material_textures.diffuse,
        [1] = material_textures.normal,
        [2] = material_textures.specular,
        [3] = material_textures.emission
    },
    .samplers[0] = render_state.sampler
});
```

#### Testing
- Multi-texture binding works correctly
- Default textures provide fallbacks
- System scales for additional texture types
- Performance impact minimal

---

## Definition of Done

### Technical Requirements
- [ ] All entity textures loaded to GPU successfully
- [ ] Texture binding in render pipeline functional
- [ ] Enhanced shaders compile and execute correctly
- [ ] Multi-texture foundation implemented
- [ ] No Sokol validation errors

### Visual Requirements
- [ ] Control Tower shows proper texture details
- [ ] Sun displays with appropriate solar texture
- [ ] Wedge Ships show hull texturing
- [ ] UV mapping displays textures without distortion
- [ ] Lighting interacts correctly with textured surfaces

### Performance Requirements
- [ ] Frame rate maintains 60+ FPS with textured rendering
- [ ] Texture loading time acceptable during startup
- [ ] Memory usage within reasonable bounds
- [ ] GPU texture streaming efficient

### Documentation Requirements
- [ ] Texture system architecture documented
- [ ] Shader pipeline explained
- [ ] Multi-texture binding system documented
- [ ] Sprint retrospective completed

---

## Risk Assessment

### High Risk
- **Texture memory usage**: Large textures could exhaust GPU memory
  - *Mitigation*: Texture streaming, compression, LOD
- **Shader complexity**: Advanced shaders could impact performance
  - *Mitigation*: Performance profiling, shader optimization

### Medium Risk
- **UV mapping issues**: Incorrect texture coordinates
  - *Mitigation*: UV validation tools, visual debugging
- **Texture format compatibility**: Platform-specific texture issues
  - *Mitigation*: Standard formats, comprehensive testing

### Low Risk
- **Texture quality**: Minor visual differences from expected
  - *Mitigation*: Asset validation, quality guidelines

---

## Testing Strategy

### Unit Tests
- Individual texture loading functions
- UV coordinate validation
- Shader compilation verification

### Integration Tests
- Complete texture pipeline end-to-end
- Multi-entity textured rendering
- Camera interaction with textured meshes

### Visual Tests
- All entities display textures correctly
- UV mapping produces expected results
- Lighting integrates well with textures

### Performance Tests
- Frame rate impact of textured vs untextured rendering
- Memory usage monitoring during texture loading
- GPU profiling for optimization opportunities

---

## Sprint Retrospective

### What Went Well
- [ ] Texture loading pipeline implementation
- [ ] Shader enhancement success
- [ ] Multi-texture foundation
- [ ] Integration with existing systems

### What Could Be Improved
- [ ] Performance optimization opportunities
- [ ] Error handling comprehensiveness
- [ ] Documentation completeness
- [ ] Testing coverage

### Action Items for Next Sprint
- [ ] Performance improvements for PBR materials
- [ ] Advanced lighting system preparation
- [ ] Texture streaming optimization
- [ ] Normal mapping implementation

---

**Previous Sprint**: [Sprint 08: Mesh Rendering with Transforms](08_mesh_rendering_transforms.md)  
**Next Sprint**: [Sprint 10: PBR Materials & Advanced Lighting](10_pbr_materials_lighting.md)
