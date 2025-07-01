#include "system_collision.h"

#include <stdio.h>

#include "core.h"

void collision_system_update(struct World* world, RenderConfig* render_config, float delta_time)
{
    (void)render_config;  // Unused
    if (!world) return;

    // Suppress unused parameter warning
    (void)delta_time;

    uint32_t collision_checks = 0;
    uint32_t collisions_found = 0;

    // Simple O(n²) collision detection for now
    // TODO: Implement spatial partitioning (octree/grid)

    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity_a = &world->entities[i];

        if (!(entity_a->component_mask & COMPONENT_COLLISION) ||
            !(entity_a->component_mask & COMPONENT_TRANSFORM))
        {
            continue;
        }

        for (uint32_t j = i + 1; j < world->entity_count; j++)
        {
            struct Entity* entity_b = &world->entities[j];

            if (!(entity_b->component_mask & COMPONENT_COLLISION) ||
                !(entity_b->component_mask & COMPONENT_TRANSFORM))
            {
                continue;
            }

            collision_checks++;

            // Check if collision layers match
            struct Collision* col_a = entity_a->collision;
            struct Collision* col_b = entity_b->collision;

            if (!(col_a->layer_mask & col_b->layer_mask)) continue;

            // Only do sphere collision for now
            if (col_a->shape != COLLISION_SPHERE || col_b->shape != COLLISION_SPHERE)
            {
                continue;
            }

            float distance =
                vector3_distance(entity_a->transform->position, entity_b->transform->position);
            float combined_radius = col_a->radius + col_b->radius;

            if (distance < combined_radius)
            {
                collisions_found++;

                // Fire collision event (for now, just log but limit spam)
                static int collision_count = 0;
                if (collision_count < 10)
                {
                    printf("💥 Collision: Entity %d <-> Entity %d (dist: %.2f)\n", entity_a->id,
                           entity_b->id, distance);
                    collision_count++;
                }
                else if (collision_count == 10)
                {
                    printf("💥 ... (collision logging suppressed after first 10)\n");
                    collision_count++;
                }

                // Simple collision response - separate objects
                if (!col_a->is_trigger && !col_b->is_trigger)
                {
                    Vector3 separation = {
                        entity_a->transform->position.x - entity_b->transform->position.x,
                        entity_a->transform->position.y - entity_b->transform->position.y,
                        entity_a->transform->position.z - entity_b->transform->position.z
                    };

                    float sep_length = vector3_length(separation);
                    if (sep_length > 0)
                    {
                        separation = vector3_multiply(separation, 1.0f / sep_length);
                        float overlap = combined_radius - distance;

                        // Move objects apart
                        entity_a->transform->position =
                            vector3_add(entity_a->transform->position,
                                        vector3_multiply(separation, overlap * 0.5f));
                        entity_b->transform->position =
                            vector3_add(entity_b->transform->position,
                                        vector3_multiply(separation, -overlap * 0.5f));

                        // Stop velocities if they have physics
                        if (entity_a->physics)
                        {
                            entity_a->physics->velocity = (Vector3){ 0, 0, 0 };
                        }
                        if (entity_b->physics)
                        {
                            entity_b->physics->velocity = (Vector3){ 0, 0, 0 };
                        }
                    }
                }
            }
        }
    }

    // Log collision stats every 5 seconds
    static uint32_t log_counter = 0;
    if (++log_counter % 100 == 0)
    {
        printf("🔍 Collision: %d checks, %d hits\n", collision_checks, collisions_found);
    }
}