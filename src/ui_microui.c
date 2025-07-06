/**
 * @file ui_microui.c
 * @brief Microui integration implementation for CGGame UI system
 */

#include "ui_microui.h"
#include "graphics_api.h"
#include "render.h"
#include "render_pass_guard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// ============================================================================
// GLOBAL UI CONTEXT
// ============================================================================

static UIContext g_ui_context = { 0 };

// ============================================================================
// DEFERRED JOB SYSTEM
// ============================================================================

typedef enum {
    JOB_NONE,
    JOB_RECREATE_UI_BUFFERS
} deferred_job_t;

static deferred_job_t g_deferred_job = JOB_NONE;

static void request_ui_buffer_recreate(void) {
    g_deferred_job = JOB_RECREATE_UI_BUFFERS;
    printf("🔧 Requested deferred UI buffer recreation\n");
}

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

typedef struct {
    float x, y;        // position
    float u, v;        // texcoord
    uint32_t color;    // color as packed RGBA bytes
} ui_vertex_t;

static struct {
    bool ready;                // Set to true ONLY after all resources are valid
    sg_shader shader;          // Shader for both pipelines
    sg_pipeline pip;
    sg_pipeline offscreen_pip;  // Pipeline for offscreen rendering
    sg_bindings bind;
    sg_pass_action pass_action;
    ui_vertex_t* vertices;     // Dynamic heap buffer
    int vertex_count;
    int vertex_capacity;       // Current capacity
    int command_count;
    size_t vbuf_size;         // Current vertex buffer size in bytes
    sg_resource_state vbuf_state;  // Track buffer state
    bool graphics_context_valid;   // Track if we can safely use graphics
    bool need_resize;         // Request for buffer resize
    int requested_capacity;   // New capacity requested
} render_state;

// ============================================================================
// DYNAMIC BUFFER MANAGEMENT
// ============================================================================

// Check if buffers are valid (called during render pass)
static bool check_buffers_valid(void) {
    // Check if we have a valid graphics context
    if (!sg_isvalid()) {
        printf("❌ Cannot check buffers - Sokol context invalid\n");
        return false;
    }
    
    // Check if buffer ID is valid first
    if (render_state.bind.vertex_buffers[0].id == SG_INVALID_ID || 
        render_state.bind.vertex_buffers[0].id == 0) {
        printf("⚠️ UI vertex buffer has invalid ID %u, requesting deferred recreation\n", 
               render_state.bind.vertex_buffers[0].id);
        render_state.vbuf_state = SG_RESOURCESTATE_INVALID;
        request_ui_buffer_recreate();
        return false;
    }
    
    // Query the buffer state
    render_state.vbuf_state = sg_query_buffer_state(render_state.bind.vertex_buffers[0]);
    
    // If buffer is invalid, request deferred recreation
    if (render_state.vbuf_state != SG_RESOURCESTATE_VALID) {
        printf("⚠️ UI vertex buffer invalid (state=%d, id=%u), requesting deferred recreation\n", 
               render_state.vbuf_state, render_state.bind.vertex_buffers[0].id);
        request_ui_buffer_recreate();
        return false;
    }
    
    return true;
}

