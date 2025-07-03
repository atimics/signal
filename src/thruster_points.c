#include "thruster_points.h"
#include "assets.h"  // For full Mesh definition
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Load thruster definitions from file
bool thruster_points_load(ThrusterArray* thrusters, const char* filename) {
    if (!thrusters || !filename) return false;
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("❌ Failed to open thruster file: %s\n", filename);
        return false;
    }
    
    // Initialize array
    thrusters->capacity = 32;
    thrusters->count = 0;
    thrusters->points = calloc(thrusters->capacity, sizeof(ThrusterPoint));
    
    char line[256];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        
        ThrusterPoint point = {0};
        
        // Parse: name, pos_x, pos_y, pos_z, dir_x, dir_y, dir_z, max_force
        int items = sscanf(line, "%31s %f %f %f %f %f %f %f",
                          point.name,
                          &point.position.x, &point.position.y, &point.position.z,
                          &point.direction.x, &point.direction.y, &point.direction.z,
                          &point.max_force);
        
        if (items != 8) {
            printf("⚠️ Line %d: Invalid format (expected 8 items, got %d)\n", line_num, items);
            continue;
        }
        
        // Normalize direction
        float len = vector3_length(point.direction);
        if (len < 0.001f) {
            printf("⚠️ Line %d: Zero-length direction vector\n", line_num);
            continue;
        }
        point.direction = vector3_multiply_scalar(point.direction, 1.0f / len);
        
        // Add to array
        if (thrusters->count >= thrusters->capacity) {
            thrusters->capacity *= 2;
            thrusters->points = realloc(thrusters->points, 
                                      thrusters->capacity * sizeof(ThrusterPoint));
        }
        
        thrusters->points[thrusters->count++] = point;
        
        printf("✅ Loaded thruster: %s at (%.2f,%.2f,%.2f) dir:(%.2f,%.2f,%.2f) %.0fN\n",
               point.name, 
               point.position.x, point.position.y, point.position.z,
               point.direction.x, point.direction.y, point.direction.z,
               point.max_force);
    }
    
    fclose(file);
    
    printf("📋 Loaded %d thrusters from %s\n", thrusters->count, filename);
    return thrusters->count > 0;
}

// Validate thruster positions against mesh vertices
bool thruster_points_validate(const ThrusterArray* thrusters, 
                             const Mesh* mesh,
                             float tolerance) {
    if (!thrusters || !mesh || !mesh->vertices) return false;
    
    bool all_valid = true;
    
    for (int i = 0; i < thrusters->count; i++) {
        const ThrusterPoint* point = &thrusters->points[i];
        bool found = false;
        float min_distance = INFINITY;
        int closest_vertex = -1;
        
        // Find closest vertex
        for (int v = 0; v < mesh->vertex_count; v++) {
            Vector3 diff = vector3_subtract(mesh->vertices[v].position, point->position);
            float dist = vector3_length(diff);
            
            if (dist < min_distance) {
                min_distance = dist;
                closest_vertex = v;
            }
            
            if (dist <= tolerance) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            printf("❌ Thruster '%s' at (%.2f,%.2f,%.2f) not found on mesh!\n",
                   point->name, point->position.x, point->position.y, point->position.z);
            printf("   Closest vertex %d at distance %.3f (tolerance: %.3f)\n",
                   closest_vertex, min_distance, tolerance);
            all_valid = false;
        } else {
            printf("✅ Thruster '%s' validated at vertex (distance: %.3f)\n",
                   point->name, min_distance);
        }
    }
    
    return all_valid;
}

// Create thruster groups based on position and direction
void thruster_points_create_groups(ThrusterPointSystem* system) {
    if (!system) return;
    
    // Clear existing groups
    for (int g = 0; g < THRUSTER_GROUP_COUNT; g++) {
        free(system->groups[g].thruster_indices);
        system->groups[g].thruster_indices = calloc(system->thrusters.count, sizeof(int));
        system->groups[g].count = 0;
        system->groups[g].activation = 0.0f;
    }
    
    // Classify each thruster into groups
    for (int i = 0; i < system->thrusters.count; i++) {
        ThrusterPoint* point = &system->thrusters.points[i];
        
        // Main engines - large thrust pointing backward
        if (point->max_force > 2000.0f && point->direction.z < -0.8f) {
            system->groups[THRUSTER_GROUP_MAIN].thruster_indices[
                system->groups[THRUSTER_GROUP_MAIN].count++] = i;
        }
        
        // RCS thrusters - classify by direction
        if (point->max_force <= 2000.0f) {
            // Forward/backward
            if (point->direction.z > 0.8f) {
                system->groups[THRUSTER_GROUP_FORWARD].thruster_indices[
                    system->groups[THRUSTER_GROUP_FORWARD].count++] = i;
            } else if (point->direction.z < -0.8f) {
                system->groups[THRUSTER_GROUP_BACKWARD].thruster_indices[
                    system->groups[THRUSTER_GROUP_BACKWARD].count++] = i;
            }
            
            // Left/right
            if (point->direction.x > 0.8f) {
                system->groups[THRUSTER_GROUP_RIGHT].thruster_indices[
                    system->groups[THRUSTER_GROUP_RIGHT].count++] = i;
            } else if (point->direction.x < -0.8f) {
                system->groups[THRUSTER_GROUP_LEFT].thruster_indices[
                    system->groups[THRUSTER_GROUP_LEFT].count++] = i;
            }
            
            // Up/down
            if (point->direction.y > 0.8f) {
                system->groups[THRUSTER_GROUP_UP].thruster_indices[
                    system->groups[THRUSTER_GROUP_UP].count++] = i;
            } else if (point->direction.y < -0.8f) {
                system->groups[THRUSTER_GROUP_DOWN].thruster_indices[
                    system->groups[THRUSTER_GROUP_DOWN].count++] = i;
            }
            
            // Rotation groups - based on position and direction
            // This is simplified - real implementation would calculate torque contribution
            if (point->position.x > 0.5f && point->direction.z != 0) {
                system->groups[THRUSTER_GROUP_YAW_LEFT].thruster_indices[
                    system->groups[THRUSTER_GROUP_YAW_LEFT].count++] = i;
            } else if (point->position.x < -0.5f && point->direction.z != 0) {
                system->groups[THRUSTER_GROUP_YAW_RIGHT].thruster_indices[
                    system->groups[THRUSTER_GROUP_YAW_RIGHT].count++] = i;
            }
        }
    }
    
    // Calculate available thrust
    for (int axis = 0; axis < 3; axis++) {
        system->linear_thrust_available[axis] = 0;
        system->angular_thrust_available[axis] = 0;
    }
    
    for (int i = 0; i < system->thrusters.count; i++) {
        ThrusterPoint* point = &system->thrusters.points[i];
        
        // Linear thrust contribution
        system->linear_thrust_available[0] += fabsf(point->direction.x) * point->max_force;
        system->linear_thrust_available[1] += fabsf(point->direction.y) * point->max_force;
        system->linear_thrust_available[2] += fabsf(point->direction.z) * point->max_force;
        
        // Angular thrust contribution (simplified - assumes 1m moment arm)
        Vector3 r = point->position;
        Vector3 torque = vector3_cross(r, vector3_multiply_scalar(point->direction, point->max_force));
        system->angular_thrust_available[0] += fabsf(torque.x);
        system->angular_thrust_available[1] += fabsf(torque.y);
        system->angular_thrust_available[2] += fabsf(torque.z);
    }
    
    printf("📊 Thruster groups created:\n");
    for (int g = 0; g < THRUSTER_GROUP_COUNT; g++) {
        if (system->groups[g].count > 0) {
            printf("   Group %d: %d thrusters\n", g, system->groups[g].count);
        }
    }
}

