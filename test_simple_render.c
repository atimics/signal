/**
 * Simple test to check basic rendering without layer system
 */

#include <stdio.h>
#include <stdbool.h>
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"

static struct {
    sg_pass_action pass_action;
} state;

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment()
    });

    // Set a dark blue clear color (same as main game)
    state.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.0f, 0.05f, 0.1f, 1.0f }
        }
    };
    
    printf("✅ Simple render test initialized\n");
}

static void frame(void) {
    // Just clear the screen with our color
    sg_begin_pass(&(sg_pass){
        .swapchain = sglue_swapchain(),
        .action = state.pass_action
    });
    sg_end_pass();
    sg_commit();
    
    static int frame_count = 0;
    if (++frame_count % 60 == 0) {
        printf("✅ Frame %d rendered - should see dark blue screen\n", frame_count);
    }
}

static void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .width = 640,
        .height = 480,
        .window_title = "Simple Render Test",
    };
}