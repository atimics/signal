# Sprint 26: Semi-Autonomous Flight Computer System

**Document ID**: SPRINT_26_FLIGHT_ASSIST  
**Date**: July 6, 2025  
**Author**: Development Team  
**Status**: Active

**Duration**: 2 weeks (14 days)  
**Start Date**: July 7, 2025  
**End Date**: July 20, 2025  
**Priority**: High - Core Gameplay Enhancement

## 🎯 Vision & Core Objectives

Build an intelligent flight assist system that extends CGame's existing flight controls with semi-autonomous capabilities. Transform "direct thruster control" into "intent-based flight" where players specify where to go and the ship calculates the optimal path.

**Key Success Metrics:**
- ✅ Extend existing `unified_flight_control` system
- ✅ Maintain 60+ FPS with 5+ autonomous ships
- ✅ <100ms input latency for flight assist responses  
- ✅ Zero regressions in existing flight functionality

---

## 🏗️ Architecture & Integration

### Build on Existing Systems

**Core Foundation** - All systems already exist:
- **Input System**: `src/component/unified_flight_control.h` - Ready for assisted mode
- **Physics System**: `src/system/physics.h` - 6DOF support complete
- **Thruster System**: `src/system/thrusters.h` - Force application ready
- **Control System**: `src/system/control.c` - Input processing pipeline

### Extend Existing Components

Instead of creating new systems, enhance existing ones:

1. **Extend UnifiedFlightControl** → Add `FLIGHT_CONTROL_ASSISTED` mode
2. **Enhance Control System** → Add intent interpretation layer  
3. **Extend Physics Integration** → Add assistance forces
4. **Enhance Input Processing** → Add target sphere calculation

---

## 📅 Week-by-Week Plan (Focused)

### **Week 1: Core Flight Assist (Days 1-7)**

#### **Day 1-2: Enhance UnifiedFlightControl Component**
**Files to Modify:**
- `src/component/unified_flight_control.h` - Add flight assist fields
- `src/component/unified_flight_control.c` - Add assist logic

**Key Additions:**
```c
// Add to UnifiedFlightControl struct:
typedef struct {
    // ...existing fields...
    
    // Flight Assist Data (Sprint 26)
    struct {
        bool enabled;
        Vector3 target_position;        // 3D target in world space
        Vector3 target_velocity;        // Desired velocity vector
        float sphere_radius;            // Distance scaling factor
        
        // Simple PD Controller (position + velocity)
        float kp_position;              // Proportional gain
        float kd_velocity;              // Derivative gain
        float responsiveness;           // Overall response scaling
        
        // Banking visual feedback
        float bank_angle;               // Current visual banking
        float target_bank_angle;        // Desired banking
    } flight_assist;
} UnifiedFlightControl;
```

#### **Day 3-4: Intent Extraction & Target Calculation**
**Files to Modify:**
- `src/system/control.c` - Add target sphere calculation
- `src/component/unified_flight_control.c` - Process intent

**Key Algorithm:**
```c
Vector3 calculate_flight_assist_target(const UnifiedFlightControl* control, 
                                      const Transform* transform,
                                      const InputState* input) {
    // Convert input to 3D vector in ship's local space
    Vector3 local_direction = {
        input->strafe,     // X: left/right
        input->vertical,   // Y: up/down  
        input->thrust      // Z: forward/back
    };
    
    // Normalize and scale by sphere radius
    Vector3 normalized = vector3_normalize(local_direction);
    Vector3 scaled = vector3_multiply(normalized, control->flight_assist.sphere_radius);
    
    // Transform to world space and add to current position
    Vector3 world_direction = quaternion_rotate_vector(transform->rotation, scaled);
    return vector3_add(transform->position, world_direction);
}
```

#### **Day 5-6: PD Controller Implementation**
**Files to Create:**
- `src/math/pd_controller.h` - Reusable PD controller
- `src/math/pd_controller.c` - Implementation

**Core PD Logic:**
```c
Vector3 pd_controller_update(float kp, float kd, 
                            Vector3 target, Vector3 current,
                            Vector3 current_velocity, float dt) {
    Vector3 position_error = vector3_subtract(target, current);
    Vector3 velocity_error = vector3_multiply(current_velocity, -1.0f); // Damping
    
    Vector3 p_term = vector3_multiply(position_error, kp);
    Vector3 d_term = vector3_multiply(velocity_error, kd);
    
    return vector3_add(p_term, d_term);
}
```

#### **Day 7: Integration & Basic Testing**
**Files to Modify:**
- `src/system/unified_control_system.c` - Update system
- `tests/systems/test_flight_assist.c` - New test file

---

### **Week 2: Polish & Advanced Features (Days 8-14)**

#### **Day 8-9: Banking & Visual Feedback**
**Files to Modify:**
- `src/system/unified_control_system.c` - Banking calculation
- `src/scripts/flight_test_scene.c` - Visual integration

**Banking Algorithm:**
```c
float calculate_banking_angle(Vector3 velocity, Vector3 lateral_acceleration) {
    float speed = vector3_length(velocity);
    if (speed < 1.0f) return 0.0f; // No banking at low speeds
    
    float lateral_g = vector3_length(lateral_acceleration) / 9.81f;
    float bank_angle = atan2f(lateral_g, 1.0f) * (180.0f / M_PI);
    
    return fmaxf(-45.0f, fminf(45.0f, bank_angle)); // Clamp to ±45°
}
```

#### **Day 10-11: Mode Switching & UI Integration**
**Files to Modify:**
- `src/services/input_service.h` - Add mode toggle action
- `src/component/unified_flight_control.c` - Mode switching
- Flight test scene UI for mode indicator

