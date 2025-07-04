/**
 * @file ui_microui.c
 * @brief Microui integration implementation for CGGame UI system
 */

#include "ui_microui.h"
#include "graphics_api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================================
// GLOBAL UI CONTEXT
// ============================================================================

static UIContext g_ui_context = { 0 };

// ============================================================================
// FONT DATA (8x8 bitmap font)
// ============================================================================

// Simple 8x8 ASCII font (space to ~) - commented out as currently unused
/*
static const unsigned char font_data[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Space
    {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00}, // !
    {0x66,0x66,0x66,0x00,0x00,0x00,0x00,0x00}, // "
    // ... (abbreviated for brevity - in real implementation, include full font)
};
*/

// ============================================================================
// RENDERING RESOURCES
// ============================================================================

static struct {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    float vertices[8192 * 8];  // Vertex buffer (x,y,u,v,r,g,b,a)
    int vertex_count;
} render_state;

// ============================================================================
// MICROUI CALLBACKS
// ============================================================================

static int text_width_callback(mu_Font font, const char *text, int len) {
    (void)font;
    if (len < 0) len = strlen(text);
    return len * 8;  // 8 pixels per character for simple font
}

static int text_height_callback(mu_Font font) {
    (void)font;
    return 8;  // 8 pixel height for simple font
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void ui_microui_init(void) {
    printf("🎨 Initializing Microui wrapper...\n");
    
    // Initialize Microui context
    mu_init(&g_ui_context.mu_ctx);
    
    // Set up text callbacks
    g_ui_context.mu_ctx.text_width = text_width_callback;
    g_ui_context.mu_ctx.text_height = text_height_callback;
    
    // Initialize font texture (simple white pixels for now)
    // Font texture is 128x128 pixels, each pixel is 4 bytes (RGBA)
    memset(g_ui_context.font_texture, 0xFF, 128 * 128 * 4);
    
    // Create rendering pipeline
    sg_shader_desc shd_desc = {
#ifdef SOKOL_METAL
        .vertex_func = {
            .source = 
                "#include <metal_stdlib>\n"
                "using namespace metal;\n"
                "struct vs_in {\n"
                "    float2 position [[attribute(0)]];\n"
                "    float2 texcoord [[attribute(1)]];\n"
                "    float4 color [[attribute(2)]];\n"
                "};\n"
                "struct vs_out {\n"
                "    float4 position [[position]];\n"
                "    float2 uv;\n"
                "    float4 color;\n"
                "};\n"
                "struct vs_uniforms {\n"
                "    float2 screen_size;\n"
                "};\n"
                "vertex vs_out _main(vs_in inp [[stage_in]], constant vs_uniforms& uniforms [[buffer(0)]]) {\n"
                "    vs_out outp;\n"
                "    float2 pos = inp.position / uniforms.screen_size;\n"
                "    pos = pos * 2.0 - 1.0;\n"
                "    pos.y = -pos.y;\n"
                "    outp.position = float4(pos, 0.0, 1.0);\n"
                "    outp.uv = inp.texcoord;\n"
                "    outp.color = inp.color;\n"
                "    return outp;\n"
                "}\n",
            .entry = "_main"
        },
        .fragment_func = {
            .source = 
                "#include <metal_stdlib>\n"
                "using namespace metal;\n"
                "struct fs_in {\n"
                "    float2 uv;\n"
                "    float4 color;\n"
                "};\n"
                "fragment float4 _main(fs_in inp [[stage_in]], texture2d<float> tex [[texture(0)]], sampler smp [[sampler(0)]]) {\n"
                "    return inp.color;\n"
                "}\n",
            .entry = "_main"
        },
#else
        .vertex_func = {
            .source = 
                "#version 330\n"
                "in vec2 position;\n"
                "in vec2 texcoord;\n"
                "in vec4 color;\n"
                "out vec2 uv;\n"
                "out vec4 frag_color;\n"
                "uniform vec2 screen_size;\n"
                "void main() {\n"
                "    vec2 pos = position / screen_size;\n"
                "    pos = pos * 2.0 - 1.0;\n"
                "    pos.y = -pos.y;\n"
                "    gl_Position = vec4(pos, 0.0, 1.0);\n"
                "    uv = texcoord;\n"
                "    frag_color = color;\n"
                "}\n",
            .entry = "main"
        },
        .fragment_func = {
            .source = 
                "#version 330\n"
                "in vec2 uv;\n"
                "in vec4 frag_color;\n"
                "out vec4 out_color;\n"
                "uniform sampler2D tex;\n"
                "void main() {\n"
                "    out_color = frag_color;\n"
                "}\n",
            .entry = "main"
        },
#endif
        .uniform_blocks[0] = {
            .stage = SG_SHADERSTAGE_VERTEX,
            .size = sizeof(float) * 2,
            .layout = SG_UNIFORMLAYOUT_NATIVE
        },
        .images[0] = {
            .stage = SG_SHADERSTAGE_FRAGMENT,
            .image_type = SG_IMAGETYPE_2D,
            .sample_type = SG_IMAGESAMPLETYPE_FLOAT
        },
        .samplers[0] = {
            .stage = SG_SHADERSTAGE_FRAGMENT,
            .sampler_type = SG_SAMPLERTYPE_FILTERING
        },
        .image_sampler_pairs[0] = {
            .stage = SG_SHADERSTAGE_FRAGMENT,
            .image_slot = 0,
            .sampler_slot = 0
        },
        .label = "microui_shader"
    };
    
    sg_pipeline_desc pip_desc = {
        .shader = sg_make_shader(&shd_desc),
        .layout = {
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT2 },  // position
                [1] = { .format = SG_VERTEXFORMAT_FLOAT2 },  // texcoord
                [2] = { .format = SG_VERTEXFORMAT_UBYTE4N }  // color
            }
        },
        .colors[0] = {
            .blend = {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
            }
        },
        .depth = {
            .write_enabled = false,
            .compare = SG_COMPAREFUNC_ALWAYS
        }
    };
    
    render_state.pip = sg_make_pipeline(&pip_desc);
    
    // Create vertex buffer
    render_state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(render_state.vertices),
        .usage = { .vertex_buffer = true, .dynamic_update = true }
    });
    
    // Create font texture
    render_state.bind.images[0] = sg_make_image(&(sg_image_desc){
        .width = 128,
        .height = 128,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = g_ui_context.font_texture,
            .size = 128 * 128 * 4
        }
    });
    
    // Create sampler
    render_state.bind.samplers[0] = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE
    });
    
    g_ui_context.initialized = true;
    printf("✅ Microui wrapper initialized\n");
}

