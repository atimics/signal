#include "scripted_flight.h"
#include "control.h"
#include "thrusters.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Global flight components (simple array for now)
#define MAX_SCRIPTED_ENTITIES 16
static struct {
    EntityID entity_id;
    ScriptedFlight flight;
} g_scripted_flights[MAX_SCRIPTED_ENTITIES];
static int g_scripted_flight_count = 0;

// Helper functions
static float vector3_distance_squared(Vector3 a, Vector3 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return dx*dx + dy*dy + dz*dz;
}

static Vector3 vector3_normalize_safe(Vector3 v) {
    float length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length < 0.001f) return (Vector3){0, 0, 1}; // Default forward
    return (Vector3){v.x/length, v.y/length, v.z/length};
}

// Use the vector3_lerp from core.h instead of defining our own

// ============================================================================
// FLIGHT PATH CREATION
// ============================================================================

FlightPath* scripted_flight_create_circuit_path(void) {
    static FlightPath circuit_path;
    memset(&circuit_path, 0, sizeof(FlightPath));
    
    // Create a rectangular circuit using the marker positions from flight_test.yaml
    circuit_path.waypoint_count = 4;
    circuit_path.loop = true;
    circuit_path.default_speed = 25.0f;
    circuit_path.max_acceleration = 15.0f;
    circuit_path.max_turn_rate = 1.5f; // radians/second
    
    // Waypoint 1: Blue marker (50, 10, 0)
    circuit_path.waypoints[0] = (Waypoint){
        .position = {50, 10, 0},
        .type = WAYPOINT_PASS_THROUGH,
        .target_speed = 25.0f,
        .tolerance = 5.0f
    };
    
    // Waypoint 2: Green marker (0, 10, 50)
    circuit_path.waypoints[1] = (Waypoint){
        .position = {0, 10, 50},
        .type = WAYPOINT_PASS_THROUGH,
        .target_speed = 25.0f,
        .tolerance = 5.0f
    };
    
    // Waypoint 3: Red marker (-50, 10, 0)
    circuit_path.waypoints[2] = (Waypoint){
        .position = {-50, 10, 0},
        .type = WAYPOINT_PASS_THROUGH,
        .target_speed = 25.0f,
        .tolerance = 5.0f
    };
    
    // Waypoint 4: Yellow marker (0, 10, -50)
    circuit_path.waypoints[3] = (Waypoint){
        .position = {0, 10, -50},
        .type = WAYPOINT_PASS_THROUGH,
        .target_speed = 25.0f,
        .tolerance = 5.0f
    };
    
    printf("🛩️  Created circuit path with %d waypoints\n", circuit_path.waypoint_count);
    return &circuit_path;
}

FlightPath* scripted_flight_create_figure_eight_path(void) {
    static FlightPath figure_eight_path;
    memset(&figure_eight_path, 0, sizeof(FlightPath));
    
    // Create a figure-8 pattern
    figure_eight_path.waypoint_count = 8;
    figure_eight_path.loop = true;
    figure_eight_path.default_speed = 20.0f;
    figure_eight_path.max_acceleration = 12.0f;
    figure_eight_path.max_turn_rate = 2.0f;
    
    float radius = 30.0f;
    float height = 15.0f;
    
    for (int i = 0; i < 8; i++) {
        float angle = (float)i * M_PI / 4.0f; // 45-degree increments
        float x = radius * cosf(angle);
        float z = radius * sinf(angle * 2.0f); // Double frequency for figure-8
        
        figure_eight_path.waypoints[i] = (Waypoint){
            .position = {x, height, z},
            .type = WAYPOINT_PASS_THROUGH,
            .target_speed = 20.0f,
            .tolerance = 4.0f
        };
    }
    
    printf("🛩️  Created figure-8 path with %d waypoints\n", figure_eight_path.waypoint_count);
    return &figure_eight_path;
}

