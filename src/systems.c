#include "systems.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "assets.h"
#include "data.h"
#include "graphics_api.h"
#include "render.h"

#include "system/physics.h"
#include "system/collision.h"
#include "system/ai.h"
#include "system/camera.h"

// Global asset and data registries
static AssetRegistry g_asset_registry;
static DataRegistry g_data_registry;

// ============================================================================
// SYSTEM SCHEDULER IMPLEMENTATION
// ============================================================================

bool scheduler_init(SystemScheduler* scheduler, RenderConfig* render_config)
{
    if (!scheduler) return false;

    memset(scheduler, 0, sizeof(struct SystemScheduler));

    // Initialize asset system first
    if (!assets_init(&g_asset_registry, "/Users/ratimics/develop/cgame/build/assets"))
    {
        printf("❌ Failed to initialize asset system\n");
        return false;
    }

    // Initialize data system
    if (!data_registry_init(&g_data_registry, "/Users/ratimics/develop/cgame/data"))
    {
        printf("❌ Failed to initialize data system\n");
        return false;
    }

    // Load entity and scene templates
    load_entity_templates(&g_data_registry, "templates/entities.txt");
    load_scene_templates(&g_data_registry, "scenes/logo.txt");         // Gold standard baseline scene
    load_scene_templates(&g_data_registry, "scenes/mesh_test.txt");
    load_scene_templates(&g_data_registry, "scenes/spaceport.txt");
    load_scene_templates(&g_data_registry, "scenes/camera_test.txt");

    // Initialize render system with asset registry FIRST
    if (!render_init(render_config, &g_asset_registry, 1200.0f, 800.0f))
    {
        printf("❌ Failed to initialize render system\n");
        return false;
    }

    // Set global render config for UI system
    set_render_config(render_config);

    // Set camera for zoomed-out solar system view
    camera_set_position(&render_config->camera,
                        (Vector3){ 0, 100, 300 });  // Position camera above and back

    // Load assets from files (now that we have a renderer for textures)
    printf("🔍 Loading assets...\n");
    assets_load_all_in_directory(&g_asset_registry);

    // Print loaded assets and templates
    assets_list_loaded(&g_asset_registry);
    list_entity_templates(&g_data_registry);
    list_scene_templates(&g_data_registry);

    // Initialize system configurations
    scheduler->systems[SYSTEM_PHYSICS] = (SystemInfo){ .name = "Physics",
                                                       .frequency = 60.0f,  // Every frame
                                                       .enabled = true,
                                                       .update_func = physics_system_update };

    scheduler->systems[SYSTEM_COLLISION] =
        (SystemInfo){ .name = "Collision",
                      .frequency = 20.0f,  // Every 3 frames at 60 FPS
                      .enabled = true,
                      .update_func = collision_system_update };

    scheduler->systems[SYSTEM_AI] =
        (SystemInfo){ .name = "AI",
                      .frequency = 5.0f,  // Base frequency, LOD will adjust
                      .enabled = true,
                      .update_func = ai_system_update };

    scheduler->systems[SYSTEM_CAMERA] = (SystemInfo){ .name = "Camera",
                                                      .frequency = 60.0f,  // Every frame
                                                      .enabled = true,
                                                      .update_func = camera_system_update };

    printf("🎯 System scheduler initialized\n");
    printf("   Physics: %.1f Hz\n", scheduler->systems[SYSTEM_PHYSICS].frequency);
    printf("   Collision: %.1f Hz\n", scheduler->systems[SYSTEM_COLLISION].frequency);
    printf("   AI: %.1f Hz (base)\n", scheduler->systems[SYSTEM_AI].frequency);
    printf("   Camera: %.1f Hz\n", scheduler->systems[SYSTEM_CAMERA].frequency);

    return true;
}

void scheduler_destroy(struct SystemScheduler* scheduler, RenderConfig* config)
{
    if (!scheduler) return;

    if (config)
    {
        render_cleanup(config);
    }
    assets_cleanup(&g_asset_registry);
    data_registry_cleanup(&g_data_registry);
    printf("🎯 System scheduler destroyed after %d frames\n", scheduler->frame_count);
    scheduler_print_stats(scheduler);
}

void scheduler_update(struct SystemScheduler* scheduler, struct World* world,
                      RenderConfig* render_config, float delta_time)
{
    if (!scheduler || !world) return;

    scheduler->total_time += delta_time;
    scheduler->frame_count++;

    // Update each system based on its frequency
    for (int i = 0; i < SYSTEM_COUNT; i++)
    {
        SystemInfo* system = &scheduler->systems[i];

        if (!system->enabled || !system->update_func) continue;

        float time_since_update = scheduler->total_time - system->last_update;
        float update_interval = 1.0f / system->frequency;

        if (time_since_update >= update_interval)
        {
            clock_t start = clock();

            system->update_func(world, render_config, delta_time);

            clock_t end = clock();
            float execution_time = ((float)(end - start)) / CLOCKS_PER_SEC;

            system->last_update = scheduler->total_time;
            scheduler->system_times[i] += execution_time;
            scheduler->system_calls[i]++;

            // Debug output every 300 frames (5 seconds at 60 FPS)
            if (scheduler->frame_count % 300 == 0 && i == SYSTEM_COLLISION)
            {
                printf("⚡ Frame %d: %s took %.2fms\n", scheduler->frame_count, system->name,
                       execution_time * 1000);
            }
        }
    }
}

void scheduler_print_stats(struct SystemScheduler* scheduler)
{
    if (!scheduler) return;

    printf("\n📊 SYSTEM PERFORMANCE STATS\n");
    printf("Total frames: %d, Total time: %.2fs\n", scheduler->frame_count, scheduler->total_time);

    for (int i = 0; i < SYSTEM_COUNT; i++)
    {
        if (scheduler->system_calls[i] > 0)
        {
            float avg_time = scheduler->system_times[i] / scheduler->system_calls[i];
            float actual_frequency = scheduler->system_calls[i] / scheduler->total_time;

            printf("  %s: %d calls, %.2fms avg, %.1f Hz actual\n", scheduler->systems[i].name,
                   scheduler->system_calls[i], avg_time * 1000, actual_frequency);
        }
    }
    printf("═══════════════════════════════════\n\n");
}

// ============================================================================
// SYSTEM CONTROL
// ============================================================================

void scheduler_enable_system(struct SystemScheduler* scheduler, SystemType type)
{
    if (scheduler && type < SYSTEM_COUNT)
    {
        scheduler->systems[type].enabled = true;
    }
}

void scheduler_disable_system(struct SystemScheduler* scheduler, SystemType type)
{
    if (scheduler && type < SYSTEM_COUNT)
    {
        scheduler->systems[type].enabled = false;
    }
}

void scheduler_set_frequency(struct SystemScheduler* scheduler, SystemType type, float frequency)
{
    if (scheduler && type < SYSTEM_COUNT && frequency > 0)
    {
        scheduler->systems[type].frequency = frequency;
    }
}

// ============================================================================
// GLOBAL REGISTRY ACCESS - TO BE REMOVED AFTER REFACTOR
// ============================================================================

DataRegistry* get_data_registry(void)
{
    return &g_data_registry;
}

AssetRegistry* get_asset_registry(void)
{
    return &g_asset_registry;
}
