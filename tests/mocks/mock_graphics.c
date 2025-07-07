// tests/mocks/mock_graphics.c
// Mock implementations for graphics functions during testing
// This allows memory performance tests to run without graphics dependencies

#include "mock_graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Mock asset registry
static MockAssetRegistry g_mock_asset_registry = {0};
MockAssetRegistry* g_asset_registry = &g_mock_asset_registry;

// Mock performance tracking
void performance_record_memory_usage(size_t bytes_allocated, size_t bytes_freed) {
    // Mock implementation - just track basic stats
    (void)bytes_allocated;
    (void)bytes_freed;
}

// Mock GPU resource functions - use proper signatures
struct GpuResources* gpu_resources_create(void) {
    return (struct GpuResources*)malloc(sizeof(int)); // Return dummy pointer
}

void gpu_resources_set_vertex_buffer(struct GpuResources* resources, gpu_buffer_t vbuf) {
    (void)resources;
    (void)vbuf;
}

void gpu_resources_set_index_buffer(struct GpuResources* resources, gpu_buffer_t ibuf) {
    (void)resources;
    (void)ibuf;
}

void gpu_resources_set_texture(struct GpuResources* resources, gpu_image_t tex) {
    (void)resources;
    (void)tex;
}

// Mock asset loading function
int load_assets_from_metadata(const char* metadata_path) {
    (void)metadata_path;
    return 1; // Success
}

// Mock Sokol graphics functions - only if not already defined
#ifndef SOKOL_GFX_INCLUDED

static uint32_t next_buffer_id = 1;
static uint32_t next_image_id = 1;

sg_buffer sg_make_buffer(const sg_buffer_desc* desc) {
    (void)desc;
    sg_buffer buffer = {next_buffer_id++};
    return buffer;
}

sg_image sg_make_image(const sg_image_desc* desc) {
    (void)desc;
    sg_image image = {next_image_id++};
    return image;
}

void sg_destroy_buffer(sg_buffer buffer) {
    (void)buffer;
}

void sg_destroy_image(sg_image image) {
    (void)image;
}

sg_resource_state sg_query_buffer_state(sg_buffer buffer) {
    (void)buffer;
    return SG_RESOURCESTATE_VALID;
}

#endif // SOKOL_GFX_INCLUDED
