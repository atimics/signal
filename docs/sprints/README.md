# CGame Sprint Backlog

## Current Sprint Status

### 🚨 ACTIVE: Sprint 21 - Ship Flight Mechanics Overhaul
**Status**: 95% Complete - Core bugs fixed, ready for human validation  
**Priority**: HIGH - Essential gameplay foundation  
**Fixed**: ✅ Thrust direction, ✅ Drag calculation, ✅ Angular stability  
**Pending**: Visual thrusters, Human validation testing

#### Quick Links
- [Sprint 21 Design Document](active/SPRINT_21_DESIGN.md)
- [Sprint 21 Findings & Fixes](active/SPRINT_21_FINDINGS_AND_FIXES.md)
- [Sprint 21 Implementation Guide](active/SPRINT_21_IMPLEMENTATION_GUIDE.md)
- [Sprint 21 Implementation Plan](active/SPRINT_21_IMPLEMENTATION_PLAN.md)
- [Best Practices Report](active/THRUST_AND_GAMEPAD_NAVIGATION_BEST_PRACTICES_REPORT.md)

#### Recent Fixes (July 2, 2025)
1. **Thrust Direction**: ✅ Implemented `quaternion_rotate_vector()` - forces now correctly transformed
2. **Drag Calculation**: ✅ Fixed formula from `vel * drag` to `vel * (1 - drag)`
3. **Angular Stability**: ✅ Added velocity clamping and proper damping
4. **Next Steps**: Fix visual thrusters, conduct human validation testing

## Sprint Overview

### What is Sprint 21?
Sprint 21 implements a complete overhaul of the ship flight mechanics system using an entity-agnostic component architecture. This allows ANY entity in the game to have flight capabilities through component composition.

### Core Components
1. **Physics Component** - Universal 6DOF physics for any entity
2. **ThrusterSystem Component** - Propulsion capability 
3. **ControlAuthority Component** - Input processing for controlled entities
4. **Transform Component** - Position and orientation

### Definition of Done
Subjective human approval of flight mechanics "feel" with target rating of 4.0+/5.0 across:
- Responsiveness
- Realism
- Fun Factor
- Precision

## Completed Sprints

### Sprint 20 - Entity Component System (ECS) Architecture
**Status**: ✅ COMPLETE  
**Achievement**: Implemented high-performance ECS foundation supporting 10,000+ entities at 60 FPS  
[View Sprint 20 Summary](completed/SPRINT_20_COMPLETE.md)

### Sprint 19 - Core Infrastructure
**Status**: ✅ COMPLETE  
**Achievement**: Established build system, testing framework, and core utilities  
[View Sprint 19 Summary](completed/SPRINT_19_COMPLETE.md)

## Upcoming Sprints (Backlog)

### Sprint 22 - Canyon Racing Prototype
**Priority**: HIGH  
**Dependencies**: Sprint 21 completion  
**Goals**:
- Implement canyon generation system
- Create racing checkpoints and timing
- Add environmental hazards
- Polish flight controls for racing gameplay

### Sprint 23 - Advanced Flight Mechanics
**Priority**: MEDIUM  
**Dependencies**: Sprint 21 completion  
**Goals**:
- A-Drive implementation (FTL travel)
- Fusion Torch mechanics
- Atmospheric flight physics
- Gravity well interactions

### Sprint 24 - Ship Systems & Customization
**Priority**: MEDIUM  
**Goals**:
- Power management system
- Weapon hardpoints
- Shield systems
- Ship upgrade mechanics

### Sprint 25 - Multiplayer Foundation
**Priority**: LOW  
**Goals**:
- Network architecture
- Physics synchronization
- Client prediction
- Server authority

## Sprint Workflow

1. **Planning**: Define sprint goals and success criteria
2. **Design**: Create architecture documents and technical design
3. **Implementation**: Build features following ECS patterns
4. **Testing**: Comprehensive unit and integration tests
5. **Validation**: Human testing for gameplay feel
6. **Documentation**: Update findings and best practices

## Key Project Conventions

- **Architecture**: Entity Component System (ECS)
- **Testing**: Comprehensive unit tests required (currently 375+ tests)
- **Performance**: Target 60+ FPS with 50+ active entities
- **Code Style**: Clean, modular components with single responsibility
- **Documentation**: Sprint docs track design, implementation, and findings

## How to Fix the Current Blocker

1. Implement `quaternion_rotate_vector()` in `src/core/core.c`
2. Update `src/system/thrusters.c:77` to transform forces
3. Run thrust direction tests
4. Validate fix with human testing

See [Sprint 21 Implementation Guide](active/SPRINT_21_IMPLEMENTATION_GUIDE.md) for detailed fix instructions.