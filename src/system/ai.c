#include "ai.h"

#include <stdio.h>
#include <stdlib.h>

#include "core.h"

void ai_system_update(struct World* world, RenderConfig* render_config, float delta_time)
{
    (void)render_config;  // Unused
    if (!world) return;

    // Suppress unused parameter warning
    (void)delta_time;

    uint32_t ai_updates = 0;

    // Find player entity for LOD calculations
    EntityID player_id = INVALID_ENTITY;
    Vector3 player_pos = { 0, 0, 0 };

    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        if (world->entities[i].component_mask & COMPONENT_PLAYER)
        {
            player_id = world->entities[i].id;
            if (world->entities[i].transform)
            {
                player_pos = world->entities[i].transform->position;
            }
            break;
        }
    }

    // Update AI entities
    for (uint32_t i = 0; i < world->entity_count; i++)
    {
        struct Entity* entity = &world->entities[i];

        if (!(entity->component_mask & COMPONENT_AI) ||
            !(entity->component_mask & COMPONENT_TRANSFORM))
        {
            continue;
        }

        struct AI* ai = entity->ai;

        // LOD: Adjust update frequency based on distance to player
        if (player_id != INVALID_ENTITY)
        {
            float distance = vector3_distance(entity->transform->position, player_pos);

            if (distance < 50.0f)
            {
                ai->update_frequency = 10.0f;  // Close: 10 Hz
            }
            else if (distance < 200.0f)
            {
                ai->update_frequency = 5.0f;  // Medium: 5 Hz
            }
            else
            {
                ai->update_frequency = 2.0f;  // Far: 2 Hz
            }
        }

        // Check if it's time to update this AI
        float time_since_update = world->total_time - ai->last_update;
        float update_interval = 1.0f / ai->update_frequency;

        if (time_since_update >= update_interval)
        {
            ai->last_update = world->total_time;
            ai_updates++;

            // Simple AI logic for now
            switch (ai->state)
            {
                case AI_STATE_IDLE:
                    // Maybe switch to patrolling
                    if (world->total_time - ai->decision_timer > 5.0f)
                    {
                        ai->state = AI_STATE_PATROLLING;
                        ai->decision_timer = world->total_time;
                        printf("🤖 Entity %d: Idle -> Patrolling\n", entity->id);
                    }
                    break;

                case AI_STATE_PATROLLING:
                    // Simple patrol movement
                    if (entity->physics)
                    {
                        entity->physics->velocity.x += (rand() % 20 - 10) * 0.1f;
                        entity->physics->velocity.z += (rand() % 20 - 10) * 0.1f;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    // Log AI stats every 10 seconds
    static uint32_t log_counter = 0;
    if (++log_counter % 50 == 0)
    {
        printf("🧠 AI: Updated %d entities\n", ai_updates);
    }
}