FlightPath* scripted_flight_create_landing_approach_path(Vector3 start_pos, Vector3 landing_pos) {
    static FlightPath approach_path;
    memset(&approach_path, 0, sizeof(FlightPath));
    
    approach_path.waypoint_count = 4;
    approach_path.loop = false;
    approach_path.default_speed = 15.0f;
    approach_path.max_acceleration = 8.0f;
    approach_path.max_turn_rate = 1.0f;
    
    // High approach
    approach_path.waypoints[0] = (Waypoint){
        .position = {landing_pos.x, landing_pos.y + 50.0f, landing_pos.z - 50.0f},
        .type = WAYPOINT_POSITION,
        .target_speed = 15.0f,
        .tolerance = 8.0f
    };
    
    // Final approach
    approach_path.waypoints[1] = (Waypoint){
        .position = {landing_pos.x, landing_pos.y + 20.0f, landing_pos.z - 20.0f},
        .type = WAYPOINT_POSITION,
        .target_speed = 8.0f,
        .tolerance = 5.0f
    };
    
    // Pre-landing hover
    approach_path.waypoints[2] = (Waypoint){
        .position = {landing_pos.x, landing_pos.y + 10.0f, landing_pos.z},
        .type = WAYPOINT_HOVER,
        .target_speed = 2.0f,
        .hover_duration = 2.0f,
        .tolerance = 3.0f
    };
    
    // Final landing
    approach_path.waypoints[3] = (Waypoint){
        .position = landing_pos,
        .type = WAYPOINT_POSITION,
        .target_speed = 1.0f,
        .tolerance = 2.0f
    };
    
    printf("🛩️  Created landing approach path from (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f)\n", 
           start_pos.x, start_pos.y, start_pos.z,
           landing_pos.x, landing_pos.y, landing_pos.z);
    return &approach_path;
}

// ============================================================================
// COMPONENT MANAGEMENT
// ============================================================================

ScriptedFlight* scripted_flight_create_component(EntityID entity_id) {
    if (g_scripted_flight_count >= MAX_SCRIPTED_ENTITIES) {
        printf("⚠️  Max scripted flight entities reached\n");
        return NULL;
    }
    
    g_scripted_flights[g_scripted_flight_count].entity_id = entity_id;
    ScriptedFlight* flight = &g_scripted_flights[g_scripted_flight_count].flight;
    memset(flight, 0, sizeof(ScriptedFlight));
    
    flight->active = false;
    flight->manual_override = false;
    flight->current_waypoint = 0;
    
    g_scripted_flight_count++;
    printf("🛩️  Created scripted flight component for entity %d\n", entity_id);
    return flight;
}

void scripted_flight_destroy_component(ScriptedFlight* component) {
    if (!component) return;
    
    // Find and remove from array
    for (int i = 0; i < g_scripted_flight_count; i++) {
        if (&g_scripted_flights[i].flight == component) {
            // Shift remaining elements
            for (int j = i; j < g_scripted_flight_count - 1; j++) {
                g_scripted_flights[j] = g_scripted_flights[j + 1];
            }
            g_scripted_flight_count--;
            printf("🛩️  Destroyed scripted flight component\n");
            return;
        }
    }
}

// ============================================================================
// FLIGHT CONTROL
// ============================================================================

void scripted_flight_start(ScriptedFlight* flight, const FlightPath* path) {
    if (!flight) {
        printf("❌ scripted_flight_start: flight is NULL\n");
        return;
    }
    if (!path) {
        printf("❌ scripted_flight_start: path is NULL\n");
        return;
    }
    
    memcpy(&flight->path, path, sizeof(FlightPath));
    flight->current_waypoint = 0;
    flight->state_timer = 0.0f;
    flight->active = true;
    flight->manual_override = false;
    flight->current_speed = 0.0f;
    
    printf("🛩️  Started scripted flight with %d waypoints (loop: %s)\n", 
           path->waypoint_count, path->loop ? "yes" : "no");
    printf("🛩️  First waypoint: pos(%.1f, %.1f, %.1f) speed:%.1f tolerance:%.1f\n",
           flight->path.waypoints[0].position.x,
           flight->path.waypoints[0].position.y,
           flight->path.waypoints[0].position.z,
           flight->path.waypoints[0].target_speed,
           flight->path.waypoints[0].tolerance);
}

void scripted_flight_stop(ScriptedFlight* flight) {
    if (!flight) return;
    flight->active = false;
    flight->manual_override = false;
    printf("🛩️  Stopped scripted flight\n");
}

