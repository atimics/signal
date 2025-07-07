/**
 * @file unified_control_system.c
 * @brief Implementation of unified control system
 */

#include "unified_control_system.h"
#include "../component/unified_flight_control.h"
#include "../services/input_service.h"
#include "../game_input.h"
#include "thrusters.h"
#include "physics.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

// Forward declaration for helper function
static void apply_control_to_thrusters(struct Entity* entity, const UnifiedFlightControl* control);

// System state
static EntityID g_player_entity = INVALID_ENTITY;
static UnifiedControlSystemStats g_stats = {0};

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

void unified_control_system_init(void) {
    g_player_entity = INVALID_ENTITY;
    memset(&g_stats, 0, sizeof(g_stats));
    printf("✅ Unified Control System initialized\n");
}

void unified_control_system_shutdown(void) {
    printf("✅ Unified Control System shut down\n");
}

// ============================================================================
// MAIN SYSTEM UPDATE
// ============================================================================

void unified_control_system_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config;
    if (!world) return;
    
    // Reset frame stats
    memset(&g_stats, 0, sizeof(g_stats));
    // float frame_start_time = 0.0f; // TODO: Add timing
    
    // Get input service for manual control
    InputService* input_service = game_input_get_service();
    
    // Process all entities with unified flight control
    for (uint32_t i = 0; i < world->entity_count; i++) {
        struct Entity* entity = &world->entities[i];
        
        // Skip entities without unified flight control component
        if (!(entity->component_mask & COMPONENT_UNIFIED_FLIGHT_CONTROL)) {
            continue;
        }
        
        UnifiedFlightControl* control = entity->unified_flight_control;
        if (!control) continue;
        
        g_stats.entities_updated++;
        
        // Determine if this is the player entity
        bool is_player = (entity->id == g_player_entity);
        if (is_player) {
            unified_flight_control_request_authority(control, AUTHORITY_PLAYER, g_player_entity);
        }
        
        // Process input for player entity
        if (is_player && input_service) {
            unified_flight_control_process_input(control, input_service);
        }
        
        // Update the control component
        unified_flight_control_update(control, delta_time);
        
        // Track stats by mode
        switch (unified_flight_control_get_mode(control)) {
            case FLIGHT_CONTROL_MANUAL:
                g_stats.manual_controls++;
                break;
            case FLIGHT_CONTROL_ASSISTED:
                g_stats.assisted_controls++;
                break;
            case FLIGHT_CONTROL_SCRIPTED:
                g_stats.scripted_controls++;
                break;
            case FLIGHT_CONTROL_AUTONOMOUS:
            case FLIGHT_CONTROL_FORMATION:
                g_stats.autonomous_controls++;
                break;
        }
        
        // Apply control commands to thrusters if entity has them
        if (entity->component_mask & COMPONENT_THRUSTER_SYSTEM && entity->thruster_system) {
            apply_control_to_thrusters(entity, control);
        }
    }
    
    // Update performance stats
    g_stats.total_update_time_ms = 0.0f; // TODO: Calculate actual time
    g_stats.average_entity_time_ms = g_stats.entities_updated > 0 ? 
        g_stats.total_update_time_ms / g_stats.entities_updated : 0.0f;
    
    // Debug logging
    static uint32_t debug_counter = 0;
    if (++debug_counter % 300 == 0) { // Every 5 seconds at 60 FPS
        printf("🎮 Unified Control Stats: %d entities, %d manual, %d assisted, %d scripted, %d autonomous\n",
               g_stats.entities_updated, g_stats.manual_controls, g_stats.assisted_controls,
               g_stats.scripted_controls, g_stats.autonomous_controls);
    }
}

// ============================================================================
// ENTITY MANAGEMENT
// ============================================================================

void unified_control_system_add_entity(struct World* world, EntityID entity_id) {
    if (!world) return;
    
    // Add UnifiedFlightControl component to entity
    if (entity_add_component(world, entity_id, COMPONENT_UNIFIED_FLIGHT_CONTROL)) {
        printf("🎮 Added entity %d to unified control system\n", entity_id);
    } else {
        printf("❌ Failed to add entity %d to unified control system\n", entity_id);
    }
}

void unified_control_system_remove_entity(struct World* world, EntityID entity_id) {
    if (!world) return;
    
    // Remove UnifiedFlightControl component from entity
    if (entity_remove_component(world, entity_id, COMPONENT_UNIFIED_FLIGHT_CONTROL)) {
        printf("🎮 Removed entity %d from unified control system\n", entity_id);
    } else {
        printf("⚠️ Entity %d was not in unified control system\n", entity_id);
    }
}

// ============================================================================
// PLAYER ENTITY MANAGEMENT
// ============================================================================

