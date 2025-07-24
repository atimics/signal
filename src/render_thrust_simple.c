/**
 * @file render_thrust_simple.c
 * @brief Simple thrust effect rendering using debug primitives
 * 
 * This is a temporary implementation that provides visual feedback
 * for thrusters without requiring full mesh integration.
 */

#include "render_thrusters.h"
#include "core.h"
#include <stdio.h>
#include <math.h>

// Simple debug visualization of thrust
void render_thrust_debug(struct World* world, EntityID entity_id) {
    if (!world) return;
    
    struct Entity* entity = entity_get(world, entity_id);
    if (!entity) return;
    
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(world, entity_id);
    
    if (!transform || !thrusters || !thrusters->thrusters_enabled) return;
    
    // Get thrust magnitudes
    float forward_thrust = fabsf(thrusters->current_linear_thrust.z);
    float left_thrust = fabsf(thrusters->current_linear_thrust.x);
    float up_thrust = fabsf(thrusters->current_linear_thrust.y);
    
    // Log significant thrust
    if (forward_thrust > 0.1f || left_thrust > 0.1f || up_thrust > 0.1f) {
        static int thrust_log_counter = 0;
        if (++thrust_log_counter % 60 == 0) { // Log once per second at 60fps
            printf("🚀 Entity %d thrust: fwd=%.2f left=%.2f up=%.2f\n", 
                   entity_id, forward_thrust, left_thrust, up_thrust);
        }
    }
    
    // Visual feedback will be added when proper debug drawing is available
    // For now, the thrust values are being correctly calculated and can be
    // used by other systems (like particle effects or UI indicators)
}

// Initialize simple thrust rendering (placeholder)
bool thrust_renderer_init(void) {
    printf("✅ Simple thrust renderer initialized\n");
    return true;
}

// Cleanup thrust rendering resources (placeholder)
void thrust_renderer_cleanup(void) {
    printf("🧹 Simple thrust renderer cleaned up\n");
}