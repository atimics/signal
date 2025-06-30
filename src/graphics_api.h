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
#endif

#include "sokol_log.h"

#endif  // GRAPHICS_API_H