void ui_microui_shutdown(void) {
    if (g_ui_context.initialized) {
        sg_destroy_pipeline(render_state.pip);
        sg_destroy_buffer(render_state.bind.vertex_buffers[0]);
        sg_destroy_image(render_state.bind.images[0]);
        sg_destroy_sampler(render_state.bind.samplers[0]);
        g_ui_context.initialized = false;
        printf("✅ Microui wrapper shut down\n");
    }
}

// ============================================================================
// FRAME MANAGEMENT
// ============================================================================

void ui_microui_begin_frame(void) {
    printf("🔍 DEBUG: MicroUI begin_frame called\n");
    
    mu_begin(&g_ui_context.mu_ctx);
    
    // Set root clip rect to cover the entire screen
    // This is needed because MicroUI expects at least one clip rect to be active
    mu_Rect screen_rect = mu_rect(0, 0, sapp_width(), sapp_height());
    mu_push_clip_rect(&g_ui_context.mu_ctx, screen_rect);
    
    printf("🔍 DEBUG: MicroUI begin_frame completed, clip_stack.idx=%d\n", g_ui_context.mu_ctx.clip_stack.idx);
}

void ui_microui_end_frame(void) {
    printf("🔍 DEBUG: MicroUI end_frame called, clip_stack.idx=%d\n", g_ui_context.mu_ctx.clip_stack.idx);
    
    // Pop the root clip rect we pushed in begin_frame
    mu_pop_clip_rect(&g_ui_context.mu_ctx);
    
    mu_end(&g_ui_context.mu_ctx);
    
    printf("🔍 DEBUG: MicroUI end_frame completed, clip_stack.idx=%d\n", g_ui_context.mu_ctx.clip_stack.idx);
}

// ============================================================================
// RENDERING
// ============================================================================

static void push_vertex(float x, float y, float u, float v, mu_Color color) {
    if (render_state.vertex_count >= 8192) return;
    
    int idx = render_state.vertex_count * 8;
    render_state.vertices[idx + 0] = x;
    render_state.vertices[idx + 1] = y;
    render_state.vertices[idx + 2] = u;
    render_state.vertices[idx + 3] = v;
    render_state.vertices[idx + 4] = color.r / 255.0f;
    render_state.vertices[idx + 5] = color.g / 255.0f;
    render_state.vertices[idx + 6] = color.b / 255.0f;
    render_state.vertices[idx + 7] = color.a / 255.0f;
    render_state.vertex_count++;
}

static void render_rect(mu_Rect rect, mu_Color color) {
    float x = rect.x;
    float y = rect.y;
    float w = rect.w;
    float h = rect.h;
    
    // Triangle 1
    push_vertex(x, y, 0, 0, color);
    push_vertex(x + w, y, 1, 0, color);
    push_vertex(x + w, y + h, 1, 1, color);
    
    // Triangle 2
    push_vertex(x, y, 0, 0, color);
    push_vertex(x + w, y + h, 1, 1, color);
    push_vertex(x, y + h, 0, 1, color);
}

