/**
 * Test program to diagnose magenta screen issue
 */

#include <stdio.h>
#include <stdbool.h>
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"

static struct {
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;
} state;

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment()
    });

    // Try different clear colors to identify the source
    state.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.0f, 1.0f, 0.0f, 1.0f }  // Green - if we see green, pass action works
        }
    };
    
    printf("✅ Test initialized - testing clear color\n");
}

static void frame(void) {
    // Just clear to see what color appears
    sg_begin_pass(&(sg_pass){
        .swapchain = sglue_swapchain(),
        .action = state.pass_action
    });
    sg_end_pass();
    sg_commit();
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
        .window_title = "Magenta Screen Test",
    };
}