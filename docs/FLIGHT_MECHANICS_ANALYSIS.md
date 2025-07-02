# Flight Mechanics Analysis - Current State & Issues

**Date**: July 2, 2025  
**Status**: Investigation Complete  
**Priority**: High - Core gameplay experience

## 🎯 **Current Flight Mechanics State**

### ✅ **What Works Well**
1. **Comprehensive Input System**: Unified keyboard + gamepad abstraction with proper analog support
2. **Advanced Camera System**: Multiple camera modes (cockpit, chase near/far, overhead) with dynamic behavior
3. **Flight Test Scene**: Well-designed testing environment with obstacles and open plain
4. **Physics Integration**: Basic velocity/acceleration physics working correctly
5. **Visual Polish**: Camera shake, dynamic FOV, velocity-based effects
6. **Performance**: Smooth 60+ FPS with complex camera calculations

### 🚨 **Critical Issues Identified**

#### 1. **Physics System Problems**
**File**: `src/system/physics.c:29-41`
- **Ground Effect Disabled**: Racing mechanics commented out due to "causing player ship to fly away"
- **Over-Simplified Physics**: Basic acceleration/velocity only, no proper force dynamics
- **No 6DOF Control**: Missing pitch, yaw, roll for true spaceflight feel
- **Linear Drag Only**: No atmospheric or realistic drag models

```c
// DISABLED: Ground-effect racing mechanics for ships - causing player ship to fly away
// DISABLED: Ships get speed boost when flying close to ground (ground effect)
// SIMPLIFIED: Apply basic acceleration to velocity
// SIMPLIFIED: Apply basic drag
```

#### 2. **Flight Controls Limitations**
**Files**: `src/scripts/flight_test_scene.c:325-393`, `src/system/input.c`
- **No Rotation Control**: Ships can't pitch, yaw, or roll naturally
- **Unrealistic Movement**: Direct force application without proper physics
- **Missing Inertia**: Instant acceleration/deceleration feels arcade-like
- **No Flight Model**: Lacks atmospheric or space flight characteristics

#### 3. **Camera-Ship Disconnect**
**File**: `src/scripts/flight_test_scene.c:144-323`
- **Camera-Heavy Logic**: Most "flight feel" comes from camera effects, not ship physics
- **Artificial Enhancement**: Camera shake and FOV changes mask poor physics
- **Follow Behavior Issues**: Camera system more complex than flight physics

#### 4. **Limited Scene Integration**
- **Test Scene Only**: Flight mechanics only properly implemented in `flight_test`
- **Derelict Scene Problems**: Different physics behavior in navigation scenes
- **No Consistency**: Each scene implements its own flight variants

## 🔧 **Technical Analysis**

### **Physics System Architecture Issues**

```c
// Current (src/system/physics.c:44-47)
physics->velocity = vector3_add(physics->velocity, vector3_multiply(physics->acceleration, delta_time));
physics->velocity = vector3_multiply(physics->velocity, physics->drag);
transform->position = vector3_add(transform->position, vector3_multiply(physics->velocity, delta_time));
```

**Problems**:
- No force accumulation
- No mass consideration
- No angular dynamics
- Linear drag model insufficient for spaceflight

### **Input System Analysis**

```c
// Current (src/system/input.c:54-62)
if (keyboard_state[INPUT_ACTION_THRUST_FORWARD]) current_input.thrust += 1.0f;
if (keyboard_state[INPUT_ACTION_STRAFE_RIGHT]) current_input.strafe += 1.0f;
if (keyboard_state[INPUT_ACTION_MANEUVER_UP]) current_input.vertical += 1.0f;
```

**Missing**:
- Pitch/yaw/roll controls
- Analog thrust curves
- Flight mode switching (atmospheric vs space)
- Thruster vectoring

### **Camera System Over-Engineering**

The camera system in `flight_test_scene.c` is doing most of the "flight feel" work:
- Velocity-based lag (lines 232-251)
- Speed-based distance adjustment (lines 254-269)
- Camera shake for movement sensation (lines 277-306)
- Dynamic FOV for speed sensation (lines 296-305)

**Problem**: The ship physics are so basic that camera tricks provide the flight experience.

## 🚀 **Required Improvements for Sprint 21**

### **Priority 1: Core Physics Overhaul**

1. **6DOF Physics System**
   - Proper force accumulation
   - Angular velocity and acceleration
   - Mass and moment of inertia
   - Thrust vectoring

2. **Realistic Flight Model**
   - Atmospheric vs vacuum physics
   - Proper drag calculations
   - Thruster mechanics
   - Velocity-dependent controls

3. **Ship Control System**
   - Pitch, yaw, roll controls
   - Thruster strength profiles
   - Flight assistance systems
   - Manual vs assisted modes

### **Priority 2: Enhanced Input**

1. **Full 6DOF Input Mapping**
   - Rotation controls (mouse or right stick)
   - Thrust vectoring
   - Flight mode switching

2. **Flight Characteristics**
   - Different ship types with unique handling
   - Thruster power curves
   - Control response profiles

### **Priority 3: Unified Implementation**

1. **Consistent Physics Across Scenes**
   - Same flight model in all scenes
   - Scene-specific environmental effects
   - Proper physics parameter configuration

2. **Ship Differentiation**
   - Different ships with unique flight characteristics
   - Upgrade systems for handling
   - Material/design affecting physics

## 🎮 **User Experience Issues**

### **Current Flight Feel Problems**

1. **Lacks Spaceflight Authenticity**
   - Feels like sliding blocks, not spaceships
   - No sense of mass or momentum
   - Missing rotational dynamics

2. **Camera Dependency**
   - Flight excitement comes from camera, not physics
   - Inconsistent when camera changes
   - Masks underlying physics problems

3. **Control Limitations**
   - Can't properly orient ship
   - No realistic maneuvering
   - Arcade-like instead of simulation-quality

### **Target Flight Experience**

1. **Authentic Spaceflight**
   - Ships feel like they have mass and momentum
   - Proper rotation and thrust vectoring
   - Realistic inertia and control authority

2. **Skill-Based Controls**
   - Learning curve for ship mastery
   - Different techniques for different situations
   - Rewarding precision flying

3. **Visceral Sensation**
   - Physics-driven excitement, not camera tricks
   - Natural movement feedback
   - Compelling flight dynamics

## 📋 **Sprint 21 Focus Areas**

### **Core Implementation Tasks**

1. **Physics System Rewrite**: Implement proper 6DOF physics with force accumulation
2. **Ship Control Overhaul**: Add pitch/yaw/roll controls with proper thruster mechanics
3. **Flight Model Design**: Create atmospheric vs space flight characteristics
4. **Input Enhancement**: Expand input system for full ship control
5. **Scene Integration**: Apply consistent flight physics across all scenes

### **Quality Targets**

- **Authentic Feel**: Ships behave like real spacecraft with mass and momentum
- **Responsive Controls**: Immediate feedback with realistic control authority
- **Consistent Experience**: Same high-quality flight physics in all scenes
- **Performance Maintained**: 60+ FPS with enhanced physics calculations

---

**The current flight mechanics are functional but limited by over-simplified physics and missing rotational dynamics. Sprint 21 should focus on implementing authentic spaceflight physics with proper 6DOF control to create a compelling and realistic flight experience.**