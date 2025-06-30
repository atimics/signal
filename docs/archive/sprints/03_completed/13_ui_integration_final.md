# Sprint 13: UI Finalization & Engine Polish

**Duration**: 2 weeks
**Priority**: High
**Dependencies**: Sprint 12 (Decal System Implementation)

## Sprint Goal
Finalize the UI system by building out a comprehensive suite of debug and development tools using the existing Nuklear integration. Polish the overall rendering pipeline and ensure production-ready stability and performance across all implemented features.

## Current State
- ✅ Complete 3D rendering pipeline with PBR materials
- ✅ Advanced lighting with shadow mapping
- ✅ Decal system for surface details
- ✅ **Basic Nuklear UI integrated and functional**
- ❌ Debug UI is minimal and lacks advanced tools (inspector, profiler, etc.)
- ❌ No comprehensive performance profiling UI

## Target State
- ✅ **Advanced debug and development UI interface built on Nuklear**
- ✅ Comprehensive performance profiling and debugging tools
- ✅ Production-ready rendering pipeline
- ✅ Full feature parity with original SDL implementation achieved

---

## Research Request

**For Research AI**: Please investigate the following areas to ensure best practices for *enhancing* the existing Nuklear integration:

### Advanced UI Patterns
1. **Complex Data Binding**: Best practices for binding complex game state (e.g., nested entity components) to Nuklear widgets.
2. **Real-time Graphing**: Techniques for creating efficient real-time performance graphs (e.g., for frame time) in Nuklear.
3. **Custom Widget Design**: Feasibility and examples of creating custom, reusable UI widgets with Nuklear's API.

### Performance Optimization
1. **UI Rendering Costs**: Benchmark data on Nuklear's performance characteristics and optimization techniques for complex UIs.
2. **Memory Footprint**: Typical memory usage patterns and strategies for minimizing UI memory overhead as the UI grows in complexity.
3. **Draw Call Optimization**: Techniques for batching Nuklear's draw commands to minimize GPU state changes, especially with multiple complex windows.

### API Usage Patterns
1. **Modern Nuklear API**: Current best practices for Nuklear 4.x+ API usage for advanced features like trees, groups, and popups.
2. **State Management**: Best practices for managing the state of a complex, multi-window debug UI.

Please provide code examples, performance benchmarks, and specific recommendations for the CGame engine's ECS architecture and C99 coding standards.

---

## Tasks

### Task 13.1: Enhance Core UI System
**Estimated**: 2 days
**Assignee**: Developer
**Files**: `src/ui.c`, `src/ui.h`

#### Acceptance Criteria
- [ ] Refactor UI state management into a more robust `NuklearState` struct.
- [ ] Implement a modern UI theme matching the project aesthetic.
- [ ] Ensure input handling correctly distinguishes between game and UI focus.
- [ ] Add top-level menu bar to toggle different UI panels.

#### Implementation Details
```c
// Enhanced Nuklear integration structure
typedef struct NuklearState {
    struct nk_context ctx;
    struct nk_buffer cmds;
    struct nk_draw_null_texture null_texture;
    bool initialized;
    
    // UI panel visibility state
    bool show_demo_window;
    bool show_performance_window;
    bool show_entity_inspector;
    bool show_material_editor;
    bool show_light_editor;
    bool show_decal_debug;
    
    // UI interaction state
    int selected_entity_id;
    int selected_material_id;
    int selected_light_id;
    
    // Performance tracking
    float frame_times[120];      // Last 120 frame times
    int frame_time_index;
    float avg_frame_time;
} NuklearState;
```

### Task 13.2: Advanced Debug UI Panels
**Estimated**: 3 days
**Assignee**: Developer
**Files**: `src/ui.c`

#### Acceptance Criteria
- [ ] Create comprehensive entity inspector with component editing.
- [ ] Implement material editor with real-time preview.
- [ ] Add lighting control panel with shadow map debugging.
- [ ] Create decal spawning and management interface.
- [ ] Add camera control and scene navigation tools.

