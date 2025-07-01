#include "physics.h"

#include <stdio.h>
#include <math.h>

void physics_system_update(struct World* world, RenderConfig* render_config, float delta_time)
{
    (void)render_config;  // Unused
    if (!world) return;

    uint32_t updates = 0;

    // Update all entities with physics + transform components
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];

        if (!(entity->component_mask & COMPONENT_PHYSICS) ||
            !(entity->component_mask & COMPONENT_TRANSFORM))
        {
            continue;
        }

        struct Physics* physics = entity->physics;
        struct Transform* transform = entity->transform;

        if (physics->kinematic) continue;  // Skip kinematic objects

        // Ground-effect racing mechanics for ships
        float ground_effect = 1.0f;
        float altitude = transform->position.y;
        
        // Ships get speed boost when flying close to ground (ground effect)
        if (altitude > 0.0f && altitude < 50.0f) {
            // Stronger ground effect at lower altitudes (inverse relationship)
            ground_effect = 1.0f + (50.0f - altitude) / 50.0f * 2.0f; // Up to 3x speed boost at ground level
            
            // Add some upward force to simulate ground effect lift
            Vector3 ground_lift = {0.0f, (50.0f - altitude) * 0.1f, 0.0f};
            physics->acceleration = vector3_add(physics->acceleration, ground_lift);
        }

        // Apply acceleration to velocity with ground effect multiplier
        Vector3 effective_acceleration = vector3_multiply(physics->acceleration, ground_effect);
        physics->velocity = vector3_add(physics->velocity, vector3_multiply(effective_acceleration, delta_time));

        // Apply drag (reduced at low altitude due to ground effect)
        float effective_drag = physics->drag + (1.0f - ground_effect) * 0.02f; // Less drag near ground
        physics->velocity = vector3_multiply(physics->velocity, effective_drag);

        // Apply velocity to position
        transform->position = vector3_add(transform->position, vector3_multiply(physics->velocity, delta_time));

        // Prevent going below ground
        if (transform->position.y < 1.0f) {
            transform->position.y = 1.0f;
            physics->velocity.y = fmaxf(0.0f, physics->velocity.y); // Stop downward velocity
        }

        transform->dirty = true;
        updates++;
    }

    // Only log occasionally to avoid spam
    static uint32_t log_counter = 0;
    if (++log_counter % 600 == 0)
    {
        printf("🔧 Physics: Updated %d entities (ground-effect racing enabled)\n", updates);
    }
}
