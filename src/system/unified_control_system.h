/**
 * @file unified_control_system.h
 * @brief Unified control system - replaces the fragmented control/controllable/scripted_flight systems
 * 
 * This system provides a single update loop for all control modes:
 * - Manual control (Sprint 25)
 * - Assisted control (Sprint 26)
 * - Scripted flight
 * - Autonomous flight (Sprint 26)
 */

#ifndef UNIFIED_CONTROL_SYSTEM_H
#define UNIFIED_CONTROL_SYSTEM_H

#include "../core.h"
#include "../render.h"
#include "../component/unified_flight_control.h"

// System initialization/shutdown
void unified_control_system_init(void);
void unified_control_system_shutdown(void);

// Main system update
void unified_control_system_update(struct World* world, RenderConfig* render_config, float delta_time);

// Entity management
void unified_control_system_add_entity(struct World* world, EntityID entity_id);
void unified_control_system_remove_entity(struct World* world, EntityID entity_id);

// Player entity management
void unified_control_system_set_player_entity(EntityID player_entity);
EntityID unified_control_system_get_player_entity(void);

// Control mode management
void unified_control_system_set_entity_mode(struct World* world, EntityID entity_id, FlightControlMode mode);
FlightControlMode unified_control_system_get_entity_mode(struct World* world, EntityID entity_id);

// Authority management (for conflict resolution)
void unified_control_system_request_entity_control(struct World* world, EntityID entity_id, 
                                                  ControlAuthority authority, EntityID requester);
void unified_control_system_release_entity_control(struct World* world, EntityID entity_id, EntityID releaser);

// Configuration presets
void unified_control_system_configure_as_player_ship(struct World* world, EntityID entity_id);
void unified_control_system_configure_as_ai_ship(struct World* world, EntityID entity_id);
void unified_control_system_configure_as_scripted_ship(struct World* world, EntityID entity_id);

// Performance monitoring
typedef struct {
    uint32_t entities_updated;
    uint32_t manual_controls;
    uint32_t assisted_controls;
    uint32_t scripted_controls;
    uint32_t autonomous_controls;
    float total_update_time_ms;
    float average_entity_time_ms;
} UnifiedControlSystemStats;

const UnifiedControlSystemStats* unified_control_system_get_stats(void);

#endif // UNIFIED_CONTROL_SYSTEM_H