// Recreate UI buffers (called AFTER render pass ends)
static void recreate_ui_buffers(void) {
    printf("🔧 UI BUFFER RECREATE: Starting deferred buffer recreation...\n");
    
    // CRITICAL: Assert no render pass is active
    ASSERT_NO_PASS_ACTIVE();
    
    // Destroy old buffer if it exists
    if (render_state.bind.vertex_buffers[0].id != SG_INVALID_ID) {
        sg_destroy_buffer(render_state.bind.vertex_buffers[0]);
        printf("   Destroyed old buffer id=%u\n", render_state.bind.vertex_buffers[0].id);
    }
    
    // Create new buffer with sufficient size based on current vertex capacity
    // Use at least 2x the current capacity for headroom
    size_t min_capacity = render_state.vertex_capacity > 0 ? render_state.vertex_capacity : 8192;
    render_state.vbuf_size = min_capacity * sizeof(ui_vertex_t) * 2;
    
    printf("   Creating buffer for %zu vertices (%zu bytes)\n", min_capacity, render_state.vbuf_size);
    
    render_state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .size = render_state.vbuf_size,
        .usage = { .vertex_buffer = true, .dynamic_update = true },
        .label = "microui_vertex_buffer_recreated"
    });
    
    // Update state
    render_state.vbuf_state = sg_query_buffer_state(render_state.bind.vertex_buffers[0]);
    
    // If in ALLOC state, do initial update to make it VALID
    if (render_state.vbuf_state == SG_RESOURCESTATE_ALLOC) {
        struct {
            float x, y, u, v;
            uint32_t color;
        } dummy_vertex = {0};
        sg_update_buffer(render_state.bind.vertex_buffers[0], &(sg_range){
            .ptr = &dummy_vertex,
            .size = sizeof(dummy_vertex)
        });
        render_state.vbuf_state = sg_query_buffer_state(render_state.bind.vertex_buffers[0]);
    }
    
    if (render_state.vbuf_state == SG_RESOURCESTATE_VALID) {
        printf("✅ UI vertex buffer recreated successfully (id=%u, size=%zu)\n", 
               render_state.bind.vertex_buffers[0].id, render_state.vbuf_size);
    } else {
        printf("❌ Failed to create valid UI vertex buffer (state=%d)\n", render_state.vbuf_state);
    }
}

static bool ensure_ui_vbuf(size_t needed_bytes) {
    // If we have enough space, nothing to do
    if (needed_bytes <= render_state.vbuf_size) {
        return true;
    }
    
    // CRITICAL: On Metal, we cannot destroy/recreate buffers during a frame
    // Request deferred recreation instead
    printf("❌ CRITICAL: UI vertex buffer too small! Need %zu bytes but only have %zu\n", 
           needed_bytes, render_state.vbuf_size);
    
    request_ui_buffer_recreate();
    
    // Abort upload this frame to prevent buffer overflow
    printf("⚠️ Aborting UI upload this frame (buffer too small)\n");
    return false;  // Caller must skip sg_update_buffer()
}

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
// DEFERRED JOB PROCESSING
// ============================================================================

// Forward declaration - defined later
static void ui_apply_vertex_resize(void);

