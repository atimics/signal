#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "core.h"
#include "systems.h"
#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static struct {
    struct World world;
    struct SystemScheduler scheduler;
    EntityID player_id;
    sg_pass_action pass_action;
} state;

void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    printf("🎮 CGGame - Sokol Initialized\n");
    printf("===================================\n\n");

    srand((unsigned int)time(NULL));

    if (!world_init(&state.world)) {
        printf("❌ Failed to initialize world\n");
        sapp_quit();
    }

    if (!scheduler_init(&state.scheduler)) {
        printf("❌ Failed to initialize scheduler\n");
        sapp_quit();
    }
    
    const char* scene_to_load = "mesh_test";
    printf("ℹ️ Loading default scene: %s\n", scene_to_load);

    // Simplified scene loading from test.c
    printf("🏗️  Loading scene '%s' from data...\n", scene_to_load);
    DataRegistry* data_registry = get_data_registry();
    if (!load_scene(&state.world, data_registry, scene_to_load)) {
        printf("❌ Failed to load scene: %s\n", scene_to_load);
        sapp_quit();
    }
    
    // Find player
    state.player_id = INVALID_ENTITY;
    for (uint32_t i = 0; i < state.world.entity_count; i++) {
        struct Entity* entity = &state.world.entities[i];
        if (entity->component_mask & COMPONENT_PLAYER) {
            state.player_id = entity->id;
            break;
        }
    }

    // Setup pass action to clear screen
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.05f, 0.1f, 1.0f } }
    };
    
    printf("\n🎮 Starting simulation...\n");
}

void frame(void) {
    const float dt = sapp_frame_duration();

    // Update world and systems
    world_update(&state.world, dt);
    scheduler_update(&state.scheduler, &state.world, dt);

    // Render
    sg_begin_pass(&(sg_pass){ .swapchain = sglue_swapchain(), .action = state.pass_action });
    // Drawing will happen here
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    scheduler_destroy(&state.scheduler);
    world_destroy(&state.world);
    sg_shutdown();
    printf("\n🏁 Simulation complete!\n");
}

void event(const sapp_event* ev) {
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_request_quit();
        }
    }
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 1280,
        .height = 720,
        .window_title = "CGGame (Sokol)",
        .logger.func = slog_func,
    };
}