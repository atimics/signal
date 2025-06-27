# Project Roadmap

This file tracks the project's development, including features, bugs, and future plans.

## Recently Completed (Sprint: "First Contact")

### ✅ Core Physics & Collision System 
- **Sphere-based collision detection**: All entities now have collision spheres
- **Collision response**: Ships stop and separate when colliding with solid objects
- **Player-Sun collision**: Bug #1 resolved - player can no longer fly through the sun
- **Mass-based physics**: Heavier objects (like the sun) are immovable in collisions

### ✅ AI System Architecture
- **Real AI integration framework**: Ready for llama.cpp when disk space permits
- **Performance monitoring**: Tracks inference times and AI task throughput  
- **Mock AI with realistic responses**: Context-aware responses based on prompt analysis
- **Task queue system**: Handles multiple concurrent AI operations

### ✅ Reactive AI Behavior
- **Proximity detection**: AI ships detect when player approaches (within 100 units)
- **AI reaction tasks**: New AI_TASK_REACTION type for dynamic responses
- **Behavioral responses**: AI ships can turn to face player, send greetings, or move away
- **Smart prompting**: AI receives contextual information about ship type and situation

### ✅ Living Universe Population
- **AI-driven entities**: 6 AI ships with different personalities populate the universe
- **Ship variety**: Fighters, traders, and explorers with unique behaviors
- **Named entities**: Ships have names and distinct characteristics

## Next Sprint Priorities

### P0: Enhanced AI Reactions
- **Visual ship orientation**: Make ships actually rotate to face the player
- **Radio communications**: Convert AI greetings into dialog system messages  
- **Advanced maneuvers**: Implement realistic evasive and approach patterns

### P1: Real Model Integration
- **Install llama.cpp**: Once disk space is available, integrate real LLM
- **Model optimization**: Test small quantized models for real-time performance
- **Context management**: Improve AI memory and conversation continuity

## Feature Roadmap

- [ ] **Asteroid Field**: 3D obstacles to navigate around
- [ ] **Enemy Ships**: AI-controlled opponents with combat AI
- [ ] **Weapons System**: Laser cannons and projectiles
- [ ] **Sound Effects**: Engine sounds and weapon fire
- [ ] **Textures**: Replace wireframes with textured models
- [ ] **Lighting**: Dynamic lighting system
- [ ] **HUD Improvements**: Radar, health, energy systems
- [ ] **Multiple Levels**: Different space environments

## Bug Tracker

| ID  | Description | Status | Priority |
| --- | ----------- | ------ | -------- |
| ~~1~~   | ~~Player can fly through the sun~~ | ✅ **Fixed**   | ~~Medium~~   |

## Performance Metrics

Current AI performance (mock implementation):
- **Average inference time**: ~75ms (simulated)
- **Concurrent AI entities**: 6 ships  
- **Reaction distance**: 100 units
- **AI task frequency**: Every 1-2 seconds per entity
