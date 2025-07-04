# Nuklear to MicroUI Migration Report

**Date**: July 3, 2025  
**Project**: CGame Engine  
**Status**: 🚧 In Progress  
**Priority**: High (WASM Compatibility Required)

## Executive Summary

The CGame project is migrating from Nuklear to MicroUI as the primary UI framework to achieve WebAssembly (WASM) compatibility and reduce dependencies. This migration is critical for the project's goal of cross-platform deployment.

**Current State**: Partial migration completed, dual-system temporarily active  
**Estimated Completion**: 1-2 days  
**Risk Level**: Medium (existing functionality preserved)

## Migration Rationale

### Why Move Away from Nuklear?

1. **WASM Compatibility Issues**: Nuklear has dependencies that complicate WASM builds
2. **Size Concerns**: Nuklear is a large library (~27,000 lines in nuklear.h)
3. **Dependencies**: Requires specific configurations and external dependencies
4. **Build Complexity**: Multiple preprocessor definitions required

### Why Choose MicroUI?

1. **Pure C**: No external dependencies, WASM-friendly
2. **Lightweight**: Minimal footprint (~297 lines in microui.h)
3. **Simple API**: Immediate mode GUI with clean, minimal API
4. **Proven**: Used in other WASM-compatible projects
5. **MIT License**: Compatible licensing

## Current Architecture Analysis

### Nuklear Usage Patterns Identified

#### 1. **Core UI Rendering** (`src/ui.c`, `src/ui_old.c`)
```c
// Current Nuklear pattern
struct nk_context* ctx = snk_new_frame();
scene_ui_render(ctx, current_scene, world, scheduler, delta_time);
snk_render(sapp_width(), sapp_height());
```

#### 2. **Scene UI System** (`src/ui_scene.c`)
- 424 lines of Nuklear-based scene UI
- Uses `nk_begin()`, `nk_layout_row_dynamic()`, `nk_button_label()`
- Handles scene selection, debug panels, logo display

#### 3. **UI Components** (`src/ui_components.c`)
- Modular widget system: SceneListWidget, ConfigWidget, PerformanceWidget
- 324 lines of Nuklear-dependent code
- Extensive use of `nk_*` functions for layout and rendering

#### 4. **HUD System** (`src/hud_system.c`)
- Multi-camera HUD with different layouts
- Uses Nuklear canvas drawing for custom UI elements
- 15+ HUD components with complex rendering

#### 5. **Graphics Integration** (`src/graphics_api.c`)
- Sokol-Nuklear integration
- Event handling via `snk_handle_event()`
- Font and image management

### MicroUI Implementation Started

#### 1. **MicroUI Wrapper** (`src/ui_microui.c`) ✅
- 397 lines of MicroUI integration
- Sokol graphics backend implemented
- Basic rendering pipeline functional

#### 2. **Scene Adapter** (`src/ui_microui_adapter.c`) ✅
- 154 lines of scene UI adaptation
- Basic navigation menu implemented
- Function signature compatibility layer

#### 3. **Header Integration** (`src/ui_microui.h`) ✅
- Complete API wrapper
- Context management
- Input handling interface

## Migration Status by Component

### ✅ Completed
- [x] MicroUI library integration
- [x] Basic Sokol rendering backend
- [x] Core wrapper API
- [x] Scene navigation adapter
- [x] Build system compatibility

### 🚧 In Progress  
- [ ] UI scene system migration (partially done)
- [ ] Function signature conflicts resolution
- [ ] Dual-system linking errors

### ⏳ Pending
- [ ] UI components widget migration
- [ ] HUD system migration
- [ ] Graphics API cleanup
- [ ] Test system updates
- [ ] Input handling migration
- [ ] Font system migration

### ❌ Not Started
- [ ] Performance profiling
- [ ] WASM build validation
- [ ] Documentation updates

## Technical Challenges Identified

### 1. **API Differences**

| Feature | Nuklear | MicroUI | Migration Complexity |
|---------|---------|---------|---------------------|
| Window Creation | `nk_begin()` | `mu_begin_window()` | Low |
| Layout | `nk_layout_row_dynamic()` | `mu_layout_row()` | Medium |
| Buttons | `nk_button_label()` | `mu_button()` | Low |
| Text Input | `nk_edit_string()` | `mu_textbox()` | Medium |
| Custom Drawing | `nk_window_get_canvas()` | Manual command buffer | High |
| Tree Views | `nk_tree_push_hashed()` | `mu_treenode()` | Medium |
| Color Management | Complex style system | Simple color enum | High |

### 2. **Current Build Issues**

#### Duplicate Symbol Error
```
duplicate symbol '_scene_ui_handle_event' in:
    build/ui_microui_adapter.o
    build/ui_scene.o
```