// Apply thrust commands to groups
void thruster_points_set_linear_thrust(ThrusterPointSystem* system,
                                      const Vector3* thrust_command) {
    if (!system || !thrust_command) return;
    
    // Reset all thrusters
    for (int i = 0; i < system->thrusters.count; i++) {
        system->thrusters.points[i].current_thrust = 0.0f;
    }
    
    // Forward/backward
    if (thrust_command->z > 0.01f) {
        system->groups[THRUSTER_GROUP_FORWARD].activation = thrust_command->z;
    } else if (thrust_command->z < -0.01f) {
        system->groups[THRUSTER_GROUP_MAIN].activation = -thrust_command->z;
        system->groups[THRUSTER_GROUP_BACKWARD].activation = -thrust_command->z;
    }
    
    // Left/right
    if (thrust_command->x > 0.01f) {
        system->groups[THRUSTER_GROUP_RIGHT].activation = thrust_command->x;
    } else if (thrust_command->x < -0.01f) {
        system->groups[THRUSTER_GROUP_LEFT].activation = -thrust_command->x;
    }
    
    // Up/down
    if (thrust_command->y > 0.01f) {
        system->groups[THRUSTER_GROUP_UP].activation = thrust_command->y;
    } else if (thrust_command->y < -0.01f) {
        system->groups[THRUSTER_GROUP_DOWN].activation = -thrust_command->y;
    }
    
    // Apply group activations to individual thrusters
    for (int g = 0; g < THRUSTER_GROUP_COUNT; g++) {
        ThrusterGroupMapping* group = &system->groups[g];
        for (int i = 0; i < group->count; i++) {
            int idx = group->thruster_indices[i];
            system->thrusters.points[idx].current_thrust = fmaxf(
                system->thrusters.points[idx].current_thrust,
                group->activation
            );
        }
        group->activation = 0; // Reset for next frame
    }
}

// Calculate total forces and torques
void thruster_points_calculate_forces(const ThrusterPointSystem* system,
                                     const struct Transform* transform,
                                     Vector3* total_force,
                                     Vector3* total_torque) {
    if (!system || !transform || !total_force || !total_torque) return;
    
    *total_force = (Vector3){0, 0, 0};
    *total_torque = (Vector3){0, 0, 0};
    
    for (int i = 0; i < system->thrusters.count; i++) {
        const ThrusterPoint* point = &system->thrusters.points[i];
        
        if (point->current_thrust < 0.001f) continue;
        
        // Calculate thrust force in local space
        Vector3 local_force = vector3_multiply_scalar(
            point->direction,
            point->max_force * point->current_thrust
        );
        
        // Transform to world space
        Vector3 world_force = quaternion_rotate_vector(transform->rotation, local_force);
        Vector3 world_position = quaternion_rotate_vector(transform->rotation, point->position);
        
        // Add to total force
        *total_force = vector3_add(*total_force, world_force);
        
        // Calculate torque (r × F)
        Vector3 torque = vector3_cross(world_position, world_force);
        *total_torque = vector3_add(*total_torque, torque);
    }
}

// Cleanup
void thruster_points_destroy(ThrusterArray* thrusters) {
    if (!thrusters) return;
    
    free(thrusters->points);
    thrusters->points = NULL;
    thrusters->count = 0;
    thrusters->capacity = 0;
}

void thruster_point_system_destroy(ThrusterPointSystem* system) {
    if (!system) return;
    
    thruster_points_destroy(&system->thrusters);
    
    for (int g = 0; g < THRUSTER_GROUP_COUNT; g++) {
        free(system->groups[g].thruster_indices);
    }
}