void ui_microui_render(int screen_width, int screen_height) {
    render_state.vertex_count = 0;
    
    // Process all Microui commands
    mu_Command *cmd = NULL;
    while (mu_next_command(&g_ui_context.mu_ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_RECT:
                render_rect(cmd->rect.rect, cmd->rect.color);
                break;
                
            case MU_COMMAND_TEXT:
                // Simple text rendering - just draw a rect for now
                // In a full implementation, render actual text glyphs
                {
                    mu_Rect text_rect = {
                        cmd->text.pos.x,
                        cmd->text.pos.y,
                        text_width_callback(cmd->text.font, cmd->text.str, -1),
                        text_height_callback(cmd->text.font)
                    };
                    render_rect(text_rect, cmd->text.color);
                }
                break;
                
            case MU_COMMAND_ICON:
                // Icons not implemented yet
                break;
                
            case MU_COMMAND_CLIP:
                // Handle scissor test for clipping
                // For now, we'll acknowledge the clip command but not implement scissor testing
                // In a full implementation, this would set up proper clipping regions
                break;
        }
    }
    
    // Upload vertex data
    if (render_state.vertex_count > 0) {
        sg_update_buffer(render_state.bind.vertex_buffers[0], &(sg_range){
            .ptr = render_state.vertices,
            .size = render_state.vertex_count * 8 * sizeof(float)
        });
        
        // Set up uniforms
        float screen_size[2] = { (float)screen_width, (float)screen_height };
        
        // Draw
        sg_apply_pipeline(render_state.pip);
        sg_apply_bindings(&render_state.bind);
        sg_apply_uniforms(0, &SG_RANGE(screen_size));
        sg_draw(0, render_state.vertex_count, 1);
    }
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

bool ui_microui_handle_event(const void* event) {
    const sapp_event* ev = (const sapp_event*)event;
    mu_Context* ctx = &g_ui_context.mu_ctx;
    
    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            g_ui_context.mouse_x = ev->mouse_x;
            g_ui_context.mouse_y = ev->mouse_y;
            g_ui_context.mouse_dx = ev->mouse_dx;
            g_ui_context.mouse_dy = ev->mouse_dy;
            mu_input_mousemove(ctx, ev->mouse_x, ev->mouse_y);
            break;
            
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            g_ui_context.mouse_buttons |= (1 << ev->mouse_button);
            mu_input_mousedown(ctx, ev->mouse_x, ev->mouse_y, 1 << ev->mouse_button);
            break;
            
        case SAPP_EVENTTYPE_MOUSE_UP:
            g_ui_context.mouse_buttons &= ~(1 << ev->mouse_button);
            mu_input_mouseup(ctx, ev->mouse_x, ev->mouse_y, 1 << ev->mouse_button);
            break;
            
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            mu_input_scroll(ctx, 0, ev->scroll_y * 30);
            break;
            
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (ev->key_code < 512) {
                g_ui_context.keys_down[ev->key_code] = true;
                int mu_key = ui_microui_convert_keycode(ev->key_code);
                if (mu_key) {
                    mu_input_keydown(ctx, mu_key);
                }
            }
            break;
            
        case SAPP_EVENTTYPE_KEY_UP:
            if (ev->key_code < 512) {
                g_ui_context.keys_down[ev->key_code] = false;
                int mu_key = ui_microui_convert_keycode(ev->key_code);
                if (mu_key) {
                    mu_input_keyup(ctx, mu_key);
                }
            }
            break;
            
        case SAPP_EVENTTYPE_CHAR:
            if (ev->char_code < 128) {
                char str[2] = { (char)ev->char_code, 0 };
                mu_input_text(ctx, str);
            }
            break;
            
        default:
            break;
    }
    
    // Return true if mouse is over any UI element
    return mu_mouse_over(ctx, mu_rect(0, 0, sapp_width(), sapp_height()));
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

int ui_microui_convert_keycode(int sokol_keycode) {
    switch (sokol_keycode) {
        // MicroUI only supports these key constants, so we'll only handle them
        case SAPP_KEYCODE_ENTER:       return MU_KEY_RETURN;
        case SAPP_KEYCODE_BACKSPACE:   return MU_KEY_BACKSPACE;
        case SAPP_KEYCODE_LEFT_SHIFT:
        case SAPP_KEYCODE_RIGHT_SHIFT: return MU_KEY_SHIFT;
        case SAPP_KEYCODE_LEFT_CONTROL:
        case SAPP_KEYCODE_RIGHT_CONTROL: return MU_KEY_CTRL;
        case SAPP_KEYCODE_LEFT_ALT:
        case SAPP_KEYCODE_RIGHT_ALT:   return MU_KEY_ALT;
        // Arrow keys are not supported by MicroUI's key enum
        case SAPP_KEYCODE_LEFT:
        case SAPP_KEYCODE_RIGHT:
        case SAPP_KEYCODE_UP:
        case SAPP_KEYCODE_DOWN:
        default:                       return 0;
    }
}

int ui_microui_text_width(const char* text) {
    return text_width_callback(NULL, text, -1);
}

int ui_microui_text_height(void) {
    return text_height_callback(NULL);
}

// ============================================================================
// CONTEXT ACCESS
// ============================================================================

UIContext* ui_microui_get_context(void) {
    if (!g_ui_context.initialized) {
        printf("⚠️ Warning: UI context accessed before initialization!\n");
        return NULL;
    }
    return &g_ui_context;
}

mu_Context* ui_microui_get_mu_context(void) {
    if (!g_ui_context.initialized) {
        printf("⚠️ Warning: MicroUI context accessed before initialization!\n");
        return NULL;
    }
    return &g_ui_context.mu_ctx;
}