# Sprint 07: 3D Camera System Implementation

**Duration**: 1 week  
**Priority**: High  
**Dependencies**: Sprint 06 (Cross-platform compilation), Sokol foundation  

## Sprint Goal
Implement a fully functional 3D camera system with view/projection matrices, replacing the simple triangle rendering with proper 3D perspective rendering.

## Current State
- ✅ Sokol rendering pipeline working
- ✅ Basic triangle rendering confirmed
- ✅ Asset loading system in place
- ❌ Camera system uses identity matrices (no 3D perspective)
- ❌ No camera movement or switching functionality

## Target State
- ✅ 3D camera with view/projection matrices
- ✅ Camera switching (1-9 keys) working
- ✅ Proper 3D perspective rendering
- ✅ Camera movement and rotation
- ✅ Integration with ECS camera components

---

## Research Request

**For Research AI**: Please investigate the following areas to ensure best practices for 3D camera system implementation:

### 3D Camera Mathematics
1. **Projection Matrices**: Detailed explanation and implementation of perspective and orthographic projection matrices
2. **View Matrix Construction**: Best practices for constructing view matrices from position, target, and up vectors
3. **Quaternion Rotations**: Using quaternions for smooth camera rotations and avoiding gimbal lock
4. **Camera Frustum**: Proper frustum calculation for culling and intersection testing

### Camera Control Modes
1. **FPS Camera**: Implementation details for first-person camera controls with mouse look
2. **Orbit Camera**: Techniques for implementing smooth orbit cameras around a target point
3. **Free Camera**: Best practices for implementing free-flying debug cameras
4. **Camera Transitions**: Smooth interpolation techniques for transitioning between camera modes

### Performance and Optimization
1. **View Frustum Culling**: Efficient algorithms for culling objects outside the camera frustum
2. **LOD Systems**: Integration of camera distance with level-of-detail systems
3. **Occlusion Culling**: Basic techniques for culling objects hidden behind other objects
4. **Camera-Relative Rendering**: Techniques for maintaining precision with large world coordinates

### ECS Integration
1. **Camera Components**: Best practices for representing cameras as ECS components
2. **Multiple Cameras**: Managing multiple active cameras and camera switching
3. **Camera Systems**: Optimal organization of camera update systems within ECS architecture
4. **Input Integration**: Clean integration of input handling with camera systems

Please provide specific matrix calculation code examples compatible with C99 and recommendations for integration with Sokol's rendering pipeline.

---

## Tasks

### Task 7.1: Enhanced Matrix Mathematics
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/render_3d.c`

#### Acceptance Criteria
- [ ] Implement `mat4_perspective()` function for projection matrix
- [ ] Implement `mat4_lookat()` function for view matrix
- [ ] Implement `mat4_multiply()` for matrix combination
- [ ] Add matrix utility functions (identity, transpose, inverse)
- [ ] Unit tests for matrix functions (optional)

#### Implementation Details
```c
// Perspective projection matrix
void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov * 0.5f * M_PI / 180.0f);
    // Implementation details...
}

// Look-at view matrix
void mat4_lookat(float* m, Vector3 eye, Vector3 target, Vector3 up) {
    Vector3 forward = vector3_normalize(vector3_subtract(target, eye));
    // Implementation details...
}
```

#### Testing
- Verify matrices produce expected transformations
- Test with known input/output values
- Visual verification with simple geometry

---

### Task 7.2: Camera Component Enhancement
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/core.h`, `src/render_camera.h`, `src/render_camera.c`

#### Acceptance Criteria
- [ ] Enhance Camera component with 3D properties
- [ ] Add cached matrix storage for performance
- [ ] Implement camera update system
- [ ] Add camera validation and bounds checking

#### Implementation Details
```c
// Enhanced camera structure
struct Camera {
    // Position and orientation
    Vector3 position;
    Vector3 target;
    Vector3 up;
    
    // Projection parameters
    float fov;           // Field of view in degrees
    float aspect_ratio;  // Width/height ratio
    float near_plane;    // Near clipping plane (0.1f)
    float far_plane;     // Far clipping plane (1000.0f)
    
    // Cached matrices (updated when camera changes)
    float view_matrix[16];
    float projection_matrix[16];
    float view_projection_matrix[16];
    bool matrices_dirty;
    
    // Camera behavior (existing)
    uint8_t behavior;
    Vector3 velocity;
    float speed;
    float sensitivity;
};
```

#### Testing
- Camera component creation and initialization
- Matrix cache invalidation when position changes
- Integration with existing camera behavior system

---

### Task 7.3: Camera System Update Implementation
**Estimated**: 1.5 days  
**Assignee**: Developer  
**Files**: `src/systems.c`, `src/render_camera.c`

#### Acceptance Criteria
- [ ] Update camera system to calculate 3D matrices
- [ ] Implement camera switching functionality
- [ ] Add smooth camera transitions (optional)
- [ ] Integrate with input system (1-9 keys)

#### Implementation Details
```c
void camera_system_update(struct World* world, float delta_time) {
    // Find active camera entity
    EntityID active_camera = world_get_active_camera(world);
    if (active_camera == INVALID_ENTITY) return;
    
    struct Camera* camera = entity_get_camera(world, active_camera);
    if (!camera) return;
    
    // Update matrices if camera changed
    if (camera->matrices_dirty) {
        camera_update_matrices(camera);
        camera->matrices_dirty = false;
    }
    
    // Apply camera behavior (existing logic)
    camera_update_behavior(world, active_camera, delta_time);
}

void camera_update_matrices(struct Camera* camera) {
    // Calculate view matrix
    mat4_lookat(camera->view_matrix, camera->position, camera->target, camera->up);
    
    // Calculate projection matrix
    mat4_perspective(camera->projection_matrix, camera->fov, camera->aspect_ratio, 
                     camera->near_plane, camera->far_plane);
    
    // Combine view and projection
    mat4_multiply(camera->view_projection_matrix, camera->projection_matrix, camera->view_matrix);
}
```

