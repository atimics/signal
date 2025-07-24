# SIGNAL Engine Technical Debt Report

**Document ID**: TECH_DEBT_2025_07  
**Date**: July 8, 2025  
**Author**: Engineering Team  
**Status**: Active Review

---

## 📊 Executive Summary

This report identifies technical debt, incomplete implementations, and actionable improvements across the SIGNAL codebase. The analysis reveals **69 files with TODO/FIXME/HACK comments** and multiple stub implementations that need attention.

### Key Findings:
- **Critical**: Input system action mapping loader is stubbed, blocking keyboard controls
- **High Priority**: Entity naming system missing, blocking multiple features
- **Medium Priority**: Multiple rendering features stubbed (thrust cones, materials)
- **Low Priority**: Platform-specific stubs (WASM, touch input)

---

## 🚨 Critical Issues (Fix Immediately)

### 1. **Input Action Map Loader** 
**File**: `src/services/input_action_maps.c`
**Issue**: JSON parsing is stubbed, forcing fallback to hardcoded bindings
**Impact**: No customizable controls, poor user experience
**Recommendation**: 
```c
// TODO: Implement actual JSON parsing
// Use existing YAML parser as reference or add lightweight JSON lib
// Expected format:
// {
//   "contexts": {
//     "gameplay": {
//       "actions": {
//         "thrust_forward": [
//           {"device": "keyboard", "key": "W"},
//           {"device": "gamepad", "axis": 5, "scale": 1.0}
//         ]
//       }
//     }
//   }
// }
```

### 2. **Entity Name/Tag System**
**Files**: `src/core.c`, multiple references
**Issue**: No way to identify entities by name
**Impact**: Blocks scene scripting, debugging, and content creation
**Recommendation**:
- Add `char name[64]` to Entity struct
- Add hashtable for name->EntityID lookup
- Implement `entity_find_by_name()` and `entity_set_name()`

---

## 🔥 High Priority Issues (Sprint 27)

### 3. **Gamepad Support in Sokol HAL**
**File**: `src/hal/input_hal_sokol.c`
**Issue**: Gamepad events not implemented
**Impact**: No controller support despite UI claiming Xbox support
**Recommendation**:
- Implement `sapp_event` gamepad handling
- Map Sokol gamepad events to HAL events
- Test with Xbox/PlayStation controllers

### 4. **Thrust Cone Rendering**
**Files**: `src/render_thrust_cones.c`, `src/render_thrusters.c`
**Issue**: Visual feedback for thrusters not implemented
**Impact**: Poor gameplay feedback, harder to control ships
**Recommendation**:
- Generate cone mesh procedurally
- Use emissive materials for glow effect
- Scale based on thrust magnitude

### 5. **Collision Spatial Partitioning**
**File**: `src/system/collision.c`
**Issue**: O(n²) collision checking
**Impact**: Performance issues with >100 entities
**Recommendation**:
- Implement octree or spatial hash
- Only check nearby entities
- Add broad phase before narrow phase

---

## 📋 Medium Priority Issues (Sprint 28-29)

### 6. **Asset Reloading System**
**File**: `src/assets.c`
**Issue**: No hot reload for development
**Impact**: Slow iteration time
**Recommendation**:
- Watch file timestamps
- Reload changed assets
- Preserve entity references

### 7. **Scene Discovery**
**File**: `src/ui_components.c`
**Issue**: Scene list is hardcoded
**Impact**: Manual updates for new scenes
**Recommendation**:
- Scan data/scenes/ directory
- Parse YAML headers for metadata
- Cache results

### 8. **Performance Metrics**
**Files**: Various system files
**Issue**: Timing calculations incomplete
**Impact**: Can't optimize bottlenecks
**Recommendation**:
- Add high-resolution timer
- Track system update times
- Display in debug UI

---

## 🔧 Technical Debt by System

### Input System
```
TODO Count: 12
Key Issues:
- [ ] Controller count detection
- [ ] Touch input support  
- [ ] Binding serialization
- [ ] Analog stick dead zones per-device
- [ ] Haptic feedback support
```

### Flight Control
```
TODO Count: 8
Key Issues:
- [ ] Strafe controls (X-axis movement)
- [ ] Formation flying logic
- [ ] Waypoint following
- [ ] Landing assistance
- [ ] Docking mechanics
```

### Rendering Pipeline
```
TODO Count: 15
Key Issues:
- [ ] Instanced rendering for thrust cones
- [ ] Material hot reload
- [ ] Shadow mapping
- [ ] Bloom post-processing
- [ ] Debug visualization overlays
```

### Physics System
```
TODO Count: 6
Key Issues:
- [ ] Continuous collision detection
- [ ] Gravity wells
- [ ] Aerodynamic drag
- [ ] Damage modeling
- [ ] Soft-body support
```

---

## 🛠️ Actionable Recommendations

### Immediate Actions (This Week)
1. **Fix Input Action Maps**
   - Add JSON parser (can use cJSON - single header)
   - Implement `load_from_file` properly
   - Add unit tests for parsing

2. **Entity Names**
   - Extend Entity struct
   - Add name registry to World
   - Update YAML loader to set names

3. **Basic Gamepad Support**
   - Wire up Sokol gamepad events
   - Test with common controllers
   - Add calibration UI

### Next Sprint (Sprint 27)
1. **Complete Flight Assist**
   - Fix strafe controls
   - Add waypoint system
   - Implement formation presets

2. **Thrust Rendering**
   - Generate cone meshes
   - Add particle effects
   - Implement heat coloring

3. **Performance Monitoring**
   - Add frame time graphs
   - System breakdown view
   - Memory usage tracking

### Future Sprints
1. **Asset Pipeline**
   - Hot reload system
   - Asset dependency tracking
   - Build optimization

2. **Advanced Physics**
   - Spatial partitioning
   - Predictive collision
   - Damage system

3. **Platform Support**
   - WASM optimizations
   - Mobile touch controls
   - Steam Deck profile

---

## 📈 Progress Tracking

### Debt Metrics
- **Total TODOs**: 69 across 45 files
- **Critical Stubs**: 3 (input maps, thrust cones, entity names)
- **Platform Stubs**: 8 (WASM, touch, gamepad)
- **Migration Debt**: 3 components pending

### Recommended KPIs
1. **TODO Burndown**: Track weekly reduction
2. **Stub Completion**: % of stubs replaced
3. **Performance**: Frame time consistency
4. **Test Coverage**: Especially for new implementations

---

## 🎯 Conclusion

The codebase is well-structured with clear separation of concerns, but has accumulated technical debt typical of rapid prototyping. The most critical issue is the input system's action mapping loader, which directly impacts gameplay.

**Recommended Priority**:
1. Input action map loader (blocking gameplay)
2. Entity naming system (blocking content)
3. Gamepad support (feature parity)
4. Thrust rendering (visual polish)
5. Performance monitoring (optimization)

With focused effort on these items, SIGNAL can transition from prototype to polished game engine while maintaining its clean architecture.

---

**Document Version**: 1.0  
**Next Review**: July 15, 2025  
**Owner**: Lead Engineer