void ui_microui_process_deferred_jobs(void) {
    // First handle vertex array resize if needed
    ui_apply_vertex_resize();
    
    if (g_deferred_job == JOB_NONE) {
        return;
    }
    
    printf("🔧 Processing deferred UI jobs...\n");
    
    switch (g_deferred_job) {
        case JOB_RECREATE_UI_BUFFERS:
            recreate_ui_buffers();
            break;
        default:
            break;
    }
    
    g_deferred_job = JOB_NONE;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

// Apply deferred vertex array resize (called after sg_commit)
static void ui_apply_vertex_resize(void) {
    if (!render_state.need_resize) return;
    
    printf("🔧 UI DEFERRED RESIZE: Applying resize from %d to %d vertices\n", 
           render_state.vertex_capacity, render_state.requested_capacity);
    
    // Reallocate the vertex array
    ui_vertex_t* new_vertices = realloc(render_state.vertices, 
                                      render_state.requested_capacity * sizeof(ui_vertex_t));
    if (!new_vertices) {
        printf("❌ ERROR: Failed to allocate vertex array for %d vertices\n", 
               render_state.requested_capacity);
        return;
    }
    
    render_state.vertices = new_vertices;
    render_state.vertex_capacity = render_state.requested_capacity;
    render_state.need_resize = false;
    
    // Update the buffer size to match new capacity
    size_t new_buffer_size = render_state.vertex_capacity * sizeof(ui_vertex_t) * 2;
    if (new_buffer_size > render_state.vbuf_size) {
        render_state.vbuf_size = new_buffer_size;
        // Also recreate the GPU buffer with new size  
        request_ui_buffer_recreate();
    }
    
    printf("✅ Vertex array resized successfully (capacity=%d, buffer_size=%zu)\n",
           render_state.vertex_capacity, render_state.vbuf_size);
}

void ui_microui_init(void) {
    printf("🎨 UI INIT: Starting MicroUI initialization...\n");
    
    // Clear the render state to ensure clean initialization
    memset(&render_state, 0, sizeof(render_state));
    
    // Set initial capacities large enough for navigation menu (≈1000 quads)
    render_state.vertex_capacity = 4096;  // Sufficient for first menu
    render_state.vertices = malloc(render_state.vertex_capacity * sizeof(ui_vertex_t));
    if (!render_state.vertices) {
        printf("❌ ERROR: Failed to allocate initial vertex array\n");
        return;
    }
    printf("✅ UI INIT: Allocated vertex array - capacity=%d vertices, size=%zu bytes\n", 
           render_state.vertex_capacity, render_state.vertex_capacity * sizeof(ui_vertex_t));
    
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
    
    // Create shader that will be shared by both pipelines
    render_state.shader = sg_make_shader(&shd_desc);
    
    sg_pipeline_desc pip_desc = {
        .shader = render_state.shader,
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
            // Don't specify pixel_format - let it default to match swapchain
        },
        .depth = {
            .write_enabled = false,
            .compare = SG_COMPAREFUNC_ALWAYS
            // Don't specify pixel_format - let it default to match swapchain
        },
        .sample_count = 1,  // CRITICAL: Must be 1 to match both swapchain and offscreen targets
        .cull_mode = SG_CULLMODE_NONE,
        .face_winding = SG_FACEWINDING_CCW,
        .label = "microui_pipeline"
    };
    
    render_state.pip = sg_make_pipeline(&pip_desc);
    
    // Validate on-screen pipeline
    sg_resource_state pip_state = sg_query_pipeline_state(render_state.pip);
    if (pip_state != SG_RESOURCESTATE_VALID) {
        printf("❌ ERROR: MicroUI on-screen pipeline invalid! State: %d\n", pip_state);
    } else {
        printf("✅ MicroUI on-screen pipeline created successfully\n");
    }
    
    // Create offscreen pipeline with proper settings
    // CRITICAL: All fields must match the render target configuration
    sg_pipeline_desc offscreen_pip_desc = {
        .shader = render_state.shader,  // Reuse the same shader
        .layout = {
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT2 },  // position
                [1] = { .format = SG_VERTEXFORMAT_FLOAT2 },  // texcoord
                [2] = { .format = SG_VERTEXFORMAT_UBYTE4N }  // color
            }
        },
        .colors[0] = {
            .pixel_format = SG_PIXELFORMAT_RGBA8,      // Must match layer config
            .write_mask = SG_COLORMASK_RGBA,          // Write all channels
            .blend = {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
            }
        },
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL,  // Match layer config
            .write_enabled = false,
            .compare = SG_COMPAREFUNC_ALWAYS
        },
        .sample_count = 1,  // CRITICAL: Must match UI layer sample_count
        .cull_mode = SG_CULLMODE_NONE,
        .face_winding = SG_FACEWINDING_CCW,
        .label = "microui_offscreen_pipeline"
    };
    
    render_state.offscreen_pip = sg_make_pipeline(&offscreen_pip_desc);
    
    // Validate the offscreen pipeline state
    pip_state = sg_query_pipeline_state(render_state.offscreen_pip);
    if (pip_state != SG_RESOURCESTATE_VALID) {
        printf("❌ ERROR: MicroUI offscreen pipeline invalid! State: %d\n", pip_state);
    } else {
        printf("✅ MicroUI offscreen pipeline created successfully\n");
    }
    
    // Create vertex buffer with initial size
    // CRITICAL: For Metal, allocate a large buffer upfront to avoid recreation
    // Start with capacity * sizeof(vertex) * 2 for headroom
    render_state.vbuf_size = render_state.vertex_capacity * sizeof(ui_vertex_t) * 2;
    
    // Ensure buffer size is valid
    if (render_state.vbuf_size == 0) {
        printf("❌ ERROR: MicroUI vertex buffer size is 0! sizeof(vertices)=%zu\n", 
               sizeof(render_state.vertices));
        render_state.vbuf_size = 4 * 1024 * 1024; // 4MB fallback
    }
    
    printf("🔧 Creating MicroUI vertex buffer: size=%zu bytes (%.1f MB)\n", 
           render_state.vbuf_size, render_state.vbuf_size / (1024.0 * 1024.0));
    
    render_state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .size = render_state.vbuf_size,
        .usage = { .vertex_buffer = true, .dynamic_update = true },  // Dynamic buffer for frequent updates
        .label = "microui_vertex_buffer"
    });
    
    // Validate the vertex buffer was created successfully
    if (render_state.bind.vertex_buffers[0].id == SG_INVALID_ID) {
        printf("❌ ERROR: Failed to create MicroUI vertex buffer!\n");
    } else {
        sg_resource_state vbuf_state = sg_query_buffer_state(render_state.bind.vertex_buffers[0]);
        printf("🔧 MicroUI vertex buffer created with id=%u, initial state=%d\n", 
               render_state.bind.vertex_buffers[0].id, vbuf_state);
        
        // Store the initial state
        render_state.vbuf_state = vbuf_state;
        
        // CRITICAL: Dynamic buffers start in ALLOC state and need an initial update to become VALID
        if (vbuf_state == SG_RESOURCESTATE_ALLOC) {
            printf("🔧 Performing initial buffer update to transition from ALLOC to VALID state...\n");
            
            // CRITICAL: Assert no render pass is active
            ASSERT_NO_PASS_ACTIVE();
            
            // Create a small dummy vertex to initialize the buffer
            struct {
                float x, y;
                float u, v;
                uint32_t color;
            } dummy_vertex = {0};
            sg_update_buffer(render_state.bind.vertex_buffers[0], &(sg_range){
                .ptr = &dummy_vertex,
                .size = sizeof(dummy_vertex)
            });
            
            // Check state after update
            render_state.vbuf_state = sg_query_buffer_state(render_state.bind.vertex_buffers[0]);
            if (render_state.vbuf_state == SG_RESOURCESTATE_VALID) {
                printf("✅ MicroUI vertex buffer now VALID after initial update (id=%u, size=%zu)\n", 
                       render_state.bind.vertex_buffers[0].id, render_state.vbuf_size);
            } else {
                printf("❌ ERROR: MicroUI vertex buffer still in state %d after update!\n", render_state.vbuf_state);
            }
        } else if (vbuf_state != SG_RESOURCESTATE_VALID) {
            printf("❌ ERROR: MicroUI vertex buffer created but in unexpected state %d!\n", vbuf_state);
            printf("   State meanings: 0=INITIAL, 1=ALLOC, 2=VALID, 3=FAILED, 4=INVALID\n");
        }
    }
    
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
    
    // Validate all required resources are created successfully
    bool all_resources_valid = true;
    
    // Check vertex buffer
    if (render_state.bind.vertex_buffers[0].id == SG_INVALID_ID ||
        render_state.vbuf_state != SG_RESOURCESTATE_VALID) {
        printf("❌ Vertex buffer not valid for UI rendering\n");
        all_resources_valid = false;
    }
    
    // Check pipelines
    if (sg_query_pipeline_state(render_state.pip) != SG_RESOURCESTATE_VALID) {
        printf("❌ Main pipeline not valid for UI rendering\n");
        all_resources_valid = false;
    }
    
    if (sg_query_pipeline_state(render_state.offscreen_pip) != SG_RESOURCESTATE_VALID) {
        printf("❌ Offscreen pipeline not valid for UI rendering\n");
        all_resources_valid = false;
    }
    
    // Check shader
    if (sg_query_shader_state(render_state.shader) != SG_RESOURCESTATE_VALID) {
        printf("❌ Shader not valid for UI rendering\n");
        all_resources_valid = false;
    }
    
    // Check font texture
    if (sg_query_image_state(render_state.bind.images[0]) != SG_RESOURCESTATE_VALID) {
        printf("❌ Font texture not valid for UI rendering\n");
        all_resources_valid = false;
    }
    
    // Only mark as ready if ALL resources are valid
    render_state.ready = all_resources_valid;
    
    if (render_state.ready) {
        g_ui_context.initialized = true;
        printf("✅ Microui wrapper initialized successfully - renderer ready\n");
    } else {
        printf("❌ Microui wrapper initialization incomplete - renderer NOT ready\n");
    }
}

