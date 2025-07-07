/**
 * @file test_microui_debug.c
 * @brief Minimal test program to debug MicroUI vertex generation issue
 */

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "ui_microui.h"
#include "microui/microui.h"
#include <stdio.h>

static void init(void) {
    printf("🧪 Test MicroUI Debug - Initializing\n");
    
    // Initialize Sokol GFX
    sg_desc desc = {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    };
    sg_setup(&desc);
    
    // Initialize MicroUI
    ui_microui_init();
    
    printf("✅ Initialization complete\n");
}

static void frame(void) {
    static int frame_count = 0;
    frame_count++;
    
    if (frame_count % 60 == 0) {
        printf("\n📊 Frame %d\n", frame_count);
    }
    
    // Get MicroUI context
    mu_Context* ctx = ui_microui_get_mu_context();
    if (!ctx) {
        printf("❌ No MicroUI context!\n");
        return;
    }
    
    // Begin frame
    printf("🎨 Begin frame (clip_stack.idx before: %d)\n", ctx->clip_stack.idx);
    ui_microui_begin_frame();
    printf("🎨 After begin_frame (clip_stack.idx: %d)\n", ctx->clip_stack.idx);
    
    // TEST 1: Simple button
    printf("🎨 Creating test button\n");
    if (mu_button(ctx, "Test Button")) {
        printf("🎉 Button clicked!\n");
    }
    
    // TEST 2: Window with content
    printf("🎨 Creating test window\n");
    if (mu_begin_window(ctx, "Test Window", mu_rect(100, 100, 300, 200))) {
        printf("🎨 Inside window\n");
        
        mu_layout_row(ctx, 1, (int[]){-1}, 0);
        mu_label(ctx, "Hello MicroUI!");
        
        if (mu_button(ctx, "Click Me!")) {
            printf("🎉 Window button clicked!\n");
        }
        
        mu_end_window(ctx);
    }
    
    // End frame
    printf("🎨 End frame (clip_stack.idx before: %d)\n", ctx->clip_stack.idx);
    ui_microui_end_frame();
    printf("🎨 After end_frame (clip_stack.idx: %d)\n", ctx->clip_stack.idx);
    
    // Clear screen
    sg_pass pass = {
        .swapchain = sglue_swapchain(),
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.2f, 0.3f, 0.4f, 1.0f }
            }
        }
    };
    sg_begin_pass(&pass);
    sg_end_pass();
    
    // Render UI
    sg_begin_pass(&(sg_pass){ 
        .swapchain = sglue_swapchain(), 
        .action = { .colors[0] = { .load_action = SG_LOADACTION_LOAD }}
    });
    ui_microui_render(sapp_width(), sapp_height());
    sg_end_pass();
    
    sg_commit();
}

static void cleanup(void) {
    printf("🧹 Cleaning up\n");
    ui_microui_shutdown();
    sg_shutdown();
}

static void event(const sapp_event* e) {
    ui_microui_handle_event(e);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 800,
        .height = 600,
        .window_title = "MicroUI Debug Test",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}