// src/graphics_api.h
#ifndef GRAPHICS_API_H
#define GRAPHICS_API_H

// This is the central hub for all Sokol graphics library includes.
// By including this single file, other modules can access the necessary
// graphics functionality without being tightly coupled to the Sokol implementation.

#include <stdint.h>

// Include the necessary Sokol headers in the correct order.
#include "sokol_gfx.h"

// Only include app layer if not running dummy backend for tests
#ifndef SOKOL_DUMMY_BACKEND
#include "sokol_app.h"
#include "sokol_glue.h"

// Nuklear UI library requires specific definitions before include
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <stdarg.h>
#define NK_INCLUDE_STANDARD_VARARGS
#include "nuklear.h"
#include "sokol_nuklear.h"
#else
// Test mode: include minimal nuklear definitions
#ifdef TEST_MODE
#include "../tests/stubs/ui_test_stubs.h"
#endif
#endif

#include "sokol_log.h"

// Screen capture functionality
/**
 * @brief Captures the current frame buffer to a BMP file
 * @param filename The output filename (should end in .bmp)
 * @return true on success, false on failure
 */
bool graphics_capture_screenshot(const char* filename);

// Test mode function declarations
#ifdef TEST_MODE
struct nk_context* get_nuklear_context(void);
void graphics_begin_frame(void);
void graphics_end_frame(void);
bool graphics_init(int width, int height);
void graphics_cleanup(void);
#endif

#endif  // GRAPHICS_API_H