void unified_control_system_set_player_entity(EntityID player_entity) {
    g_player_entity = player_entity;
    printf("🎮 Unified Control: Player entity set to %d\n", player_entity);
}

EntityID unified_control_system_get_player_entity(void) {
    return g_player_entity;
}

// ============================================================================
// CONTROL MODE MANAGEMENT
// ============================================================================

void unified_control_system_set_entity_mode(struct World* world, EntityID entity_id, FlightControlMode mode) {
    if (!world) return;
    
    UnifiedFlightControl* control = entity_get_unified_flight_control(world, entity_id);
    if (control) {
        unified_flight_control_set_mode(control, mode);
        printf("🎮 Entity %d control mode set to %d\n", entity_id, mode);
    } else {
        printf("⚠️ Entity %d does not have unified flight control\n", entity_id);
    }
}

FlightControlMode unified_control_system_get_entity_mode(struct World* world, EntityID entity_id) {
    if (!world) return FLIGHT_CONTROL_MANUAL;
    
    UnifiedFlightControl* control = entity_get_unified_flight_control(world, entity_id);
    return control ? unified_flight_control_get_mode(control) : FLIGHT_CONTROL_MANUAL;
}

// ============================================================================
// AUTHORITY MANAGEMENT
// ============================================================================

void unified_control_system_request_entity_control(struct World* world, EntityID entity_id, 
                                                  ControlAuthority authority, EntityID requester) {
    if (!world) return;
    
    UnifiedFlightControl* control = entity_get_unified_flight_control(world, entity_id);
    if (control) {
        unified_flight_control_request_authority(control, authority, requester);
    } else {
        printf("⚠️ Entity %d does not have unified flight control\n", entity_id);
    }
}

void unified_control_system_release_entity_control(struct World* world, EntityID entity_id, EntityID releaser) {
    if (!world) return;
    
    UnifiedFlightControl* control = entity_get_unified_flight_control(world, entity_id);
    if (control) {
        unified_flight_control_release_authority(control, releaser);
    } else {
        printf("⚠️ Entity %d does not have unified flight control\n", entity_id);
    }
}

// ============================================================================
// CONFIGURATION PRESETS
// ============================================================================

void unified_control_system_configure_as_player_ship(struct World* world, EntityID entity_id) {
    if (!world) return;
    
    // Ensure entity has UnifiedFlightControl component
    if (!(entity_get_unified_flight_control(world, entity_id))) {
        unified_control_system_add_entity(world, entity_id);
    }
    
    UnifiedFlightControl* control = entity_get_unified_flight_control(world, entity_id);
    if (control) {
        unified_flight_control_setup_manual_flight(control);
        unified_flight_control_request_authority(control, AUTHORITY_PLAYER, entity_id);
        printf("🎮 Entity %d configured as player ship\n", entity_id);
    }
}

void unified_control_system_configure_as_ai_ship(struct World* world, EntityID entity_id) {
    if (!world) return;
    
    // Ensure entity has UnifiedFlightControl component
    if (!(entity_get_unified_flight_control(world, entity_id))) {
        unified_control_system_add_entity(world, entity_id);
    }
    
    UnifiedFlightControl* control = entity_get_unified_flight_control(world, entity_id);
    if (control) {
        unified_flight_control_setup_autonomous_flight(control);
        unified_flight_control_request_authority(control, AUTHORITY_AI, entity_id);
        printf("🎮 Entity %d configured as AI ship\n", entity_id);
    }
}

void unified_control_system_configure_as_scripted_ship(struct World* world, EntityID entity_id) {
    if (!world) return;
    
    // Ensure entity has UnifiedFlightControl component
    if (!(entity_get_unified_flight_control(world, entity_id))) {
        unified_control_system_add_entity(world, entity_id);
    }
    
    UnifiedFlightControl* control = entity_get_unified_flight_control(world, entity_id);
    if (control) {
        unified_flight_control_set_mode(control, FLIGHT_CONTROL_SCRIPTED);
        unified_flight_control_request_authority(control, AUTHORITY_SCRIPT, entity_id);
        printf("🎮 Entity %d configured as scripted ship\n", entity_id);
    }
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

const UnifiedControlSystemStats* unified_control_system_get_stats(void) {
    return &g_stats;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static void apply_control_to_thrusters(struct Entity* entity, const UnifiedFlightControl* control) {
    if (!entity || !entity->thruster_system || !control) return;
    
    struct ThrusterSystem* thrusters = entity->thruster_system;
    
    // Get control commands
    Vector3 linear_command = unified_flight_control_get_linear_command(control);
    Vector3 angular_command = unified_flight_control_get_angular_command(control);
    
    // Apply commands to thruster system
    thruster_set_linear_command(thrusters, linear_command);
    thruster_set_angular_command(thrusters, angular_command);
}