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

// Simple 8x8 ASCII font (space to ~) - basic but functional bitmap font
static const unsigned char font_data[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Space (32)
    {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00}, // ! (33)
    {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, // " (34)
    {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00}, // # (35)
    {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, // $ (36)
    {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00}, // % (37)
    {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00}, // & (38)
    {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00}, // ' (39)
    {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, // ( (40)
    {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00}, // ) (41)
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // * (42)
    {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00}, // + (43)
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, // , (44)
    {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00}, // - (45)
    {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, // . (46)
    {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00}, // / (47)
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, // 0 (48)
    {0x18,0x18,0x38,0x18,0x18,0x18,0x7E,0x00}, // 1 (49)
    {0x3C,0x66,0x06,0x0C,0x30,0x60,0x7E,0x00}, // 2 (50)
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // 3 (51)
    {0x06,0x0E,0x1E,0x66,0x7F,0x06,0x06,0x00}, // 4 (52)
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // 5 (53)
    {0x3C,0x66,0x60,0x7C,0x66,0x66,0x3C,0x00}, // 6 (54)
    {0x7E,0x66,0x0C,0x18,0x18,0x18,0x18,0x00}, // 7 (55)
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // 8 (56)
    {0x3C,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00}, // 9 (57)
    {0x00,0x00,0x18,0x00,0x00,0x18,0x00,0x00}, // : (58)
    {0x00,0x00,0x18,0x00,0x00,0x18,0x18,0x30}, // ; (59)
    {0x0E,0x18,0x30,0x60,0x30,0x18,0x0E,0x00}, // < (60)
    {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00}, // = (61)
    {0x70,0x18,0x0C,0x06,0x0C,0x18,0x70,0x00}, // > (62)
    {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00}, // ? (63)
    {0x3C,0x66,0x6E,0x6E,0x60,0x62,0x3C,0x00}, // @ (64)
    {0x18,0x3C,0x66,0x7E,0x66,0x66,0x66,0x00}, // A (65)
    {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, // B (66)
    {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, // C (67)
    {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, // D (68)
    {0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0x00}, // E (69)
    {0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0x00}, // F (70)
    {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00}, // G (71)
    {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, // H (72)
    {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // I (73)
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00}, // J (74)
    {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, // K (75)
    {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, // L (76)
    {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, // M (77)
    {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, // N (78)
    {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // O (79)
    {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, // P (80)
    {0x3C,0x66,0x66,0x66,0x66,0x3C,0x0E,0x00}, // Q (81)
    {0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00}, // R (82)
    {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00}, // S (83)
    {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, // T (84)
    {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, // U (85)
    {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, // V (86)
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // W (87)
    {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, // X (88)
    {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, // Y (89)
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, // Z (90)
    {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00}, // [ (91)
    {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00}, // \ (92)
    {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00}, // ] (93)
    {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00}, // ^ (94)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, // _ (95)
    {0x30,0x18,0x0C,0x00,0x00,0x00,0x00,0x00}, // ` (96)
    {0x00,0x00,0x3C,0x06,0x3E,0x66,0x3E,0x00}, // a (97)
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x7C,0x00}, // b (98)
    {0x00,0x00,0x3C,0x60,0x60,0x60,0x3C,0x00}, // c (99)
    {0x06,0x06,0x3E,0x66,0x66,0x66,0x3E,0x00}, // d (100)
    {0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, // e (101)
    {0x0E,0x18,0x18,0x7E,0x18,0x18,0x18,0x00}, // f (102)
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x7C}, // g (103)
    {0x60,0x60,0x7C,0x66,0x66,0x66,0x66,0x00}, // h (104)
    {0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00}, // i (105)
    {0x06,0x00,0x0E,0x06,0x06,0x06,0x66,0x3C}, // j (106)
    {0x60,0x60,0x66,0x6C,0x78,0x6C,0x66,0x00}, // k (107)
    {0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, // l (108)
    {0x00,0x00,0x66,0x7F,0x7F,0x6B,0x63,0x00}, // m (109)
    {0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00}, // n (110)
    {0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00}, // o (111)
    {0x00,0x00,0x7C,0x66,0x66,0x7C,0x60,0x60}, // p (112)
    {0x00,0x00,0x3E,0x66,0x66,0x3E,0x06,0x06}, // q (113)
    {0x00,0x00,0x7C,0x66,0x60,0x60,0x60,0x00}, // r (114)
    {0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00}, // s (115)
    {0x18,0x18,0x7E,0x18,0x18,0x18,0x0E,0x00}, // t (116)
    {0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, // u (117)
    {0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00}, // v (118)
    {0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00}, // w (119)
    {0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, // x (120)
    {0x00,0x00,0x66,0x66,0x66,0x3E,0x0C,0x78}, // y (121)
    {0x00,0x00,0x7E,0x0C,0x18,0x30,0x7E,0x00}, // z (122)
    {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00}, // { (123)
    {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00}, // | (124)
    {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00}, // } (125)
    {0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00}, // ~ (126)
};

// ============================================================================
// RENDERING RESOURCES
// ============================================================================

static struct {
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
    struct {
        float x, y;        // position
        float u, v;        // texcoord
        uint32_t color;    // color as packed RGBA bytes
    } vertices[8192];
    int vertex_count;
    int command_count;
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
    
    // Set up brighter color scheme for better visibility
    // Terminal-style green phosphor aesthetic - minimal visual elements
    g_ui_context.mu_ctx.style->colors[MU_COLOR_WINDOWBG] = (mu_Color){ 0, 0, 0, 200 };      // Semi-transparent black
    g_ui_context.mu_ctx.style->colors[MU_COLOR_BUTTON] = (mu_Color){ 0, 0, 0, 0 };          // Completely transparent
    g_ui_context.mu_ctx.style->colors[MU_COLOR_BUTTONHOVER] = (mu_Color){ 0, 0, 0, 0 };     // No hover effect
    g_ui_context.mu_ctx.style->colors[MU_COLOR_BUTTONFOCUS] = (mu_Color){ 0, 0, 0, 0 };     // No focus effect
    g_ui_context.mu_ctx.style->colors[MU_COLOR_TEXT] = (mu_Color){ 0, 255, 0, 255 };        // Bright green text
    g_ui_context.mu_ctx.style->colors[MU_COLOR_BORDER] = (mu_Color){ 0, 0, 0, 0 };          // No borders
    g_ui_context.mu_ctx.style->colors[MU_COLOR_PANELBG] = (mu_Color){ 0, 0, 0, 0 };         // Transparent panels
    
    // Initialize font texture with bitmap font data
    // Font texture is 128x128 pixels, each pixel is 4 bytes (RGBA)
    // We'll arrange the 95 characters in a 16x16 grid (only using first 95 slots)
    memset(g_ui_context.font_texture, 0x00, 128 * 128 * 4); // Clear to black/transparent
    
    // IMPORTANT: Create a solid white block at the bottom-right corner for rectangle rendering
    // We'll use the area from (120,120) to (127,127) as a solid white block
    for (int y = 120; y < 128; y++) {
        for (int x = 120; x < 128; x++) {
            int pixel_idx = (y * 128 + x) * 4;
            g_ui_context.font_texture[pixel_idx + 0] = 255; // R
            g_ui_context.font_texture[pixel_idx + 1] = 255; // G
            g_ui_context.font_texture[pixel_idx + 2] = 255; // B
            g_ui_context.font_texture[pixel_idx + 3] = 255; // A
        }
    }
    
    // Render each character into the texture
    for (int char_idx = 0; char_idx < 95; char_idx++) {
        int char_x = (char_idx % 16) * 8;  // 16 characters per row, 8 pixels wide each
        int char_y = (char_idx / 16) * 8;  // 8 pixels tall each
        
        // Render the 8x8 character bitmap
        for (int y = 0; y < 8; y++) {
            unsigned char row = font_data[char_idx][y];
            for (int x = 0; x < 8; x++) {
                int pixel_x = char_x + x;
                int pixel_y = char_y + y;
                int pixel_idx = (pixel_y * 128 + pixel_x) * 4; // 4 bytes per pixel (RGBA)
                
                if (row & (0x80 >> x)) { // Check if bit is set (pixel should be white)
                    g_ui_context.font_texture[pixel_idx + 0] = 255; // R
                    g_ui_context.font_texture[pixel_idx + 1] = 255; // G
                    g_ui_context.font_texture[pixel_idx + 2] = 255; // B
                    g_ui_context.font_texture[pixel_idx + 3] = 255; // A
                } else { // Pixel should be transparent
                    g_ui_context.font_texture[pixel_idx + 0] = 0;   // R
                    g_ui_context.font_texture[pixel_idx + 1] = 0;   // G
                    g_ui_context.font_texture[pixel_idx + 2] = 0;   // B
                    g_ui_context.font_texture[pixel_idx + 3] = 0;   // A
                }
            }
        }
    }
    
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
                "    float4 tex_color = tex.sample(smp, inp.uv);\n"
                "    return inp.color * tex_color;\n"
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
                "    vec4 tex_color = texture(tex, uv);\n"
                "    out_color = frag_color * tex_color;\n"
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
    
    // Initialize event queue
    g_ui_context.event_queue.count = 0;
    
    g_ui_context.initialized = true;
    printf("✅ Microui wrapper initialized with event queue\n");
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

// Forward declarations
static void ui_microui_process_event(const sapp_event* ev);
static void render_rect(mu_Rect rect, mu_Color color);
static void render_text(const char* text, mu_Vec2 pos, mu_Color color);

void ui_microui_begin_frame(void) {
    // printf("🎨 MicroUI: begin_frame called\n");
    
    if (!g_ui_context.initialized) {
        printf("❌ Error: MicroUI begin_frame called before initialization!\n");
        return;
    }
    
    // Clear any previous frame state and reset the context
    mu_begin(&g_ui_context.mu_ctx);
    // printf("🎨 MicroUI: mu_begin() called, clip_stack.idx=%d\n", g_ui_context.mu_ctx.clip_stack.idx);
    
    // Process all queued events now that we're in an active frame
    if (g_ui_context.event_queue.count > 0) {
        // Only log if it's not just mouse moves filling the queue
        int non_mouse_events = 0;
        for (int i = 0; i < g_ui_context.event_queue.count; i++) {
            if (g_ui_context.event_queue.events[i].type != SAPP_EVENTTYPE_MOUSE_MOVE) {
                non_mouse_events++;
            }
        }
        
        if (non_mouse_events > 0) {
            printf("🎨 MicroUI: Processing %d queued events (%d non-mouse)\n", 
                   g_ui_context.event_queue.count, non_mouse_events);
        }
        
        for (int i = 0; i < g_ui_context.event_queue.count; i++) {
            ui_microui_process_event(&g_ui_context.event_queue.events[i]);
        }
        
        // Clear the event queue
        g_ui_context.event_queue.count = 0;
    }
    
    // DIRECTLY push unclipped rect to clip stack (like begin_root_container does)
    // This avoids calling mu_push_clip_rect which itself calls mu_get_clip_rect
    mu_Rect unclipped_rect = { 0, 0, 0x1000000, 0x1000000 };
    
    // Manually push to clip stack (equivalent to push(ctx->clip_stack, unclipped_rect))
    if (g_ui_context.mu_ctx.clip_stack.idx < MU_CLIPSTACK_SIZE) {
        g_ui_context.mu_ctx.clip_stack.items[g_ui_context.mu_ctx.clip_stack.idx] = unclipped_rect;
        g_ui_context.mu_ctx.clip_stack.idx++;
    }
    
    // printf("🎨 MicroUI: directly pushed unclipped rect, clip_stack.idx=%d\n", g_ui_context.mu_ctx.clip_stack.idx);
    
    // Verify clip stack has at least one entry
    if (g_ui_context.mu_ctx.clip_stack.idx <= 0) {
        printf("❌ Error: MicroUI clip stack still empty after direct push! idx=%d\n", 
               g_ui_context.mu_ctx.clip_stack.idx);
    }
}

void ui_microui_end_frame(void) {
    // printf("🎨 MicroUI: end_frame called, clip_stack.idx=%d\n", g_ui_context.mu_ctx.clip_stack.idx);
    
    if (!g_ui_context.initialized) {
        printf("❌ Error: MicroUI end_frame called before initialization!\n");
        return;
    }
    
    // Debug: Verify clip stack has at least one entry before popping
    if (g_ui_context.mu_ctx.clip_stack.idx <= 0) {
        printf("❌ Error: MicroUI clip stack empty before popping! idx=%d\n", 
               g_ui_context.mu_ctx.clip_stack.idx);
        // Don't try to pop if stack is already empty
        mu_end(&g_ui_context.mu_ctx);
        return;
    }
    
    // Pop the root clip rect we pushed in begin_frame (manually)
    g_ui_context.mu_ctx.clip_stack.idx--;
    // printf("🎨 MicroUI: manually popped clip rect, clip_stack.idx=%d\n", g_ui_context.mu_ctx.clip_stack.idx);
    
    mu_end(&g_ui_context.mu_ctx);
    // printf("🎨 MicroUI: mu_end() called\n");
    
    // Process commands immediately after ending the frame
    // This ensures commands are available for rendering
    render_state.vertex_count = 0;
    render_state.command_count = 0;
    
    // Process all Microui commands
    mu_Command *cmd = NULL;
    int rect_count = 0, text_count = 0, clip_count = 0, icon_count = 0;
    while (mu_next_command(&g_ui_context.mu_ctx, &cmd)) {
        render_state.command_count++;
        switch (cmd->type) {
            case MU_COMMAND_RECT:
                rect_count++;
                render_rect(cmd->rect.rect, cmd->rect.color);
                break;
                
            case MU_COMMAND_TEXT:
                text_count++;
                // Render actual text using bitmap font
                render_text(cmd->text.str, cmd->text.pos, cmd->text.color);
                break;
                
            case MU_COMMAND_ICON:
                icon_count++;
                // Icons not implemented yet
                break;
                
            case MU_COMMAND_CLIP:
                clip_count++;
                // Handle scissor test for clipping
                // For now, we'll acknowledge the clip command but not implement scissor testing
                break;
        }
    }
    
    // Log command breakdown (reduced frequency)
    static int log_counter = 0;
    if (render_state.command_count > 0 && (log_counter++ % 60 == 0)) { // Log once per second at 60fps
        printf("🎨 MicroUI: %d commands (%d rect, %d text, %d clip, %d icon), %d vertices\n", 
               render_state.command_count, rect_count, text_count, clip_count, icon_count, render_state.vertex_count);
    }
}

// ============================================================================
// RENDERING
// ============================================================================

static void push_vertex(float x, float y, float u, float v, mu_Color color) {
    if (render_state.vertex_count >= 8192) return;
    
    render_state.vertices[render_state.vertex_count].x = x;
    render_state.vertices[render_state.vertex_count].y = y;
    render_state.vertices[render_state.vertex_count].u = u;
    render_state.vertices[render_state.vertex_count].v = v;
    
    // Pack color as RGBA bytes into uint32_t
    render_state.vertices[render_state.vertex_count].color = 
        ((uint32_t)color.r) |
        ((uint32_t)color.g << 8) |
        ((uint32_t)color.b << 16) |
        ((uint32_t)color.a << 24);
    
    render_state.vertex_count++;
}

static void render_text(const char* text, mu_Vec2 pos, mu_Color color) {
    float x = pos.x;
    float y = pos.y;
    
    while (*text) {
        char c = *text;
        if (c >= 32 && c <= 126) { // Printable ASCII range
            int char_idx = c - 32; // Convert to font_data index (space = 0)
            
            // Calculate texture coordinates for this character
            float char_x = (char_idx % 16) * 8.0f;  // 16 chars per row
            float char_y = (char_idx / 16) * 8.0f;  // 8 pixels tall
            
            // Normalize texture coordinates (0.0 to 1.0)
            float u1 = char_x / 128.0f;
            float v1 = char_y / 128.0f;
            float u2 = (char_x + 8.0f) / 128.0f;
            float v2 = (char_y + 8.0f) / 128.0f;
            
            // Render character as two triangles with proper texture coordinates
            // Triangle 1
            push_vertex(x, y, u1, v1, color);
            push_vertex(x + 8, y, u2, v1, color);
            push_vertex(x + 8, y + 8, u2, v2, color);
            
            // Triangle 2
            push_vertex(x, y, u1, v1, color);
            push_vertex(x + 8, y + 8, u2, v2, color);
            push_vertex(x, y + 8, u1, v2, color);
        }
        
        x += 8; // Advance to next character position
        text++;
    }
}

static void render_rect(mu_Rect rect, mu_Color color) {
    float x = rect.x;
    float y = rect.y;
    float w = rect.w;
    float h = rect.h;
    
    // printf("🎨 Rendering rect: (%d,%d,%d,%d) color:(%d,%d,%d,%d)\n", 
    //        rect.x, rect.y, rect.w, rect.h, color.r, color.g, color.b, color.a);
    
    // For solid rectangles, sample from the solid white block we created
    // We placed a solid white 8x8 block at (120,120) to (127,127)
    float u1 = 120.0f / 128.0f;   // Start of white block
    float v1 = 120.0f / 128.0f;   // Start of white block
    float u2 = 127.0f / 128.0f;   // End of white block
    float v2 = 127.0f / 128.0f;   // End of white block
    
    // Triangle 1
    push_vertex(x, y, u1, v1, color);
    push_vertex(x + w, y, u2, v1, color);
    push_vertex(x + w, y + h, u2, v2, color);
    
    // Triangle 2
    push_vertex(x, y, u1, v1, color);
    push_vertex(x + w, y + h, u2, v2, color);
    push_vertex(x, y + h, u1, v2, color);
}

void ui_microui_render(int screen_width, int screen_height) {
    // Commands have already been processed in end_frame
    // Just upload and render the vertices
    
    printf("🔍 MicroUI: Starting render - screen: %dx%d, vertices: %d\n", 
           screen_width, screen_height, render_state.vertex_count);
    
    // Quick context validity check
    if (!sg_isvalid()) {
        printf("❌ MicroUI: Sokol graphics context invalid, skipping render\n");
        return;
    }
    
    // Upload vertex data and render if we have vertices
    if (render_state.vertex_count > 0) {
        printf("🔍 MicroUI: Uploading %d vertices and rendering...\n", render_state.vertex_count);
        
        // Upload vertex data to GPU
        sg_update_buffer(render_state.bind.vertex_buffers[0], &(sg_range){
            .ptr = render_state.vertices,
            .size = render_state.vertex_count * sizeof(render_state.vertices[0])
        });
        
        // Set up screen size uniforms
        float screen_size[2] = { (float)screen_width, (float)screen_height };
        
        // Apply pipeline, bindings, uniforms and draw
        sg_apply_pipeline(render_state.pip);
        sg_apply_bindings(&render_state.bind);
        sg_apply_uniforms(0, &SG_RANGE(screen_size));
        sg_draw(0, render_state.vertex_count, 1);
        
        printf("✅ MicroUI: Rendered %d vertices successfully\n", render_state.vertex_count);
    } else {
        printf("🔍 MicroUI: No vertices to render\n");
    }
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

// External function to check if UI is visible (defined in ui.c)
extern bool ui_is_visible(void);

bool ui_microui_handle_event(const void* event) {
    const sapp_event* ev = (const sapp_event*)event;
    
    // Don't process events if MicroUI context isn't properly initialized
    if (!g_ui_context.initialized) {
        return false;
    }
    
    // Don't queue events when UI is not visible
    // This prevents the queue from filling up during logo scene
    if (!ui_is_visible()) {
        return false;
    }
    
    // Queue the event instead of processing it immediately
    // This fixes the timing issue where events arrive outside of active frames
    if (g_ui_context.event_queue.count < UI_EVENT_QUEUE_SIZE) {
        g_ui_context.event_queue.events[g_ui_context.event_queue.count++] = *ev;
        // Log only important events (not mouse moves which are very frequent)
        if (ev->type != SAPP_EVENTTYPE_MOUSE_MOVE) {
            printf("🎨 MicroUI: Queued event type %d (queue size: %d)\n", ev->type, g_ui_context.event_queue.count);
        }
        return true;  // Event queued successfully
    }
    
    // Queue is full, drop the event
    printf("🎨 MicroUI: WARNING - Event queue full, dropping event\n");
    return false;
}

// Internal function to process a queued event
static void ui_microui_process_event(const sapp_event* ev) {
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
            printf("🎨 MicroUI: Mouse down at (%.0f,%.0f) button=%d mu_button=%d\n", 
                   ev->mouse_x, ev->mouse_y, ev->mouse_button, 1 << ev->mouse_button);
            break;
            
        case SAPP_EVENTTYPE_MOUSE_UP:
            g_ui_context.mouse_buttons &= ~(1 << ev->mouse_button);
            mu_input_mouseup(ctx, ev->mouse_x, ev->mouse_y, 1 << ev->mouse_button);
            printf("🎨 MicroUI: Mouse up at (%.0f,%.0f) button=%d mu_button=%d\n", 
                   ev->mouse_x, ev->mouse_y, ev->mouse_button, 1 << ev->mouse_button);
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

// Test helpers
int ui_microui_get_rendered_vertex_count(void) {
    return render_state.vertex_count;
}

int ui_microui_get_rendered_command_count(void) {
    return render_state.command_count;
}

// Test utilities compatibility functions
int ui_microui_get_vertex_count(void) {
    return render_state.vertex_count;
}

int ui_microui_get_command_count(void) {
    return render_state.command_count;
}

int ui_microui_get_draw_call_count(void) {
    // Each command typically results in one draw call
    return render_state.command_count;
}

const void* ui_microui_get_vertex_data(void) {
    return render_state.vertices;
}

size_t ui_microui_get_vertex_data_size(void) {
    return render_state.vertex_count * sizeof(render_state.vertices[0]);
}

bool ui_microui_is_font_texture_bound(void) {
    return true; // Font texture is always bound during rendering
}

size_t ui_microui_get_memory_usage(void) {
    // Basic memory usage calculation
    return sizeof(g_ui_context) + sizeof(render_state) + 
           (render_state.vertex_count * sizeof(render_state.vertices[0]));
}