#### Implementation Details
```c
// UI panel functions
void ui_render_entity_inspector(World* world, NuklearState* ui_state);
void ui_render_material_editor(World* world, NuklearState* ui_state);
void ui_render_light_editor(World* world, NuklearState* ui_state);
void ui_render_decal_debug(World* world, NuklearState* ui_state);
void ui_render_scene_hierarchy(World* world, NuklearState* ui_state);

// Entity Inspector Panel Example
void ui_render_entity_inspector(World* world, NuklearState* ui_state) {
    if (nk_begin(&ui_state->ctx, "Entity Inspector", nk_rect(50, 50, 300, 400),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_TITLE)) {
        
        // Entity selection list
        nk_layout_row_dynamic(&ui_state->ctx, 200, 1);
        if (nk_group_begin(&ui_state->ctx, "Entities", NK_WINDOW_BORDER)) {
            for (uint32_t i = 0; i < world->entity_count; i++) {
                Entity* entity = &world->entities[i];
                char label[128];
                snprintf(label, sizeof(label), "Entity %d", entity->id);
                if (nk_selectable_label(&ui_state->ctx, label, NK_TEXT_ALIGN_LEFT, ui_state->selected_entity_id == entity->id)) {
                    ui_state->selected_entity_id = entity->id;
                }
            }
            nk_group_end(&ui_state->ctx);
        }
        
        // Component editors for selected entity
        // ...
    }
    nk_end(&ui_state->ctx);
}
```

### Task 13.3: Performance Profiling System
**Estimated**: 2 days
**Assignee**: Developer
**Files**: `src/ui.c`, `src/systems.c`

#### Acceptance Criteria
- [ ] Implement frame time graphing and statistics in a dedicated UI panel.
- [ ] Add GPU timing for rendering passes (if supported by the backend).
- [ ] Create memory usage monitoring for core systems.
- [ ] Add system performance breakdown (ECS, Physics, Rendering).

#### Implementation Details
```c
// Performance Profiler Panel
void ui_render_performance_profiler(const SystemScheduler* scheduler, NuklearState* ui_state) {
    if (nk_begin(&ui_state->ctx, "Performance", nk_rect(400, 50, 350, 300),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_TITLE)) {
        
        // Frame time graph
        nk_layout_row_dynamic(&ui_state->ctx, 100, 1);
        nk_plot_function(&ui_state->ctx, NK_CHART_LINES, ui_state->frame_times, 120, 0);

        // Stats
        nk_layout_row_dynamic(&ui_state->ctx, 20, 1);
        nk_labelf(&ui_state->ctx, NK_TEXT_LEFT, "Avg FPS: %.2f", 1.0f / ui_state->avg_frame_time);
        
        // System breakdown
        for (int i = 0; i < SYSTEM_COUNT; i++) {
            nk_labelf(&ui_state->ctx, NK_TEXT_LEFT, "%s: %.2f ms", scheduler->systems[i].name, scheduler->system_times[i] * 1000);
        }
    }
    nk_end(&ui_state->ctx);
}
```

### Task 13.4: Rendering Pipeline Polish
**Estimated**: 2 days
**Assignee**: Developer
**Files**: `src/render_*.c`

#### Acceptance Criteria
- [ ] Implement frustum culling for entities and lights.
- [ ] Add automatic LOD system for distant objects.
- [ ] Optimize shader uniform updates.
- [ ] Add rendering statistics collection (draw calls, triangles) and display them in the UI.

### Task 13.5: Asset Pipeline Integration
**Estimated**: 2 days
**Assignee**: Developer
**Files**: `src/assets.c`, `tools/asset_compiler.py`

#### Acceptance Criteria
- [ ] Implement asset hot-reloading for development (e.g., textures, materials).
- [ ] Add asset validation and error reporting to the UI.

---
(The rest of the original Sprint 13 plan remains largely relevant and does not need to be changed)
