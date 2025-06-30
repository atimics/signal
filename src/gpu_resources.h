// R06 Interface: PIMPL pattern for graphics resources
// Industry Standard: Public interface for GPU resource management

#ifndef GPU_RESOURCES_H
#define GPU_RESOURCES_H

#include "core.h"

// Forward declaration (R06 pattern) - fully opaque
struct GpuResources;

// GPU buffer and image types - completely opaque to users
typedef struct { uint32_t id; } gpu_buffer_t;
typedef struct { uint32_t id; } gpu_image_t;

// Industry Standard: Constructor/Destructor
struct GpuResources* gpu_resources_create(void);
void gpu_resources_destroy(struct GpuResources* resources);

// Industry Standard: Accessor functions (avoid exposing internal types)
gpu_buffer_t gpu_resources_get_vertex_buffer(struct GpuResources* resources);
gpu_buffer_t gpu_resources_get_index_buffer(struct GpuResources* resources);
gpu_image_t gpu_resources_get_texture(struct GpuResources* resources);

// Industry Standard: Mutator functions - these accept opaque handles
void gpu_resources_set_vertex_buffer(struct GpuResources* resources, gpu_buffer_t vbuf);
void gpu_resources_set_index_buffer(struct GpuResources* resources, gpu_buffer_t ibuf);
void gpu_resources_set_texture(struct GpuResources* resources, gpu_image_t tex);

// Additional helper functions for validation
bool gpu_resources_is_vertex_buffer_valid(struct GpuResources* resources);
bool gpu_resources_is_index_buffer_valid(struct GpuResources* resources);
bool gpu_resources_is_texture_valid(struct GpuResources* resources);

#endif // GPU_RESOURCES_H
