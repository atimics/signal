/**
 * @file ui_microui.h
 * @brief Microui integration wrapper for CGGame UI system
 * 
 * This provides a compatibility layer between Microui and the existing UI API,
 * allowing us to use Microui (pure C, WASM-compatible) instead of Nuklear.
 */

#ifndef UI_MICROUI_H
#define UI_MICROUI_H

#include "microui/microui.h"
#include "sokol_app.h"
#include <stdbool.h>

// Forward declarations
struct World;
struct SystemScheduler;

// ============================================================================
// MICROUI WRAPPER CONTEXT
// ============================================================================

// Event queue for buffering input events
// Increased from 64 to 512 to handle burst inputs (key-repeat + mouse move)
#define UI_EVENT_QUEUE_SIZE 512

typedef struct {
    sapp_event events[UI_EVENT_QUEUE_SIZE];
    int count;
} UIEventQueue;

typedef struct {
    mu_Context mu_ctx;          // Microui context
    bool initialized;           // Is the UI system initialized?
    
    // Input state for event handling
    int mouse_x, mouse_y;
    int mouse_dx, mouse_dy;
    int mouse_buttons;
    bool keys_down[512];        // Key state tracking
    
    // Event queue for deferred processing
    UIEventQueue event_queue;   // Queue events to process during frame
    
    // Rendering data
    unsigned char font_texture[128 * 128 * 4];  // Built-in font texture (RGBA)
} UIContext;

// ============================================================================
// MICROUI INITIALIZATION
// ============================================================================

// Initialize the Microui wrapper
void ui_microui_init(void);

// Shutdown the Microui wrapper
void ui_microui_shutdown(void);

// ============================================================================
// MICROUI FRAME MANAGEMENT
// ============================================================================

// Begin a new UI frame
void ui_microui_begin_frame(void);

// End the current UI frame and prepare for rendering
void ui_microui_end_frame(void);

// ============================================================================
// MICROUI RENDERING
// ============================================================================

// Upload vertex data to GPU (must be called outside any render pass)
void ui_microui_upload_vertices(void);

// Render the UI to screen using Sokol (called within an active render pass)
void ui_microui_render(int screen_width, int screen_height);

// Process any deferred jobs (must be called AFTER sg_commit)
void ui_microui_process_deferred_jobs(void);

// ============================================================================
// PUBLIC HELPERS FOR MAIN LOOP
// ============================================================================

// Check if UI renderer is ready (buffers & pipeline are VALID)
bool ui_microui_ready(void);

// One-shot renderer initialization (call at boot before any scenes)
void ui_microui_init_renderer(void);

// Call after sg_commit() each frame for deferred operations
void ui_microui_end_of_frame(void);

// ============================================================================
// MICROUI INPUT HANDLING
// ============================================================================

// Handle input events - returns true if UI captured the event
bool ui_microui_handle_event(const void* event);

// ============================================================================
// MICROUI HELPER FUNCTIONS
// ============================================================================

// Convert key codes from Sokol to Microui format
int ui_microui_convert_keycode(int sokol_keycode);

// Get text width for layout calculations
int ui_microui_text_width(const char* text);

// Get text height for layout calculations  
int ui_microui_text_height(void);

// ============================================================================
// GLOBAL UI CONTEXT ACCESS
// ============================================================================

// Get the global UI context
UIContext* ui_microui_get_context(void);

// Get the Microui context directly
mu_Context* ui_microui_get_mu_context(void);

// Test helpers - get render state info
int ui_microui_get_rendered_vertex_count(void);
int ui_microui_get_rendered_command_count(void);

#endif // UI_MICROUI_H