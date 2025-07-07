// R06 Implementation: PIMPL pattern for graphics resources
// Industry Standard: Hide Sokol GFX types from core headers

#include "gpu_resources.h"

#include <stdlib.h>

#include "core.h"
#include "graphics_api.h"

// Internal conversion functions
static sg_buffer gpu_buffer_to_sg_buffer(gpu_buffer_t buf)
{
    return (sg_buffer){ .id = buf.id };
}

static gpu_buffer_t sg_buffer_to_gpu_buffer(sg_buffer buf)
{
    return (gpu_buffer_t){ .id = buf.id };
}

static sg_image gpu_image_to_sg_image(gpu_image_t img)
{
    return (sg_image){ .id = img.id };
}

static gpu_image_t sg_image_to_gpu_image(sg_image img)
{
    return (gpu_image_t){ .id = img.id };
}

// R06 PIMPL: The actual implementation struct (hidden from headers)
struct GpuResources
{
    sg_buffer vbuf;  // Vertex buffer
    sg_buffer ibuf;  // Index buffer
    sg_image tex;    // Texture
};

// Industry Standard: Constructor function
struct GpuResources* gpu_resources_create(void)
{
    struct GpuResources* resources = malloc(sizeof(struct GpuResources));
    if (resources)
    {
        // Initialize with invalid handles
        resources->vbuf.id = SG_INVALID_ID;
        resources->ibuf.id = SG_INVALID_ID;
        resources->tex.id = SG_INVALID_ID;
    }
    return resources;
}

// Industry Standard: Destructor function
void gpu_resources_destroy(struct GpuResources* resources)
{
    if (resources)
    {
        // Clean up GPU resources if needed
        // (Sokol handles cleanup automatically on shutdown)
        free(resources);
    }
}

// Industry Standard: Accessor functions
gpu_buffer_t gpu_resources_get_vertex_buffer(struct GpuResources* resources)
{
    return resources ? sg_buffer_to_gpu_buffer(resources->vbuf) : (gpu_buffer_t){ SG_INVALID_ID };
}

gpu_buffer_t gpu_resources_get_index_buffer(struct GpuResources* resources)
{
    return resources ? sg_buffer_to_gpu_buffer(resources->ibuf) : (gpu_buffer_t){ SG_INVALID_ID };
}

gpu_image_t gpu_resources_get_texture(struct GpuResources* resources)
{
    return resources ? sg_image_to_gpu_image(resources->tex) : (gpu_image_t){ SG_INVALID_ID };
}

// Industry Standard: Mutator functions
void gpu_resources_set_vertex_buffer(struct GpuResources* resources, gpu_buffer_t vbuf)
{
    if (resources)
    {
        resources->vbuf = gpu_buffer_to_sg_buffer(vbuf);
    }
}

void gpu_resources_set_index_buffer(struct GpuResources* resources, gpu_buffer_t ibuf)
{
    if (resources)
    {
        resources->ibuf = gpu_buffer_to_sg_buffer(ibuf);
    }
}

void gpu_resources_set_texture(struct GpuResources* resources, gpu_image_t tex)
{
    if (resources)
    {
        resources->tex = gpu_image_to_sg_image(tex);
    }
}

// Additional helper functions for validation
bool gpu_resources_is_vertex_buffer_valid(struct GpuResources* resources)
{
    return resources && resources->vbuf.id != SG_INVALID_ID;
}

bool gpu_resources_is_index_buffer_valid(struct GpuResources* resources)
{
    return resources && resources->ibuf.id != SG_INVALID_ID;
}

bool gpu_resources_is_texture_valid(struct GpuResources* resources)
{
    return resources && resources->tex.id != SG_INVALID_ID;
}
