# HUD System Removal Plan

**Sprint**: UI Framework Migration  
**Date**: July 3, 2025  
**Priority**: High (Blocking WASM compatibility)

## Rationale for HUD Removal

The current HUD system is heavily dependent on Nuklear's custom drawing API and complex styling system. Rather than attempting to port this complex system to MicroUI (which lacks equivalent functionality), we will remove it and rebuild a simpler, MicroUI-native HUD later.

## Current HUD System Analysis

### Files to Remove
- `src/hud_system.c` (765+ lines) - Main HUD implementation
- `src/hud_system.h` - HUD system interface
- `src/hud_system_stubs.c` - Test stubs

### Dependencies to Clean Up
- Nuklear canvas drawing (`nk_window_get_canvas`)
- Complex color management and styling
- Multi-camera HUD layout system
- Custom drawing primitives (lines, circles, rects)

### Features Being Removed
- **Targeting Reticle**: Custom circle/line drawing
- **Speedometer**: Custom gauge rendering
- **Thrust Indicator**: Bar charts and meters
- **Instrument Panel**: Complex multi-widget layout
- **Communications Panel**: Rich text display
- **Trade Interface**: Complex interaction widgets
- **Waypoint Display**: Custom path drawing
- **Tactical Overlay**: Real-time data visualization
- **Proximity Warnings**: Alert system UI
- **Weapon Systems**: Status displays
- **Exploration Scanner**: Data visualization

## Removal Strategy

### Phase 1: Disable HUD Integration
1. Remove HUD system initialization from main loop
2. Remove HUD includes from UI system
3. Comment out HUD rendering calls
4. Update build system to exclude HUD files

### Phase 2: Remove HUD Files
1. Delete `hud_system.c` and `hud_system.h`
2. Remove HUD test stubs
3. Clean up any remaining references
4. Update documentation

### Phase 3: Simplify UI System
1. Remove complex UI components that depend on Nuklear
2. Focus on basic scene navigation and debug UI
3. Ensure core game functionality remains intact

## Impact Assessment

### What Continues to Work
- ✅ Core game engine (physics, rendering, input)
- ✅ Scene navigation and management
- ✅ Basic debug UI (entity browser, performance metrics)
- ✅ Flight controls and camera system
- ✅ 3D rendering and lighting

### What Will Be Temporarily Lost
- ❌ Visual flight instruments
- ❌ Targeting displays
- ❌ Status indicators
- ❌ Real-time telemetry display
- ❌ Complex UI overlays

### What Can Be Rebuilt Later (MicroUI-Native)
- 🔄 Simple speedometer (text-based)
- 🔄 Basic status display (text labels)
- 🔄 Simplified targeting (crosshair overlay)
- 🔄 Essential flight information
- 🔄 Debug overlays

## Implementation Steps

### Step 1: Disable HUD Calls (Immediate)
```c
// In main game loop, comment out:
// hud_system_render(ctx, &state.world);
```

### Step 2: Remove Build Dependencies
- Update Makefile to exclude `hud_system.c`
- Remove from test builds
- Clean up object files

### Step 3: Future HUD Design (Post-Migration)
Design principles for new MicroUI-based HUD:
- **Text-Heavy**: Use labels and simple formatting
- **Window-Based**: Individual panels instead of overlay drawing
- **Minimal Graphics**: Basic rectangles and text only
- **Modular**: Individual components that can be toggled

## Testing Strategy

### Verification Steps
1. Ensure game launches without HUD
2. Verify scene navigation still works
3. Confirm flight controls remain functional
4. Test debug UI functionality
5. Validate build system changes

### Regression Prevention
- Core gameplay must remain unaffected
- Physics simulation continues normally
- Rendering pipeline operates correctly
- Input handling preserved

## Timeline

### Immediate (Today)
- [ ] Disable HUD system calls
- [ ] Remove from build system
- [ ] Test basic functionality

### Short Term (1-2 days)
- [ ] Remove HUD source files
- [ ] Clean up references
- [ ] Update documentation

### Future (Post UI Migration)
- [ ] Design simple MicroUI HUD
- [ ] Implement basic flight instruments
- [ ] Add essential status displays

## Success Criteria

✅ **Clean Build**: Game compiles without HUD system  
✅ **Functional Game**: Core gameplay preserved  
✅ **Stable UI**: Scene navigation and debug UI work  
✅ **WASM Ready**: No Nuklear dependencies blocking WASM build  

---

*This removal is temporary - a simpler, more maintainable HUD will be rebuilt using MicroUI after the migration is complete.*
