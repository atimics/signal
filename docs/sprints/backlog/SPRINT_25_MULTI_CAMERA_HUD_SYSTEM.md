# Sprint 23: Multi-Camera HUD System

## Vision Statement
Implement comprehensive camera modes with context-appropriate HUDs to support distinct gameplay phases: cockpit for social/trade, chase-near for canyon racing, and chase-far for space combat/exploration.

## Problem Statement
The current targeting reticle implementation is working but the game needs specialized HUDs for different camera modes to create distinct gameplay experiences:
- **Cockpit view**: Immersive instrument panel for social interactions and trading
- **Near chase**: Racing-focused HUD with speed indicators and targeting
- **Far chase**: Combat/exploration HUD with waypoints and tactical information

## Definition of Done
- [ ] Three distinct camera modes with appropriate HUD layouts
- [ ] Smooth transitions between camera modes
- [ ] Context-aware UI elements that appear/disappear based on camera mode
- [ ] Performance: No more than 1ms additional UI overhead per frame
- [ ] All modes integrate with existing targeting reticle system

## Technical Implementation Plan

### Phase 1: Camera Mode Architecture (Week 1)
- [x] Current targeting reticle working but needs non-blocking overlay
- [ ] Extend camera system to support HUD mode metadata
- [ ] Create camera mode enum and transition system
- [ ] Implement smooth camera switching with appropriate timing

### Phase 2: HUD Component System (Week 2)
- [ ] Design modular HUD component architecture
- [ ] Create base HUD components: speedometer, compass, targeting display
- [ ] Implement HUD visibility management per camera mode
- [ ] Add HUD positioning and scaling system

### Phase 3: Mode-Specific HUDs (Week 3)
#### Cockpit View HUD
- [ ] Full instrument panel with ship status indicators
- [ ] Communication interface mockup
- [ ] Trade/cargo interface mockup
- [ ] Navigation computer display

#### Near Chase HUD
- [ ] Speed/thrust indicator
- [ ] Current targeting reticle (already working)
- [ ] Proximity warning system
- [ ] Bank angle indicator for racing

#### Far Chase HUD
- [ ] Waypoint system with distance indicators
- [ ] Tactical overlay for contacts
- [ ] Weapon systems interface (mockup)
- [ ] Exploration scanner display

### Phase 4: Integration & Polish (Week 4)
- [ ] Integrate with existing flight controls
- [ ] Performance optimization
- [ ] Visual polish and animations
- [ ] User experience testing

## Current Status: In Progress (Day 1)

### Completed
- ✅ Targeting reticle system working
- ✅ Fixed reticle UI blocking other interface elements
- ✅ Basic camera mode switching exists in flight_test_scene.c

### Current Issues
- ⚠️ Reticle was blocking other UI elements (FIXED: changed from NK_WINDOW_BACKGROUND to NK_WINDOW_NO_INPUT)

### Next Actions
1. Review existing camera mode switching in flight_test_scene.c
2. Design HUD component architecture 
3. Create HUD manager system that coordinates with camera modes
4. Implement first cockpit HUD mockup

## Technical Architecture

### Camera Mode Integration
```c
typedef enum {
    CAMERA_MODE_COCKPIT = 0,     // First-person with full instrument panel
    CAMERA_MODE_CHASE_NEAR = 1,  // Close chase with racing HUD
    CAMERA_MODE_CHASE_FAR = 2,   // Far chase with tactical HUD
    CAMERA_MODE_COUNT = 3
} FlightCameraMode;
```

### HUD Component System
```c
typedef struct {
    bool visible;
    Vector2 position;
    Vector2 size;
    float opacity;
    void (*render_func)(struct nk_context* ctx, struct World* world);
} HUDComponent;

typedef struct {
    HUDComponent speedometer;
    HUDComponent targeting_reticle;
    HUDComponent proximity_warnings;
    HUDComponent instrument_panel;
    HUDComponent waypoint_display;
    HUDComponent tactical_overlay;
} HUDSystem;
```

### Data Flow
```
Input → Camera Mode Switch → HUD Manager → Mode-Specific HUD → Nuklear Rendering
                           ↓
                    Update HUD Components
                           ↓
                    Apply Mode-Specific Layout
```

## Sprint Context

### Previous Sprint Success
- Sprint 21: 98% complete - 6DOF flight mechanics fully functional
- Sprint 22: 40% complete - Advanced input processing with statistical filtering

### This Sprint's Place in Roadmap
This sprint creates the foundation for distinct gameplay experiences that will be essential for:
- **Derelict exploration**: Cockpit view for close-quarters navigation
- **Canyon racing**: Near chase view with racing-focused indicators  
- **Space combat**: Far chase view with tactical information
- **Social/trading**: Cockpit view with appropriate interfaces

### Risk Management
- **Risk**: Complex UI state management between modes
- **Mitigation**: Use simple visibility flags and modular components
- **Risk**: Performance impact of multiple HUD systems
- **Mitigation**: Lazy loading and efficient rendering patterns

## Success Metrics
- Camera mode transitions feel smooth and responsive
- Each mode provides appropriate visual information for its gameplay
- No regression in flight control responsiveness
- Foundation ready for future gameplay feature integration

---

**Sprint Start**: 2025-07-02  
**Expected Completion**: 2025-07-30  
**Sprint Lead**: Claude Code Assistant  
**Priority**: High (Critical for gameplay experience differentiation)