# Scene System Overhaul - Completed

## Overview
Successfully overhauled the CGame engine's scene system to align with the FTL navigation design and the game's core vision of exploring "The Graveyard" star system.

## Completed Changes

### Scene File Structure
**Before:**
- `logo.txt` - Engine logo test
- `spaceport.txt` - Generic space station demo  
- `racing.txt` - Ground racing prototype
- `mesh_test.txt` - 3D model validation
- `camera_test.txt` - Camera system test
- `logo_test.txt` - Additional logo testing
- `scene_selector.txt` - Basic scene menu

**After:**
- `logo.txt` - System boot/validation sequence (unchanged)
- `navigation_menu.txt` - FTL Threadline Planner interface
- `system_overview.txt` - The Graveyard system-wide view with derelict nodes
- `slipstream_nav.txt` - FTL navigation testbed
- `derelict_alpha.txt` - Ancient Aethelian command ship exploration
- `derelict_beta.txt` - Deep-space archaeological site

### Dynamic Scene Discovery System
- **Replaced hardcoded scene loading** with dynamic directory scanning
- **Function:** `load_all_scene_templates()` in `data.c/h`
- **Benefits:** Automatically discovers new scene files without code changes
- **Uses:** POSIX `dirent.h` for cross-platform directory scanning

### Scene Transition Logic Updates
- **Logo scene**: Now transitions to `navigation_menu` after 8 seconds (instead of `spaceport_alpha`)
- **Scene states**: Updated logic in `scene_script.c` to handle new FTL navigation scenes
- **Menu integration**: `navigation_menu` excluded from scene selector (serves as main hub)

### UI System Enhancements
- **Dynamic scene listing**: UI automatically discovers and lists all available scenes
- **Lore-accurate descriptions**: Updated `get_scene_description()` with FTL navigation theme:
  - "System Overview - Sector-wide FTL navigation hub" 
  - "Slipstream Navigation - FTL threadline planning testbed"
  - "Derelict Alpha - Ancient station excavation site"
  - "Derelict Beta - Deep-space archaeological exploration"
  - "Threadline Planner - Primary FTL navigation interface"

### Code Architecture Improvements
- **Removed hardcoded dependencies** on specific scene names in systems
- **Improved error handling** for scene loading and discovery
- **Consistent naming** throughout codebase for new FTL navigation concept

## Technical Implementation

### Files Modified
- `src/data.c/h` - Added dynamic scene template loading
- `src/ui.c` - Updated scene discovery and descriptions  
- `src/scene_script.c` - Updated scene transition logic
- `src/scripts/logo_scene.c` - Fixed transition target
- `src/scripts/racing_scene.c` - Updated navigation references
- `data/scenes/*` - Renamed, created, and removed scene files as needed

### Key Functions Added
```c
void load_all_scene_templates(struct World* world);  // Dynamic scene discovery
```

### Scene Flow
```
Logo Scene (8s auto-transition)
    ↓
Navigation Menu (FTL Threadline Planner)
    ↓ (User selection)
System Overview / Slipstream Nav / Derelict Alpha / Derelict Beta
```

## Alignment with Game Vision

### FTL Navigation Theme
- **The Graveyard**: Star system filled with ancient Aethelian derelicts
- **FTL Threadlines**: Navigation routes between major derelict sites
- **Exploration Loop**: Plan routes → Navigate slipstreams → Explore derelicts → Return to plan

### Lore Integration
- **Aethelian Derelicts**: Ancient alien ark-ships serve as navigation nodes
- **Archaeological Theme**: Each derelict site offers unique exploration opportunities
- **Risk/Reward**: Slipstream navigation involves planning and risk assessment

## Next Steps (Future Sprints)

### FTL Navigation Implementation
- [ ] Visual threadline planner interface
- [ ] Route risk calculation and feedback
- [ ] Hop insertion and route optimization
- [ ] Real-time slipstream navigation mechanics

### Scene Graph System
- [ ] Hierarchical scene management as per research docs
- [ ] Scene culling and optimization
- [ ] Multi-level scene loading (system → derelict → interior)

### Enhanced Content
- [ ] Detailed derelict interior scenes
- [ ] Multiple system environments
- [ ] Dynamic hazard and discovery generation

## Validation
- ✅ All code compiles without errors
- ✅ Dynamic scene discovery working
- ✅ UI properly lists all new scenes
- ✅ Logo → Navigation Menu transition functioning
- ✅ Scene descriptions updated with lore-accurate content
- ✅ No hardcoded scene dependencies remaining

The scene system is now fully aligned with the FTL navigation design and ready for the next phase of development focusing on the actual FTL navigation mechanics and expanded content.
