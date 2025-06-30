// R06 Implementation: PIMPL pattern for graphics resources
// Industry Standard: Hide Sokol GFX types from core headers

#include "core.h"
#include "graphics_api.h"
#include <stdlib.h>

// R06 PIMPL: The actual implementation struct (hidden from headers)
struct GpuResources {
    sg_buffer vbuf;    // Vertex buffer
    sg_buffer ibuf;    // Index buffer  
    sg_image tex;      // Texture
};

// Industry Standard: Constructor function
struct GpuResources* gpu_resources_create(void) {
    struct GpuResources* resources = malloc(sizeof(struct GpuResources));
    if (resources) {
        // Initialize with invalid handles
        resources->vbuf.id = SG_INVALID_ID;
        resources->ibuf.id = SG_INVALID_ID;
        resources->tex.id = SG_INVALID_ID;
    }
    return resources;
}

// Industry Standard: Destructor function
void gpu_resources_destroy(struct GpuResources* resources) {
    if (resources) {
        // Clean up GPU resources if needed
        // (Sokol handles cleanup automatically on shutdown)
        free(resources);
    }
}

// Industry Standard: Accessor functions
sg_buffer gpu_resources_get_vertex_buffer(struct GpuResources* resources) {
    return resources ? resources->vbuf : (sg_buffer){SG_INVALID_ID};
}

sg_buffer gpu_resources_get_index_buffer(struct GpuResources* resources) {
    return resources ? resources->ibuf : (sg_buffer){SG_INVALID_ID};
}

sg_image gpu_resources_get_texture(struct GpuResources* resources) {
    return resources ? resources->tex : (sg_image){SG_INVALID_ID};
}

// Industry Standard: Mutator functions
void gpu_resources_set_vertex_buffer(struct GpuResources* resources, sg_buffer vbuf) {
    if (resources) {
        resources->vbuf = vbuf;
    }
}

void gpu_resources_set_index_buffer(struct GpuResources* resources, sg_buffer ibuf) {
    if (resources) {
        resources->ibuf = ibuf;
    }
}

void gpu_resources_set_texture(struct GpuResources* resources, sg_image tex) {
    if (resources) {
        resources->tex = tex;
    }
}
