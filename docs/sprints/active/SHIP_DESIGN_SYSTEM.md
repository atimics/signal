# Ship Design System Architecture

## Overview
This document outlines the strategy for implementing a modular, physics-based ship design system in CGame using ODE (Open Dynamics Engine) for realistic multi-body dynamics simulation.

## Core Concepts

### 1. Layered Ship Architecture
Ships are composed of multiple layers/systems:
- **Structural Shell**: Primary hull and load-bearing structures
- **Propulsion Layer**: Engines, thrusters, fuel lines
- **Systems Layer**: Power, life support, sensors
- **Mechanical Layer**: Actuators, gimbals, control surfaces

### 2. Component-Based Assembly
Each ship part is a discrete component with:
- Physical properties (mass, inertia, drag)
- Attachment nodes (standardized connection points)
- Functional properties (thrust, power, etc.)
- Visual representation (mesh + materials)
- ODE rigid body representation

### 3. Physics Integration
Using ODE's capabilities:
- Each part is a separate rigid body
- Parts connected via joints (fixed, hinge, universal)
- Composite body for optimized simulation
- Realistic gimbal/actuator constraints
- Proper mass distribution and inertia

## Implementation Strategy

### Phase 1: Foundation (Current)
- [x] Define ship part data structures
- [x] Create part library system
- [x] Design YAML part definition format
- [ ] Implement basic part loading
- [ ] Create assembly data structure

### Phase 2: Part Library
- [ ] Create basic hull meshes
- [ ] Design engine/thruster meshes
- [ ] Model structural components
- [ ] Implement tank/resource parts
- [ ] Build part catalog UI

### Phase 3: Assembly System
- [ ] Part attachment validation
- [ ] Node compatibility checking
- [ ] Mass/balance calculation
- [ ] Visual assembly preview
- [ ] Save/load ship designs

### Phase 4: Physics Simulation
- [ ] ODE body creation per part
- [ ] Joint creation for connections
- [ ] Composite body optimization
- [ ] Thrust force application
- [ ] Gimbal motor control

### Phase 5: Advanced Features
- [ ] Fuel flow simulation
- [ ] Structural stress analysis
- [ ] Part failure/damage
- [ ] Thermal management
- [ ] Resource networks

## Part Categories

### Hulls
- Command pods (crew capacity)
- Cargo bays (volume/mass limits)
- Service modules (equipment space)
- Adapters (size transitions)

### Propulsion
- Main engines (gimbal-capable)
- RCS blocks (attitude control)
- Ion drives (low thrust, high ISP)
- Verniers (precision control)

### Tanks
- Fuel tanks (various sizes)
- Oxidizer tanks
- Monopropellant storage
- Battery banks

### Structural
- Beams/trusses
- Struts (reinforcement)
- Decouplers/separators
- Landing gear

### Systems
- Solar panels
- Radiators
- Reaction wheels
- Sensors/antennas

## Data Formats

### Part Definition (YAML)
```yaml
part_id: unique_identifier
display_name: "Human Readable Name"
category: CATEGORY_ENUM

physical:
  mass: 100.0  # kg
  center_of_mass: [x, y, z]
  drag_coefficient: 0.2
  
visual:
  mesh: "mesh_asset_name"
  material: "material_name"
  
attachment_nodes:
  - name: "node_name"
    position: [x, y, z]
    direction: [x, y, z]
    type: NODE_TYPE
    size: diameter
    
properties:
  # Category-specific properties
```

### Ship Assembly (JSON)
```json
{
  "ship_name": "Test Vessel Alpha",
  "root_part": "hull_basic_pod",
  "parts": [
    {
      "part_id": "engine_rl10",
      "instance_id": "engine_1",
      "parent": "hull_basic_pod",
      "attachment": "bottom",
      "position": [0, -1.2, 0],
      "rotation": [0, 0, 0, 1]
    }
  ]
}
```

## Technical Challenges

### 1. Mesh Generation
- Need procedural mesh generation for parts
- UV mapping for textures
- LOD system for complex assemblies

### 2. Joint Stability
- Proper joint parameters for stability
- Handling large mass ratios
- Preventing joint explosions

### 3. Performance
- Efficient collision detection
- Composite body optimization
- LOD physics simulation

### 4. User Interface
- Intuitive part placement
- Visual feedback for connections
- Real-time mass/balance display

## Test Cases

### Basic Assembly Test
1. Create pod + engine + tanks
2. Verify mass calculation
3. Test gimbal control
4. Launch sequence simulation

### Complex Ship Test
1. Multi-stage rocket
2. Space station assembly
3. Modular cargo vessel
4. Fighter with RCS

### Stress Tests
1. High part count (100+ parts)
2. Complex joint chains
3. Rapid assembly/disassembly
4. Collision scenarios

## Integration Points

### With Existing Systems
- ECS architecture for ship entities
- Rendering system for part meshes
- Input system for assembly controls
- Save/load with scene system

### New Systems Needed
- Part browser UI
- Assembly workspace
- Joint visualization
- Resource flow display

## Success Metrics
- Stable physics with 50+ part ships
- Intuitive assembly process
- Realistic flight dynamics
- 60 FPS with complex ships
- Save/load reliability

## Next Steps
1. Implement part loading system
2. Create basic part meshes
3. Build assembly logic
4. Test with simple rocket
5. Iterate based on results