**Resolution**: Choose between implementations or use conditional compilation.

#### Sokol API Compatibility
- Fixed shader descriptor format (newer Sokol API)
- Updated buffer usage flags
- Corrected uniform application calls

### 3. **Missing MicroUI Features**

| Nuklear Feature | MicroUI Equivalent | Workaround Needed |
|-----------------|-------------------|-------------------|
| Arrow key constants | Not available | Input handling redesign |
| Complex styling | Basic theming | Custom color management |
| Rich text formatting | Plain text only | HTML-like markup system |
| Docking | Not available | Manual layout management |

## Migration Strategy

### Phase 1: Core System (Current)
1. ✅ Establish MicroUI wrapper
2. ✅ Create rendering backend
3. 🚧 Resolve build conflicts
4. ⏳ Basic scene navigation

### Phase 2: Component Migration
1. Port SceneListWidget to MicroUI
2. Port ConfigWidget and PerformanceWidget
3. Update EntityBrowserWidget
4. Create MicroUI utility functions

### Phase 3: HUD System
1. Analyze custom drawing requirements
2. Implement MicroUI command buffer extensions
3. Port targeting reticle and instruments
4. Test multi-camera layouts

### Phase 4: Cleanup & Optimization
1. Remove Nuklear dependencies
2. Update test stubs
3. Performance profiling
4. WASM build validation

## Code Impact Analysis

### Files Requiring Major Changes
- `src/ui_scene.c` (424 lines) - Complete rewrite
- `src/ui_components.c` (324 lines) - Widget API changes
- `src/hud_system.c` (custom drawing) - Command buffer approach
- `src/graphics_api.c` - Remove Nuklear integration

### Files Requiring Minor Changes
- `src/ui.c` - Update initialization calls
- `src/main.c` - Remove NK_IMPLEMENTATION
- Test files - Update UI stubs

### Files Unaffected
- Core ECS system
- Physics system
- Asset loading
- Scene management
- Input processing (except UI events)

## Risk Assessment

### Low Risk
- Basic UI widgets (buttons, labels, input fields)
- Layout systems
- Event handling

### Medium Risk
- Complex UI components (trees, groups)
- Custom styling and theming
- Performance differences

### High Risk
- Custom drawing and canvas operations
- HUD system visualization
- Integration with existing game systems

## Testing Strategy

### Unit Tests
- ✅ Basic UI widget stubs exist
- ⏳ MicroUI wrapper tests needed
- ⏳ Scene navigation tests

### Integration Tests
- ⏳ UI-game system interaction
- ⏳ Input event handling
- ⏳ Rendering pipeline validation

### WASM Compatibility Tests
- ⏳ WebAssembly build validation
- ⏳ Browser compatibility testing
- ⏳ Performance benchmarking

## Performance Considerations

### Expected Improvements
- Smaller binary size (MicroUI ~2KB vs Nuklear ~500KB)
- Faster compilation times
- Reduced memory footprint
- Better WASM performance

### Potential Regressions
- Custom drawing performance
- Text rendering quality
- Complex layout calculations

## Timeline & Milestones

### Immediate (Next 24 hours)
1. Fix duplicate symbol errors
2. Complete scene navigation migration
3. Basic UI components functional

### Short Term (2-3 days)
1. Complete widget system migration
2. HUD system basic functionality
3. Remove Nuklear dependencies

### Medium Term (1 week)
1. WASM build integration
2. Performance optimization
3. Documentation updates

## Recommendations

### Immediate Actions
1. **Resolve Build Conflicts**: Choose single implementation for `scene_ui_handle_event`
2. **Complete Scene Migration**: Finish `ui_scene.c` conversion to MicroUI
3. **Test Basic Functionality**: Ensure scene navigation works

### Strategic Decisions
1. **Dual System Approach**: Temporarily maintain both systems for fallback
2. **Incremental Migration**: Migrate one UI component at a time
3. **Custom Extensions**: Build MicroUI extensions for complex HUD needs

### Quality Assurance
1. **Regression Testing**: Ensure no game functionality lost
2. **Visual Comparison**: Screenshot comparison between systems
3. **Performance Monitoring**: Track frame rates and memory usage

## Conclusion

The Nuklear to MicroUI migration is technically feasible and strategically important for WASM compatibility. The migration is approximately 40% complete with core infrastructure in place. The remaining work focuses on porting existing UI components and ensuring feature parity.

**Recommended Priority**: High - Continue migration to completion before adding new UI features.

**Success Criteria**:
- ✅ Clean WASM build
- ✅ All existing UI functionality preserved
- ✅ Performance maintained or improved
- ✅ Test coverage maintained

---

*This report will be updated as migration progresses. Next update planned for completion of basic scene navigation.*