void ui_microui_shutdown(void) {
    if (g_ui_context.initialized) {
        sg_destroy_pipeline(render_state.pip);
        sg_destroy_pipeline(render_state.offscreen_pip);
        sg_destroy_shader(render_state.shader);
        sg_destroy_buffer(render_state.bind.vertex_buffers[0]);
        sg_destroy_image(render_state.bind.images[0]);
        sg_destroy_sampler(render_state.bind.samplers[0]);
        
        // Free the dynamic vertex array
        if (render_state.vertices) {
            free(render_state.vertices);
            render_state.vertices = NULL;
        }
        
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
    
    // Log command breakdown - ALWAYS for debugging
    if (render_state.command_count > 0) {
        printf("📊 UI COMMANDS: %d commands (%d rect, %d text, %d clip, %d icon) → %d vertices (capacity=%d)\n", 
               render_state.command_count, rect_count, text_count, clip_count, icon_count, 
               render_state.vertex_count, render_state.vertex_capacity);
        
        // Warn if we're getting close to capacity
        float usage = (float)render_state.vertex_count / render_state.vertex_capacity;
        if (usage > 0.8f) {
            printf("⚠️ UI CAPACITY WARNING: Using %.1f%% of vertex capacity!\n", usage * 100);
        }
    }
}

// ============================================================================
// RENDERING
// ============================================================================

static void push_vertex(float x, float y, float u, float v, mu_Color color) {
    // Safety check - ensure we have allocated vertex memory
    if (!render_state.vertices) {
        static int error_logged = 0;
        if (!error_logged) {
            printf("❌ ERROR: MicroUI vertex array is NULL!\n");
            error_logged = 1;
        }
        return;
    }
    
    // Check if we need more capacity
    if (render_state.vertex_count >= render_state.vertex_capacity) {
        // Request resize for next frame (double the capacity)
        int new_capacity = render_state.vertex_capacity ? render_state.vertex_capacity * 2 : 8192;
        if (new_capacity > render_state.requested_capacity) {
            render_state.requested_capacity = new_capacity;
            render_state.need_resize = true;
            printf("⚠️ UI RESIZE REQUEST: vertex_count=%d >= capacity=%d, requesting new_capacity=%d\n",
                   render_state.vertex_count, render_state.vertex_capacity, new_capacity);
        }
        // CRITICAL: Abort current frame to avoid overflow
        static int abort_logged = 0;
        if (abort_logged++ < 5) {
            printf("❌ UI UPLOAD ABORTED: Stopping vertex upload to prevent buffer overflow\n");
        }
        return;
    }
    
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

// Upload vertex data outside of any render pass
void ui_microui_upload_vertices(void) {
    // CRITICAL: Don't do anything if renderer isn't ready
    if (!render_state.ready) {
        static int skip_count = 0;
        if (skip_count++ % 60 == 0) {  // Log once per second
            printf("⚠️ UI: Upload skipped - renderer not ready\n");
        }
        return;
    }
    
    // Don't upload if we have no vertices
    if (render_state.vertex_count == 0) {
        return;
    }
    
    // Calculate upload size first
    const size_t upload_size = render_state.vertex_count * sizeof(ui_vertex_t);
    
    // Log upload attempt
    static int upload_id = 0;
    printf("📤 UI UPLOAD #%d: Uploading %d vertices (%zu bytes) to GPU buffer (size=%zu)\n",
           ++upload_id, render_state.vertex_count, upload_size, render_state.vbuf_size);
    
    // CRITICAL: Ensure buffer is large enough BEFORE any validation
    // This way if we need to recreate, we do it before checking validity
    if (!ensure_ui_vbuf(upload_size)) {
        return;  // Buffer too small, skip upload this frame
    }
    
    // CRITICAL: Check if context is valid before calling sg_update_buffer
    if (!sg_isvalid()) {
        printf("❌ CRITICAL: MicroUI Upload: Sokol context invalid! Cannot upload vertices.\n");
        return;
    }
    
    // CRITICAL: Assert no render pass is active - buffer updates must happen outside passes
    ASSERT_NO_PASS_ACTIVE();
    
    // Check if buffers are valid (will request deferred recreation if needed)
    if (!check_buffers_valid()) {
        printf("⚠️ UI buffers not valid, skipping upload this frame\n");
        return;
    }
    
    // CRITICAL: Double-check we're not writing beyond array bounds
    if (!render_state.vertices) {
        printf("❌ CRITICAL: Vertex array is NULL before upload!\n");
        return;
    }
    
    // Paranoid check - ensure we don't read beyond allocated memory
    if (render_state.vertex_count > render_state.vertex_capacity) {
        printf("❌ CRITICAL: vertex_count %d exceeds capacity %d!\n", 
               render_state.vertex_count, render_state.vertex_capacity);
        render_state.vertex_count = render_state.vertex_capacity;
    }
    
    // Recalculate upload size after potential clamping
    const size_t final_upload_size = render_state.vertex_count * sizeof(ui_vertex_t);
    if (final_upload_size > render_state.vbuf_size) {
        printf("❌ ERROR: Final upload size %zu exceeds buffer size %zu\n", 
               final_upload_size, render_state.vbuf_size);
        return;
    }
    
    // ALWAYS log upload attempts for debugging
    printf("📤 UI UPLOAD: Uploading %d vertices (%zu bytes to %zu byte buffer, id=%u)\n", 
           render_state.vertex_count, final_upload_size, render_state.vbuf_size,
           render_state.bind.vertex_buffers[0].id);
    
    // CRITICAL: Assert no encoder is active before buffer update
    #ifdef DEBUG
    // Include the layer system header for encoder check
    extern bool layer_is_encoder_active(void);
    if (layer_is_encoder_active()) {
        printf("❌ CRITICAL: Attempting sg_update_buffer while encoder is active! This will crash!\n");
        assert(!"sg_update_buffer called while encoder is active - this will crash!");
    }
    #endif
    
    // Upload vertex data to GPU (MUST be called outside any render pass)
    sg_update_buffer(render_state.bind.vertex_buffers[0], &(sg_range){
        .ptr = render_state.vertices,
        .size = final_upload_size
    });
    
    // Verify context is still valid after upload
    if (!sg_isvalid()) {
        printf("❌ CRITICAL: MicroUI Upload corrupted Sokol context! Upload size was %zu bytes\n", upload_size);
    }
}

void ui_microui_render(int screen_width, int screen_height) {
    // CRITICAL: Don't render if renderer isn't ready
    if (!render_state.ready) {
        return;
    }
    
    // Commands have already been processed in end_frame
    // Just apply state and draw the vertices (buffer upload happens separately)
    
    // Quick context validity check
    if (!sg_isvalid()) {
        printf("⚠️ MicroUI Render: Sokol context invalid - skipping render\n");
        return;
    }
    
    // DEBUG: Always log render calls to track the issue
    static int render_call_count = 0;
    render_call_count++;
    // Commented out for less spam
    // printf("🎨 MicroUI Render #%d: vertex_count=%d, commands=%d, screen=%dx%d, offscreen=%s\n", 
    //        render_call_count, render_state.vertex_count, render_state.command_count, 
    //        screen_width, screen_height, render_is_offscreen_mode() ? "yes" : "no");
    
    // CRITICAL FIX: Don't call any Sokol render functions if we have no vertices
    // This prevents corrupting the graphics context with empty draw calls
    if (render_state.vertex_count == 0) {
        // Only log occasionally to avoid spam
        if (render_call_count % 60 == 1) {
            printf("🎨 MicroUI: No vertices to render (frame %d)\n", render_call_count);
        }
        return;
    }
    
    // Ensure buffer is valid
    if (render_state.bind.vertex_buffers[0].id == SG_INVALID_ID) {
        printf("❌ MicroUI Render: Invalid vertex buffer - skipping render\n");
        return;
    }
    
    // Set up screen size uniforms
    float screen_size[2] = { (float)screen_width, (float)screen_height };
    
    // Apply appropriate pipeline based on render target
    sg_pipeline pip_to_use = render_is_offscreen_mode() ? render_state.offscreen_pip : render_state.pip;
    
    // Validate pipeline before applying
    sg_resource_state pip_state = sg_query_pipeline_state(pip_to_use);
    if (pip_state != SG_RESOURCESTATE_VALID) {
        printf("❌ ERROR: MicroUI pipeline invalid before apply! State: %d, offscreen: %s\n", 
               pip_state, render_is_offscreen_mode() ? "yes" : "no");
        return;
    }
    
    // Log occasionally to reduce spam
    static int apply_counter = 0;
    if (apply_counter++ % 60 == 0) {  // Once per second
        printf("🎨 MicroUI: Applying %s pipeline (vertices=%d)...\n", 
               render_is_offscreen_mode() ? "offscreen" : "swapchain", render_state.vertex_count);
    }
    
    sg_apply_pipeline(pip_to_use);
    
    // Check if context is still valid after applying pipeline
    if (!sg_isvalid()) {
        printf("❌ CRITICAL: sg_apply_pipeline corrupted Sokol context!\n");
        return;
    }
    
    sg_apply_bindings(&render_state.bind);
    sg_apply_uniforms(0, &SG_RANGE(screen_size));
    
    // Draw
    sg_draw(0, render_state.vertex_count, 1);
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
    
    // CRITICAL: Don't process events if Sokol context is invalid
    // This can happen during scene transitions or other state changes
    if (!sg_isvalid()) {
        return false;
    }
    
    // Check if queue is getting full (>80% capacity)
    float queue_usage = (float)g_ui_context.event_queue.count / UI_EVENT_QUEUE_SIZE;
    if (queue_usage > 0.8f) {
        static int overflow_warning_count = 0;
        if (overflow_warning_count++ < 5) {  // Limit spam
            printf("⚠️ MicroUI: Event queue at %.0f%% capacity, dropping old events\n", queue_usage * 100);
        }
        
        // Drop oldest events to make room
        int events_to_drop = g_ui_context.event_queue.count / 4;  // Drop 25% of queue
        memmove(&g_ui_context.event_queue.events[0], 
                &g_ui_context.event_queue.events[events_to_drop],
                (g_ui_context.event_queue.count - events_to_drop) * sizeof(sapp_event));
        g_ui_context.event_queue.count -= events_to_drop;
    }
    
    // Queue the event for processing during frame
    if (g_ui_context.event_queue.count < UI_EVENT_QUEUE_SIZE) {
        g_ui_context.event_queue.events[g_ui_context.event_queue.count++] = *ev;
        // Log only important events (not mouse moves which are very frequent)
        if (ev->type != SAPP_EVENTTYPE_MOUSE_MOVE) {
            static int event_log_counter = 0;
            if (event_log_counter++ % 10 == 0) {  // Reduce spam
                printf("🎨 MicroUI: Queue size: %d/%d (%.0f%%)\n", 
                       g_ui_context.event_queue.count, UI_EVENT_QUEUE_SIZE, queue_usage * 100);
            }
        }
        return true;  // Event queued successfully
    }
    
    // Queue is full, track overflow
    static int overflow_count = 0;
    overflow_count++;
    if (overflow_count % 100 == 1) {  // Report every 100 dropped events
        printf("❌ MicroUI: Event queue overflow! Total dropped: %d\n", overflow_count);
    }
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
            
            // NOTE: Pointer capture disabled - causes Metal context invalidation
            // sapp_lock_mouse(true);
            
            printf("🎨 MicroUI: Mouse down at (%.0f,%.0f) button=%d mu_button=%d\n", 
                   ev->mouse_x, ev->mouse_y, ev->mouse_button, 1 << ev->mouse_button);
            break;
            
        case SAPP_EVENTTYPE_MOUSE_UP:
            g_ui_context.mouse_buttons &= ~(1 << ev->mouse_button);
            mu_input_mouseup(ctx, ev->mouse_x, ev->mouse_y, 1 << ev->mouse_button);
            
            // NOTE: Pointer capture disabled - causes Metal context invalidation
            // if (g_ui_context.mouse_buttons == 0) {
            //     sapp_lock_mouse(false);
            // }
            
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
    size_t vertex_memory = render_state.vertex_capacity * sizeof(ui_vertex_t);
    return sizeof(g_ui_context) + sizeof(render_state) + vertex_memory;
}

// ============================================================================
// PUBLIC HELPERS FOR MAIN LOOP
// ============================================================================

bool ui_microui_ready(void) {
    return render_state.ready && sg_isvalid();
}

void ui_microui_init_renderer(void) {
    if (!render_state.ready) {
        ui_microui_init();  // Call existing init routine
    }
}

void ui_microui_end_of_frame(void) {
    ui_apply_vertex_resize();           // Deferred CPU array resize
    ui_microui_process_deferred_jobs(); // Deferred GPU buffer recreate
}