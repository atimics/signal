# Sprint 21 Implementation Plan - Entity-Agnostic Flight Mechanics

**Date**: July 2, 2025  
**Status**: ✅ READY FOR IMPLEMENTATION  
**Architecture**: Approved capability composition design

## 🎯 **Implementation Overview**

**Approved Architecture**: Entity-agnostic flight mechanics through component composition
- **Physics**: Universal 6DOF capability for ANY entity
- **ThrusterSystem**: Propulsion for entities that need it
- **ControlAuthority**: Input processing for controlled entities

## 📋 **Sub-Sprint Breakdown**

### **Sprint 21A: Enhanced Physics Foundation** (3-4 days)
**Goal**: Universal 6DOF physics for any entity

#### **Tasks**:
1. **Enhanced Physics Component** (Day 1)
   - [ ] Add angular velocity/acceleration to Physics struct
   - [ ] Add force/torque accumulator fields
   - [ ] Add moment of inertia and environmental context
   - [ ] Update core.h with enhanced Physics definition

2. **Physics System Rewrite** (Day 2-3)
   - [ ] Implement force/torque accumulation and application
   - [ ] Add 6DOF integration with optional angular dynamics
   - [ ] Performance LOD for angular physics (near vs far entities)
   - [ ] Environmental physics support (space/atmosphere)

3. **Testing & Validation** (Day 3-4)
   - [ ] Unit tests for 6DOF physics calculations
   - [ ] Performance tests for angular dynamics scaling
   - [ ] Integration tests with existing linear physics

#### **Success Criteria**:
- [ ] ANY entity can have 6DOF physics via `has_6dof` flag
- [ ] Performance maintains 60+ FPS with 100+ physics entities
- [ ] Angular dynamics work correctly for rotation and momentum

### **Sprint 21B: Thruster System** (3-4 days)
**Goal**: Universal propulsion capability

#### **Tasks**:
1. **ThrusterSystem Component** (Day 1)
   - [ ] Create ThrusterSystem struct with force/torque capabilities
   - [ ] Add component to ECS system
   - [ ] Define thruster characteristics and efficiency

2. **Thruster Force Application** (Day 2-3)
   - [ ] Convert thrust commands to forces/torques
   - [ ] Environmental efficiency calculations
   - [ ] Thruster response time and characteristics
   - [ ] Integration with Physics system

3. **Entity Integration** (Day 3-4)
   - [ ] Add ThrusterSystem to player ships
   - [ ] Add to AI ships and controllable entities
   - [ ] Test with projectiles and guided objects

#### **Success Criteria**:
- [ ] ANY entity can have thrusters independent of control
- [ ] Realistic thrust characteristics and efficiency
- [ ] Proper force/torque application to Physics system

### **Sprint 21C: Control Authority** (3-4 days)
**Goal**: Flexible input processing and control

#### **Tasks**:
1. **ControlAuthority Component** (Day 1)
   - [ ] Create ControlAuthority struct
   - [ ] Link controlled entities to input sources
   - [ ] Flight assistance and stability systems

2. **Enhanced Input Processing** (Day 2)
   - [ ] Add pitch/yaw/roll to InputState
   - [ ] Mouse integration for rotation control
   - [ ] Multiple control scheme support

3. **Control Systems** (Day 3-4)
   - [ ] Process input to thruster commands
   - [ ] Flight assistance algorithms
   - [ ] Manual vs assisted flight modes
   - [ ] Control sensitivity and response curves

#### **Success Criteria**:
- [ ] Clean separation between control and physics/thrusters
- [ ] Multiple input schemes (gamepad, keyboard+mouse)
- [ ] Flight assistance systems working correctly

### **Sprint 21D: Integration & Polish** (2-3 days)
**Goal**: Complete system integration and optimization

#### **Tasks**:
1. **Scene Integration** (Day 1)
   - [ ] Update flight_test_scene.c with new systems
   - [ ] Apply consistent physics across all scenes
   - [ ] Remove camera-based flight feel hacks

2. **Performance Optimization** (Day 2)
   - [ ] Profile 6DOF physics performance
   - [ ] Optimize component memory layout
   - [ ] LOD system tuning for angular dynamics

3. **Polish & Testing** (Day 2-3)
   - [ ] Comprehensive integration testing
   - [ ] Fine-tune control responsiveness
   - [ ] Validate entity-agnostic design across entity types