#### **Day 12-13: Performance Optimization & Multi-Ship Testing**
**Files to Modify:**
- `src/system/unified_control_system.c` - Performance optimizations
- `tests/integration/test_flight_integration.c` - Multi-ship tests

#### **Day 14: Final Polish & Documentation**
**Files to Create:**
- `docs/manual/FLIGHT_ASSIST.md` - User guide
- Update `docs/guides/COMPONENT_REFERENCE.md`

---

## 🔧 Implementation Details

### Core Data Structures

Extend existing `UnifiedFlightControl` instead of creating new components:

```c
// In src/component/unified_flight_control.h
typedef struct {
    // ...existing fields remain unchanged...
    
    // NEW: Flight Assist Extension (Sprint 26)
    struct FlightAssistData {
        bool enabled;                   // Flight assist on/off
        FlightAssistMode mode;          // BASIC, PRECISION, RACE
        
        // Target System
        Vector3 target_position;        // Current 3D target
        Vector3 last_target;            // For velocity estimation
        float sphere_radius_base;       // Base radius (50m)
        float sphere_radius_current;    // Current radius (with boost/brake)
        
        // Control Parameters  
        float kp_position;              // Position gain (2.0)
        float kd_velocity;              // Velocity damping (0.5)
        float max_acceleration;         // Physics limit (30 m/s²)
        float responsiveness;           // User preference (0.5-1.0)
        
        // Visual Feedback
        float bank_angle;               // Current banking (-45° to +45°)
        float bank_rate;                // Banking change rate
        
        // Performance Tracking
        float computation_time_ms;      // For optimization
        uint32_t update_count;          // Frame counter
    } flight_assist;
} UnifiedFlightControl;
```

### Integration with Existing Systems

#### **Input Processing Pipeline:**
```
Input Service → UnifiedFlightControl → Intent Extraction → Flight Assist → Thruster Commands
```

#### **System Update Order:**
1. `input_service_update()` - Capture player input
2. `unified_control_system_update()` - Process all control modes (including new assist)
3. `thruster_system_update()` - Apply computed commands
4. `physics_system_update()` - Integrate forces to movement

### Configuration Files

**flight_assist_config.yaml** (new):
```yaml
flight_assist:
  default:
    sphere_radius: 50.0          # Base targeting distance
    kp_position: 2.0             # Position control gain
    kd_velocity: 0.5             # Velocity damping
    max_acceleration: 30.0       # m/s² limit
    max_banking: 45.0           # degrees
    
  modes:
    basic:
      responsiveness: 0.6
      smoothing: 0.8
    precision:
      responsiveness: 0.3
      smoothing: 0.9
    race:
      responsiveness: 1.0
      smoothing: 0.4
```

---

## 🧪 Testing Strategy

### **Unit Tests** (Days 1-14)
- [ ] PD controller stability and convergence
- [ ] Target calculation accuracy  
- [ ] Mode switching logic
- [ ] Banking angle calculations

### **Integration Tests** (Days 8-14)
- [ ] Flight assist with existing input system
- [ ] Multi-ship performance testing
- [ ] Compatibility with existing scenes

### **Performance Benchmarks** (Day 12-13)
- [ ] Single ship: <0.1ms per frame
- [ ] 5 ships: <0.5ms total per frame
- [ ] No memory leaks over 1000 frames
- [ ] Smooth 60+ FPS maintained

---

## 📊 Success Criteria

### **Technical Requirements**
- ✅ Zero breaking changes to existing flight controls
- ✅ Seamless mode switching (Direct ↔ Assisted)
- ✅ Performance: 60+ FPS with 5+ assisted ships
- ✅ Memory: <100KB additional per ship
- ✅ Latency: <100ms input to thruster response

### **User Experience Goals**
- ✅ "Natural feel" - banking, smooth acceleration
- ✅ Easy to learn (<5 minutes for basic use)
- ✅ Precision capable (for advanced maneuvers)
- ✅ Optional (always can fall back to direct control)

### **Quality Gates**
- ✅ All existing tests pass
- ✅ New functionality has >90% test coverage
- ✅ Code review approval
- ✅ Performance regression tests pass

---

## 🚧 Risk Mitigation

### **Technical Risks**
- **PD Tuning Difficulty** → Start with proven values from literature, user-configurable
- **Performance Impact** → Profile early, optimize hot paths, implement LOD
- **Integration Complexity** → Build incrementally, test continuously

### **User Experience Risks**  
- **"Feels artificial"** → Extensive playtesting, natural banking, smooth responses
- **"Takes control away"** → Make optional, clear mode indicators, instant override

---

## 🎯 Future Extensions (Post-Sprint)

This sprint creates foundation for:
- **Sprint 27**: Waypoint autopilot using same target system
- **Sprint 28**: Formation flying with multi-ship coordination  
- **Sprint 29**: AI opponents using same flight assist
- **Canyon Racing**: Precision flight through tight spaces

---

## 📝 Deliverables Summary

### **Code Changes**
- [ ] Enhanced `UnifiedFlightControl` component
- [ ] PD controller math library
- [ ] Flight assist integration in control system
- [ ] Mode switching UI integration

### **Documentation**
- [ ] User manual for flight assist features
- [ ] Technical documentation for developers
- [ ] Configuration guide for tuning

### **Testing**
- [ ] Comprehensive test suite
- [ ] Performance benchmarks
- [ ] Integration validation

---

*Sprint 26 focuses on enhancing existing systems rather than building from scratch. This approach minimizes risk while delivering maximum value for canyon racing gameplay.*

**Document Version**: 2.0 (Consolidated)  
**Last Updated**: July 6, 2025  
**Review Date**: July 13, 2025 (Week 1 Checkpoint)
