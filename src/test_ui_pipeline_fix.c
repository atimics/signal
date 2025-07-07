/**
 * Test program to verify UI pipeline fix
 */

#include <stdio.h>
#include <stdbool.h>
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "ui_microui.h"
#include "microui/microui.h"

static struct {
    sg_pass_action pass_action;
    bool ui_initialized;
} state;

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment()
    });

    state.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.2f, 0.3f, 0.3f, 1.0f }
        }
    };

    // Initialize UI
    ui_microui_init();
    state.ui_initialized = true;
    
    printf("✅ Test initialized - UI pipeline configured\n");
}

static void frame(void) {
    if (!state.ui_initialized) return;

    // Begin UI frame
    ui_microui_begin_frame();
    
    // Create a simple test UI
    mu_Context* ctx = ui_microui_get_context();
    if (mu_begin_window(ctx, "Test Window", mu_rect(100, 100, 300, 200))) {
        mu_layout_row(ctx, 1, (int[]){-1}, -1);
        mu_text(ctx, "UI Pipeline Test");
        mu_label(ctx, "This tests the fixed pipeline configuration");
        
        if (mu_button(ctx, "Test Button")) {
            printf("🎯 Button clicked!\n");
        }
        
        mu_end_window(ctx);
    }
    
    // End UI frame
    ui_microui_end_frame();

    // Render everything
    sg_begin_pass(&(sg_pass){
        .swapchain = sglue_swapchain(),
        .action = state.pass_action
    });
    
    // Check context validity before UI render
    if (!sg_isvalid()) {
        printf("❌ Context invalid before UI render\n");
        return;
    }
    
    // Render UI
    ui_microui_render(sapp_width(), sapp_height());
    
    // Check context validity after UI render
    if (!sg_isvalid()) {
        printf("❌ Context invalid after UI render\n");
        return;
    }
    
    // This is where the crash was happening - at sg_end_pass()
    sg_end_pass();
    
    // If we get here, the fix worked!
    static int frame_count = 0;
    if (++frame_count % 60 == 0) {
        printf("✅ Frame %d rendered successfully - UI pipeline fix confirmed!\n", frame_count);
    }
    
    sg_commit();
}

static void cleanup(void) {
    ui_microui_shutdown();
    sg_shutdown();
}

static void event(const sapp_event* ev) {
    ui_microui_handle_event(ev);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 800,
        .height = 600,
        .window_title = "UI Pipeline Fix Test",
    };
}