void scripted_flight_pause(ScriptedFlight* flight) {
    if (!flight) return;
    flight->manual_override = true;
    printf("🛩️  Paused scripted flight (manual override)\n");
}

void scripted_flight_resume(ScriptedFlight* flight) {
    if (!flight) return;
    flight->manual_override = false;
    printf("🛩️  Resumed scripted flight\n");
}

// ============================================================================
// FLIGHT NAVIGATION
// ============================================================================

static void update_scripted_entity(struct World* world, EntityID entity_id, ScriptedFlight* flight, float delta_time) {
    static int call_counter = 0;
    if (++call_counter % 60 == 0) {
        printf("🔍 DEBUG: update_scripted_entity called for entity %d\n", entity_id);
    }
    
    if (!flight->active || flight->manual_override) {
        static int debug_counter = 0;
        if (++debug_counter % 60 == 0) {
            printf("🛩️  Scripted flight inactive: entity=%d, active=%d, override=%d\n", 
                   entity_id, flight->active, flight->manual_override);
        }
        return;
    }
    
    // Get entity components
    struct Transform* transform = entity_get_transform(world, entity_id);
    struct Physics* physics = entity_get_physics(world, entity_id);
    struct ThrusterSystem* thrusters = entity_get_thruster_system(world, entity_id);
    
    if (!transform || !physics || !thrusters) {
        printf("❌ Scripted flight missing components: T=%p P=%p TH=%p\n",
               transform, physics, thrusters);
        return;
    }
    
    // Get current waypoint
    if (flight->current_waypoint >= flight->path.waypoint_count) {
        if (flight->path.loop) {
            flight->current_waypoint = 0;
            printf("🛩️  Looping back to waypoint 0\n");
        } else {
            scripted_flight_stop(flight);
            return;
        }
    }
    
    Waypoint* target_waypoint = &flight->path.waypoints[flight->current_waypoint];
    Vector3 current_pos = transform->position;
    Vector3 target_pos = target_waypoint->position;
    
    // Calculate distance to target
    float distance_sq = vector3_distance_squared(current_pos, target_pos);
    float distance = sqrtf(distance_sq);
    
    // Check if we've reached the current waypoint
    if (distance < target_waypoint->tolerance) {
        if (target_waypoint->type == WAYPOINT_HOVER) {
            if (flight->hover_start_time == 0.0f) {
                flight->hover_start_time = flight->state_timer;
                printf("🛩️  Started hovering at waypoint %d\n", flight->current_waypoint);
            } else if (flight->state_timer - flight->hover_start_time >= target_waypoint->hover_duration) {
                flight->hover_start_time = 0.0f;
                flight->current_waypoint++;
                printf("🛩️  Finished hovering, moving to waypoint %d\n", flight->current_waypoint);
            }
        } else {
            flight->current_waypoint++;
            printf("🛩️  Reached waypoint %d, moving to waypoint %d\n", 
                   flight->current_waypoint - 1, flight->current_waypoint);
        }
        return;
    }
    
    // Calculate desired direction and velocity
    Vector3 direction = vector3_normalize_safe(vector3_subtract(target_pos, current_pos));
    
    // Calculate desired speed based on distance and waypoint type
    float desired_speed = target_waypoint->target_speed;
    if (distance < 20.0f) {
        // Slow down as we approach the waypoint
        desired_speed *= (distance / 20.0f);
        desired_speed = fmaxf(desired_speed, 2.0f);
    }
    
    // Calculate thrust commands
    Vector3 desired_velocity = vector3_multiply_scalar(direction, desired_speed);
    Vector3 velocity_error = vector3_subtract(desired_velocity, physics->velocity);
    
    // Simple PID controller for velocity
    float kp = 0.8f; // Proportional gain
    Vector3 linear_force = vector3_multiply_scalar(velocity_error, kp * physics->mass);
    
    // Limit force to maximum thrust capability
    float max_force = thrusters->max_linear_force.z; // Use forward thrust as reference
    float force_magnitude = vector3_length(linear_force);
    if (force_magnitude > max_force) {
        linear_force = vector3_multiply_scalar(linear_force, max_force / force_magnitude);
    }
    
    // TODO: Calculate orientation for ship to face direction of travel
    // For now, just apply thrust without orientation control
    
    // Convert world force to thrust commands (normalized -1 to 1)
    Vector3 thrust_command = {0, 0, 0};
    if (max_force > 0.0f) {
        thrust_command.x = linear_force.x / max_force;
        thrust_command.y = linear_force.y / max_force;
        thrust_command.z = linear_force.z / max_force;
        
        // Clamp to valid range
        thrust_command.x = fmaxf(-1.0f, fminf(1.0f, thrust_command.x));
        thrust_command.y = fmaxf(-1.0f, fminf(1.0f, thrust_command.y));
        thrust_command.z = fmaxf(-1.0f, fminf(1.0f, thrust_command.z));
    }
    
    // Apply thrust commands through thruster system
    thruster_set_linear_command(thrusters, thrust_command);
    
    // Debug output - ENHANCED
    static int thrust_debug_counter = 0;
    if (++thrust_debug_counter % 30 == 0) {
        printf("\n🔍 === SCRIPTED FLIGHT DEBUG ===\n");
        printf("📍 Entity %d: waypoint %d/%d, dist=%.1f\n",
               entity_id, flight->current_waypoint, flight->path.waypoint_count, distance);
        printf("📍 Position: [%.1f,%.1f,%.1f] → Target: [%.1f,%.1f,%.1f]\n",
               current_pos.x, current_pos.y, current_pos.z,
               target_pos.x, target_pos.y, target_pos.z);
        printf("🚀 Thrust CMD: [%.2f,%.2f,%.2f] (normalized -1 to 1)\n",
               thrust_command.x, thrust_command.y, thrust_command.z);
        printf("🚀 Linear Force: [%.1f,%.1f,%.1f] N\n",
               linear_force.x, linear_force.y, linear_force.z);
        printf("📊 Velocity: current=[%.1f,%.1f,%.1f] desired=[%.1f,%.1f,%.1f]\n",
               physics->velocity.x, physics->velocity.y, physics->velocity.z,
               desired_velocity.x, desired_velocity.y, desired_velocity.z);
        printf("📊 Speed: current=%.1f, desired=%.1f\n",
               flight->current_speed, desired_speed);
        
        // Check thruster state
        printf("⚙️ Thruster state: enabled=%d, max_force.z=%.1f\n",
               thrusters->thrusters_enabled, thrusters->max_linear_force.z);
        printf("⚙️ Current thrust in thruster: [%.2f,%.2f,%.2f]\n",
               thrusters->current_linear_thrust.x,
               thrusters->current_linear_thrust.y,
               thrusters->current_linear_thrust.z);
        printf("=================================\n\n");
    }
    
    flight->current_speed = vector3_length(physics->velocity);
    flight->state_timer += delta_time;
}