#### **Success Criteria**:
- [ ] Consistent flight experience across all scenes
- [ ] 60+ FPS maintained with enhanced physics
- [ ] Entity-agnostic design validated with multiple entity types

## 🔧 **Technical Implementation Details**

### **Enhanced Physics Component**
```c
struct Physics {
    // Linear dynamics
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 force_accumulator;    // NEW: Sum forces per frame
    float mass;
    
    // Angular dynamics (NEW)
    Vector3 angular_velocity;     // Rotation rates
    Vector3 angular_acceleration;
    Vector3 torque_accumulator;   // Sum torques per frame
    Vector3 moment_of_inertia;    // Per-axis resistance
    
    // Configuration
    float drag_linear;
    float drag_angular;
    bool kinematic;
    bool has_6dof;               // Enable angular dynamics
    
    // Environment
    enum { PHYSICS_SPACE, PHYSICS_ATMOSPHERE } environment;
};
```

### **ThrusterSystem Component**
```c
struct ThrusterSystem {
    // Thruster capabilities
    Vector3 max_linear_force;     // Max thrust per axis
    Vector3 max_angular_torque;   // Max torque per axis
    
    // Current state
    Vector3 current_linear_thrust;
    Vector3 current_angular_thrust;
    
    // Characteristics
    float thrust_response_time;
    float atmosphere_efficiency;
    float vacuum_efficiency;
    bool thrusters_enabled;
};
```

### **ControlAuthority Component**
```c
struct ControlAuthority {
    EntityID controlled_by;       // Who controls this entity
    
    // Control settings
    float control_sensitivity;
    float stability_assist;
    bool flight_assist_enabled;
    
    // Input state
    Vector3 input_linear;         // Processed linear input
    Vector3 input_angular;        // Processed angular input
    float input_boost;
    bool input_brake;
    
    enum { CONTROL_MANUAL, CONTROL_ASSISTED, CONTROL_AUTOPILOT } control_mode;
};
```

## 📊 **System Integration Flow**

```
Input System
    ↓ (raw input)
Control System
    ↓ (thrust commands)
Thruster System
    ↓ (forces/torques)
Physics System
    ↓ (position/rotation updates)
Transform Component
    ↓ (rendering)
Render System
```

## 🎯 **Entity Composition Examples**

### **Player Ship**
- Physics (6DOF enabled)
- ThrusterSystem (full capabilities)
- ControlAuthority (player input)
- Transform, Renderable, Player components

### **AI Ship**
- Physics (6DOF enabled)
- ThrusterSystem (full capabilities)  
- ControlAuthority (AI input)
- Transform, Renderable, AI components

### **Debris**
- Physics (6DOF enabled, no thrusters)
- Transform, Renderable components
- Realistic physics without control

### **Guided Missile**
- Physics (6DOF enabled)
- ThrusterSystem (limited capabilities)
- Transform, Renderable components
- Self-guided, no external control

## ✅ **Quality Gates**

### **Each Sub-Sprint Must Meet**:
- [ ] Clean compilation with zero warnings
- [ ] All unit tests passing
- [ ] Performance benchmarks maintained
- [ ] Architecture principles followed

### **Final Sprint Success**:
- [ ] Entity-agnostic design validated
- [ ] 60+ FPS with enhanced physics
- [ ] Multiple entity types working correctly
- [ ] Control schemes functioning properly

## 📁 **Files to Create/Modify**

### **Core Systems**
- `src/core.h` - Enhanced Physics, new ThrusterSystem, ControlAuthority
- `src/system/physics.c/.h` - 6DOF physics implementation
- `src/system/thrusters.c/.h` - NEW thruster force application
- `src/system/control.c/.h` - NEW input processing and flight assistance

### **Integration**
- `src/scripts/flight_test_scene.c` - Updated to use new systems
- `data/templates/entities.txt` - Component configurations for entity types

### **Testing**
- `tests/unit/test_physics_6dof.c` - 6DOF physics validation
- `tests/unit/test_thrusters.c` - Thruster system tests
- `tests/unit/test_control.c` - Control authority tests
- `tests/integration/test_flight_complete.c` - Full system validation

---

**🚀 Sprint 21 is ready for implementation with approved entity-agnostic architecture. Each sub-sprint has clear boundaries and success criteria.**