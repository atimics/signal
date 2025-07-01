#include "physics.h"

#include <stdio.h>

#include "core.h"

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

        // Apply acceleration to velocity
        physics->velocity =
            vector3_add(physics->velocity, vector3_multiply(physics->acceleration, delta_time));

        // Apply drag
        physics->velocity = vector3_multiply(physics->velocity, physics->drag);

        // Apply velocity to position
        transform->position =
            vector3_add(transform->position, vector3_multiply(physics->velocity, delta_time));

        transform->dirty = true;
        updates++;
    }

    // Only log occasionally to avoid spam
    static uint32_t log_counter = 0;
    if (++log_counter % 600 == 0)
    {
        printf("🔧 Physics: Updated %d entities\n", updates);
    }
}
