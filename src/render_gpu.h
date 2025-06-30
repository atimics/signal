// R06 Interface: PIMPL pattern for graphics resources
// Industry Standard: Public interface for GPU resource management

#ifndef RENDER_GPU_H
#define RENDER_GPU_H

#include "core.h"

// Forward declarations (R06 pattern)
struct sg_buffer;
struct sg_image;

// Industry Standard: Constructor/Destructor
struct GpuResources* gpu_resources_create(void);
void gpu_resources_destroy(struct GpuResources* resources);

// Industry Standard: Accessor functions (avoid exposing internal types)
struct sg_buffer gpu_resources_get_vertex_buffer(struct GpuResources* resources);
struct sg_buffer gpu_resources_get_index_buffer(struct GpuResources* resources);
struct sg_image gpu_resources_get_texture(struct GpuResources* resources);

// Industry Standard: Mutator functions
void gpu_resources_set_vertex_buffer(struct GpuResources* resources, struct sg_buffer vbuf);
void gpu_resources_set_index_buffer(struct GpuResources* resources, struct sg_buffer ibuf);
void gpu_resources_set_texture(struct GpuResources* resources, struct sg_image tex);

// Additional helper functions for validation
bool gpu_resources_is_vertex_buffer_valid(struct GpuResources* resources);
bool gpu_resources_is_index_buffer_valid(struct GpuResources* resources);
bool gpu_resources_is_texture_valid(struct GpuResources* resources);

#endif // RENDER_GPU_H
