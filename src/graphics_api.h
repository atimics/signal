// src/graphics_api.h
#ifndef GRAPHICS_API_H
#define GRAPHICS_API_H

// src/graphics_api.h
#ifndef GRAPHICS_API_H
#define GRAPHICS_API_H

// This is the central hub for all Sokol graphics library includes.
// By including this single file, other modules can access the necessary
// graphics functionality without being tightly coupled to the Sokol implementation.

// Forward-declare the Sokol types that are used in other public headers.
// This avoids including the entire sokol_gfx.h header in other headers,
// which can lead to redefinition errors.
#include <stdint.h>
typedef struct sg_buffer { uint32_t id; } sg_buffer;
typedef struct sg_image { uint32_t id; } sg_image;
typedef struct sg_pipeline { uint32_t id; } sg_pipeline;
typedef struct sg_sampler { uint32_t id; } sg_sampler;
typedef struct sg_shader { uint32_t id; } sg_shader;


// Include the necessary Sokol headers in the correct order.
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"
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

#endif // GRAPHICS_API_H


#endif // GRAPHICS_API_H
