// src/system/controllable.h
// System for processing controllable components and converting input to forces

#ifndef SYSTEM_CONTROLLABLE_H
#define SYSTEM_CONTROLLABLE_H

#include "../core.h"

// Forward declarations
struct World;
struct RenderConfig;

// Initialize controllable system
void controllable_system_init(void);
void controllable_system_shutdown(void);

// Update controllable entities
void controllable_system_update(struct World* world, struct RenderConfig* render_config, float delta_time);

// Set which entity is controlled by the player
void controllable_system_set_player_entity(EntityID entity_id);
EntityID controllable_system_get_player_entity(void);

// Enable/disable input for specific entity
void controllable_system_enable_entity(struct World* world, EntityID entity_id, bool enabled);

// Debug
void controllable_system_debug_print(struct World* world);

#endif // SYSTEM_CONTROLLABLE_H