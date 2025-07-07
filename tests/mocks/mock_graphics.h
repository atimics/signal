// tests/mocks/mock_graphics.h
// Mock implementations for graphics functions during testing
// This allows memory performance tests to run without graphics dependencies

#ifndef MOCK_GRAPHICS_H
#define MOCK_GRAPHICS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations to avoid conflicts
struct GpuResources;
typedef struct { uint32_t id; } gpu_buffer_t;
typedef struct { uint32_t id; } gpu_image_t;

// Mock asset registry structure
typedef struct {
    int initialized;
    size_t asset_count;
    size_t memory_used;
} MockAssetRegistry;

// Global mock asset registry
extern MockAssetRegistry* g_asset_registry;

// Mock performance tracking functions
void performance_record_memory_usage(size_t bytes_allocated, size_t bytes_freed);

// Mock GPU resource functions - use proper signatures to match real headers
struct GpuResources* gpu_resources_create(void);
void gpu_resources_set_vertex_buffer(struct GpuResources* resources, gpu_buffer_t vbuf);
void gpu_resources_set_index_buffer(struct GpuResources* resources, gpu_buffer_t ibuf);
void gpu_resources_set_texture(struct GpuResources* resources, gpu_image_t tex);

// Mock asset loading functions
int load_assets_from_metadata(const char* metadata_path);

// Mock Sokol graphics types and functions - only define if not already defined
#ifndef SOKOL_GFX_INCLUDED
typedef struct { uint32_t id; } sg_buffer;
typedef struct { uint32_t id; } sg_image;
typedef enum { SG_RESOURCESTATE_VALID = 1, SG_RESOURCESTATE_INVALID = 0 } sg_resource_state;
typedef struct { void* ptr; size_t size; } sg_range;
typedef struct { sg_range data; int usage; int type; } sg_buffer_desc;
typedef struct { int width; int height; int pixel_format; sg_range data; } sg_image_desc;

// Mock Sokol graphics functions
sg_buffer sg_make_buffer(const sg_buffer_desc* desc);
sg_image sg_make_image(const sg_image_desc* desc);
void sg_destroy_buffer(sg_buffer buffer);
void sg_destroy_image(sg_image image);
sg_resource_state sg_query_buffer_state(sg_buffer buffer);
#endif

#ifdef __cplusplus
}
#endif

#endif // MOCK_GRAPHICS_H
