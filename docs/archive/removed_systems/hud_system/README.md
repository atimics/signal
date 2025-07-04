# Archived HUD System

**Date Archived**: July 3, 2025  
**Reason**: Nuklear dependency incompatible with MicroUI migration  
**Status**: Preserved for reference, not compiled  

## Files Archived

- `hud_system.c` (23,372 bytes) - Main HUD implementation
- `hud_system.h` (5,662 bytes) - HUD system interface  
- `ui_old.c` - Legacy UI system that used HUD

## What This System Provided

### Multi-Camera HUD Layouts
- **Cockpit Mode**: Full instrument panel for immersive view
- **Chase Near**: Minimal overlay for chase camera
- **Chase Far**: Simplified distance view

### HUD Components (15 total)
1. **Targeting Reticle** - Custom drawn crosshair with dynamic sizing
2. **Speedometer** - Gauge-style speed indicator
3. **Thrust Indicator** - Bar chart showing thruster output
4. **Instrument Panel** - Full cockpit dashboard
5. **Communications** - Message display system
6. **Trade Interface** - Commerce UI
7. **Waypoint Display** - Navigation markers
8. **Tactical Overlay** - Combat information
9. **Proximity Warnings** - Collision alerts
10. **Weapon Systems** - Armament status
11. **Exploration Scanner** - Discovery interface
12. **Shield Status** - Defense indicators
13. **Power Management** - Energy distribution
14. **Cargo Management** - Inventory display
15. **Ship Status** - Hull integrity and systems

### Technical Features
- **Custom Drawing**: Used Nuklear canvas API for complex visuals
- **Real-time Updates**: 60fps refresh of all components
- **Smooth Transitions**: Animated mode switching
- **Responsive Layout**: Dynamic sizing based on screen resolution
- **Component Opacity**: Fade effects for non-essential elements

## Why It Was Removed

1. **Nuklear Dependency**: Heavy reliance on `nk_window_get_canvas()` and complex drawing
2. **WASM Incompatibility**: Nuklear dependencies block WebAssembly builds
3. **Complexity**: 765+ lines of tightly coupled UI/rendering code
4. **Maintenance Burden**: Complex styling and layout management

## Future Replacement Strategy

A new, simpler HUD will be built using MicroUI with these principles:

### Design Philosophy
- **Text-Heavy**: Use labels and simple formatting instead of custom drawing
- **Window-Based**: Individual panels instead of overlay drawing
- **Minimal Graphics**: Basic rectangles and text only
- **Modular Design**: Toggle-able components

### Essential Features to Rebuild
- [ ] Simple speedometer (text-based)
- [ ] Basic targeting crosshair (simple overlay)  
- [ ] Flight status display (text labels)
- [ ] Debug information panel
- [ ] Essential warnings/alerts

### Advanced Features (Future)
- [ ] Mini-map or navigation display
- [ ] Equipment status
- [ ] Mission objectives
- [ ] Performance metrics overlay

## Code Statistics

| File | Lines | Key Dependencies |
|------|-------|------------------|
| `hud_system.c` | 765 | Nuklear canvas, nk_context |
| `hud_system.h` | 156 | Vector2, HUD component enums |
| `ui_old.c` | 196 | Sokol-Nuklear integration |

**Total**: 1,117 lines of HUD-related code removed

## Restoration Notes

If this system needs to be restored temporarily:

1. Copy files back to `src/`
2. Add `hud_system.c` to Makefile SOURCES
3. Update UI system to call HUD functions
4. Ensure Nuklear is properly linked

However, this would block WASM compatibility and should be avoided.

---

*This system represents significant engineering effort but was incompatible with the project's WASM goals. The essential functionality will be rebuilt in a simpler, more maintainable form using MicroUI.*