// ============================================================================
// SYSTEM UPDATE
// ============================================================================

void scripted_flight_update(struct World* world, RenderConfig* render_config, float delta_time) {
    (void)render_config; // Unused
    
    static int update_counter = 0;
    if (++update_counter % 60 == 0) {
        printf("🔍 DEBUG: scripted_flight_update called, count=%d\n", g_scripted_flight_count);
        for (int i = 0; i < g_scripted_flight_count; i++) {
            printf("  - Entity %d: active=%d\n", 
                   g_scripted_flights[i].entity_id,
                   g_scripted_flights[i].flight.active);
        }
    }
    
    for (int i = 0; i < g_scripted_flight_count; i++) {
        update_scripted_entity(world, g_scripted_flights[i].entity_id, 
                             &g_scripted_flights[i].flight, delta_time);
    }
}

// ============================================================================
// DEBUG FUNCTIONS
// ============================================================================

void scripted_flight_debug_render(struct World* world, const ScriptedFlight* flight) {
    (void)world; // For now, just print debug info
    
    if (!flight || !flight->active) return;
    
    static float debug_timer = 0.0f;
    debug_timer += 0.016f; // Assume ~60fps
    
    if (debug_timer > 2.0f) { // Print every 2 seconds
        printf("🛩️  DEBUG: Waypoint %d/%d, Speed: %.1f, Active: %s, Override: %s\n",
               flight->current_waypoint, flight->path.waypoint_count,
               flight->current_speed,
               flight->active ? "YES" : "NO",
               flight->manual_override ? "YES" : "NO");
        debug_timer = 0.0f;
    }
}