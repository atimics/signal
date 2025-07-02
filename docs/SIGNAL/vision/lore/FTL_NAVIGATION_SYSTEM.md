# SIGNAL: FTL Navigation System

**Project**: SIGNAL Engine & "The Ghost Signal" Game  
**Type**: Lore & Gameplay Systems  
**Status**: Vision Document - Worldbuilding Foundation  
**Updated**: January 2025

---

## ✦ Core Concept

The player navigates between derelicts using FTL routes that form dynamically based on gravitational field intersections. These "Sheaths" change over time due to orbiting hazards and AI activity. The player sees only what their sensors reveal and must decide how much to risk—or invest in better clarity.

This system transforms travel into a strategic mini-game where information is currency and risk assessment is survival.

---

## 🎮 Gameplay Summary

1. **Route Selection**: Player selects destination from navigation map
2. **Route Analysis**: System displays potential FTL routes (Sheaths) with stability and threat indicators based on sensor quality
3. **Intelligence Gathering**: Player optionally slots Echoes, purchases scans, or uses forecasts to improve route clarity
4. **Commitment**: Player commits to a route and initiates FTL jump
5. **Dynamic Outcomes**: Results may vary based on actual stability vs. predicted data

---

## 🧩 Core Mechanics

| Element | Description |
|---------|-------------|
| **FTL Nodes** | Start and end points for FTL travel (wrecks, derelicts, nav buoys) |
| **FTL Lanes (Sheaths)** | Temporary threads between nodes, valid when local gravity vectors cancel |
| **Hazards** | Asteroids, pirates, or Warden scouts that may block or threaten a path |
| **Sensor Coverage** | Determines accuracy of the player's route data and threat assessment |
| **Echoes / Forecasts** | Slottable intelligence items that reveal hidden information about lanes or threats |
| **Stability Score** | Percentage (0–100%) indicating likelihood of successful, safe travel |

---

## 🖥 UI Elements (FTL Planner)

| UI Element | Function |
|------------|----------|
| **Map View** | 2D tactical display of FTL nodes and lanes (represented as colored lines) |
| **Lane Colors** | Blue (stable), Yellow (uncertain), Red (unstable), Gray (unknown) |
| **Flicker Intensity** | Visual indicator of sensor confidence - more flicker = less certainty |
| **Hover Panels** | Detailed information on threat type, forecast window, stability, last scan time |
| **Echo Slot Panel** | Drag & drop interface for deploying intelligence to unlock better route data |
| **Confirm Button** | Final commitment interface - player locks in route and initiates jump |

---

## 🧪 Example Navigation Scenario

**Situation**: Player at WRECK-A, destination DERELICT-B

**Available Routes**:
1. **Route X**: 85% stable, last scan 10 minutes ago, asteroid drift possible
2. **Route Y**: 50% stable, known pirate movement nearby, fast transit
3. **Route Z**: Hidden route—requires slotting a "Fleet Command Echo" to reveal

**Player Options**:
- Take the safe but predictable Route X
- Risk Route Y for speed but potential combat
- Invest a valuable Echo to unlock the mystery Route Z
- Spend time/resources to rescan for better intelligence

**Dynamic Elements**:
- Route stability changes over time due to orbital mechanics
- Pirate movements update based on player actions and elapsed time
- Hidden routes may lead to unique discoveries or dangerous encounters

---

## 📡 Sensor Technology & Intelligence

### Sensor Tiers
- **Basic Sensors**: Show major routes and obvious hazards
- **Enhanced Arrays**: Reveal minor routes and threat details  
- **Military-Grade**: Detect hidden lanes and stealth threats
- **Aethelian Tech**: Interface directly with Ghost Signal for unique intel

### Echo Types
- **Navigation Echoes**: Reveal hidden routes or improve stability calculations
- **Tactical Echoes**: Provide intelligence on hostile movements
- **Engineering Echoes**: Show maintenance routes and safe harbors
- **Command Echoes**: Unlock high-security military lanes

---

## 🌌 Narrative Integration

The FTL system serves multiple narrative purposes:

### World Building
- **Technological Decay**: Ancient nav beacons failing, routes becoming unstable
- **Factional Control**: Different groups control different lane types
- **Historical Echoes**: Old routes tell stories through their destinations

### Player Agency
- **Risk vs. Reward**: Fast routes may be dangerous; safe routes may be monitored
- **Information Economy**: Intelligence becomes a valuable resource
- **Exploration Incentive**: Hidden routes lead to unique content

### Atmospheric Tension
- **Uncertainty**: Never complete information creates constant tension
- **Consequence**: Poor route choices have meaningful impacts
- **Discovery**: Finding new routes feels like genuine exploration

---

## 🛠 Technical Implementation Notes

### Performance Considerations
- Route calculations should be precomputed and cached
- Real-time updates only for dynamic elements (hazards, timers)
- UI should feel responsive even with complex route algorithms

### Scalability
- System must support expansion to hundreds of nodes
- Route generation should be data-driven for easy content creation
- Echo effects should be modular for easy addition of new intelligence types

### Player Experience
- Tutorial should gradually introduce complexity
- Visual feedback must clearly communicate risk levels
- Save/load must preserve route states and timing

---

**The FTL Navigation System transforms necessary travel into meaningful choice, where every journey is a calculated risk and every route tells a story.**