#### Testing
- Multiple cameras in scene
- Camera switching with 1-9 keys
- Matrix updates when camera moves
- Performance impact measurement

---

### Task 7.4: Render Integration
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/render_3d.c`, `src/main.c`

#### Acceptance Criteria
- [ ] Replace identity MVP matrix with camera matrices
- [ ] Update render config to use active camera
- [ ] Implement proper 3D rendering pipeline
- [ ] Maintain backward compatibility with existing rendering

#### Implementation Details
```c
void render_frame(struct World* world, RenderConfig* config, EntityID player_id, float delta_time) {
    // Get active camera
    EntityID active_camera = world_get_active_camera(world);
    struct Camera* camera = entity_get_camera(world, active_camera);
    
    if (!camera) {
        // Fallback to identity matrices
        vs_uniforms_t uniforms;
        mat4_identity(uniforms.mvp);
    } else {
        // Use camera matrices
        vs_uniforms_t uniforms;
        memcpy(uniforms.mvp, camera->view_projection_matrix, sizeof(uniforms.mvp));
    }
    
    // Apply uniforms and render
    sg_range uniform_data = SG_RANGE(uniforms);
    sg_apply_uniforms(0, &uniform_data);
    sg_draw(0, 3, 1);
}
```

#### Testing
- Triangle renders at different sizes based on camera distance
- Camera movement affects rendering
- Multiple camera views show different perspectives

---

### Task 7.5: Input Integration & Camera Controls
**Estimated**: 1 day  
**Assignee**: Developer  
**Files**: `src/main.c`, event handling

#### Acceptance Criteria
- [ ] Camera switching with 1-9 keys working
- [ ] Debug camera information display
- [ ] Proper camera validation and error handling
- [ ] Camera state persistence

#### Implementation Details
```c
static void event(const sapp_event* ev) {
    switch (ev->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (ev->key_code >= SAPP_KEYCODE_1 && ev->key_code <= SAPP_KEYCODE_9) {
                int camera_index = ev->key_code - SAPP_KEYCODE_1;
                
                // Find camera by index and switch to it
                if (switch_to_camera(camera_index)) {
                    printf("📹 Switched to camera %d\n", camera_index + 1);
                    
                    // Update aspect ratio for new camera
                    update_camera_aspect_ratio(1280.0f / 720.0f);
                }
            }
            break;
    }
}
```

#### Testing
- All 9 camera slots functional
- Error handling for invalid camera indices
- Smooth camera transitions
- Debug output shows current camera information

---

## Definition of Done

### Technical Requirements
- [ ] All matrix math functions implemented and tested
- [ ] Camera component fully enhanced with 3D properties
- [ ] Camera system integrated with ECS
- [ ] Camera switching working with 1-9 keys
- [ ] No rendering errors or validation failures

### Visual Requirements
- [ ] Triangle renders with proper 3D perspective
- [ ] Camera movement changes triangle appearance
- [ ] Different cameras show different views
- [ ] No visual artifacts or glitches

### Performance Requirements
- [ ] Frame rate maintains 60+ FPS
- [ ] Camera matrix calculations optimized
- [ ] No memory leaks in camera system
- [ ] Minimal impact on existing systems

### Documentation Requirements
- [ ] Code comments explaining matrix calculations
- [ ] Camera system architecture documented
- [ ] Input controls documented for users
- [ ] Sprint retrospective completed

---

## Risk Assessment

### High Risk
- **Matrix math complexity**: Incorrect calculations could break rendering
  - *Mitigation*: Use reference implementations, thorough testing
- **Performance impact**: Matrix calculations every frame
  - *Mitigation*: Cache matrices, only update when needed

### Medium Risk
- **ECS integration**: Camera system changes affect other systems
  - *Mitigation*: Incremental integration, extensive testing
- **Input conflicts**: Camera switching conflicts with other controls
  - *Mitigation*: Clear input mapping, conflict resolution

### Low Risk
- **Visual artifacts**: Minor rendering issues during development
  - *Mitigation*: Progressive enhancement, fallback options

---

## Testing Strategy

### Unit Tests
- Matrix math functions with known inputs/outputs
- Camera component creation and updates
- Matrix cache invalidation logic

### Integration Tests
- Camera system with ECS
- Input system integration
- Rendering pipeline integration

### Visual Tests
- Camera movement affects rendering correctly
- Multiple cameras show different perspectives
- No visual artifacts or glitches

### Performance Tests
- Frame rate impact measurement
- Memory usage monitoring
- Matrix calculation optimization verification

---

## Sprint Retrospective

### What Went Well
- [ ] Matrix math implementation
- [ ] ECS integration smoothness
- [ ] Camera switching functionality
- [ ] Performance optimization

### What Could Be Improved
- [ ] Testing coverage
- [ ] Documentation completeness
- [ ] Code organization
- [ ] Error handling

### Action Items for Next Sprint
- [ ] Lessons learned from camera implementation
- [ ] Performance optimizations identified
- [ ] Technical debt to address
- [ ] Foundation improvements for mesh rendering

---

**Next Sprint**: [Sprint 08: Mesh Rendering with Transforms](08_mesh_rendering_transforms.md)
