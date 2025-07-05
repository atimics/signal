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
#if !defined(SOKOL_DUMMY_BACKEND)
#include "sokol_app.h"
#include "sokol_glue.h"
#endif

// Test mode: include minimal UI definitions
#ifdef TEST_MODE
#include "../tests/stubs/ui_test_stubs.h"
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
// Legacy Nuklear context removed - using MicroUI now
void graphics_begin_frame(void);
void graphics_end_frame(void);
bool graphics_init(int width, int height);
void graphics_cleanup(void);
#endif

#endif  // GRAPHICS_API_H
