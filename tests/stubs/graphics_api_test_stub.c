/**
 * @file graphics_api_test_stub.c
 * @brief Test stub for graphics API that avoids sokol implementation
 */

#ifdef TEST_MODE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Include the test nuklear definitions
#include "ui_test_stubs.h"

// Forward declarations for types used in stubs (some already in engine_test_stubs.c)
typedef struct { uint32_t id; } sg_shader;
typedef struct { uint32_t id; } sg_pipeline;
typedef struct { uint32_t id; } sg_sampler;
typedef struct { uint32_t id; } sg_bindings;
typedef struct { uint32_t id; } sg_buffer;
typedef struct { uint32_t id; } sg_image;

// Resource state enum (from engine_test_stubs.c)
typedef enum {
    SG_RESOURCESTATE_INITIAL,
    SG_RESOURCESTATE_ALLOC,
    SG_RESOURCESTATE_VALID,
    SG_RESOURCESTATE_FAILED,
    SG_RESOURCESTATE_INVALID
} sg_resource_state;

// Sokol constants
#define SG_INVALID_ID 0

// Dummy material type for stubs
typedef struct {
    uint32_t id;
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float shininess;
} MaterialProperties;

// Static dummy resources for testing
static MaterialProperties dummy_material = {
    .id = 1,
    .ambient = {0.2f, 0.2f, 0.2f},
    .diffuse = {0.8f, 0.8f, 0.8f},
    .specular = {1.0f, 1.0f, 1.0f},
    .shininess = 32.0f
};

// Stub implementation for get_nuklear_context
struct nk_context* get_nuklear_context(void) {
    static struct nk_context dummy_ctx;
    return &dummy_ctx;
}

// Stub implementations for any other graphics functions that might be called
void graphics_begin_frame(void) {
    // No-op in test mode
}

void graphics_end_frame(void) {
    // No-op in test mode
}

bool graphics_init(int width, int height) {
    (void)width; (void)height;
    return true; // Always succeed in test mode
}

void graphics_cleanup(void) {
    // No-op in test mode
}

void graphics_capture_screenshot(void) {
    // No-op in test mode
}

// ==== SOKOL GRAPHICS STUBS (only non-duplicates) ====

// Resource creation functions (not already in engine_test_stubs.c)
sg_shader sg_make_shader(void* desc) {
    (void)desc;
    return (sg_shader){.id = 42}; // Return dummy valid ID
}

sg_pipeline sg_make_pipeline(void* desc) {
    (void)desc;
    return (sg_pipeline){.id = 42}; // Return dummy valid ID
}

sg_sampler sg_make_sampler(void* desc) {
    (void)desc;
    return (sg_sampler){.id = 42}; // Return dummy valid ID
}

// Resource destruction functions (not already in engine_test_stubs.c)
void sg_destroy_shader(sg_shader shd) {
    (void)shd;
    // No-op in test mode
}

void sg_destroy_pipeline(sg_pipeline pip) {
    (void)pip;
    // No-op in test mode
}

void sg_destroy_sampler(sg_sampler smp) {
    (void)smp;
    // No-op in test mode
}

// Resource allocation functions (for Sprint 23 100% test coverage)
sg_buffer sg_alloc_buffer(void) {
    return (sg_buffer){.id = 100};
}

sg_image sg_alloc_image(void) {
    return (sg_image){.id = 200};
}

sg_shader sg_alloc_shader(void) {
    return (sg_shader){.id = 300};
}

sg_resource_state sg_query_image_state(sg_image img) {
    (void)img;
    return SG_RESOURCESTATE_VALID;
}

// Rendering functions
void sg_apply_pipeline(sg_pipeline pip) {
    (void)pip;
    // No-op in test mode
}

void sg_apply_bindings(sg_bindings* bindings) {
    (void)bindings;
    // No-op in test mode
}

void sg_apply_uniforms(int stage, int ub_index, void* data, int size) {
    (void)stage; (void)ub_index; (void)data; (void)size;
    // No-op in test mode
}

void sg_draw(int base_element, int num_elements, int num_instances) {
    (void)base_element; (void)num_elements; (void)num_instances;
    // No-op in test mode
}

// Resource validation functions
bool sg_isvalid(void) {
    return true; // Always valid in test mode
}

sg_resource_state sg_query_shader_state(sg_shader shd) {
    (void)shd;
    return SG_RESOURCESTATE_VALID;
}

sg_resource_state sg_query_pipeline_state(sg_pipeline pip) {
    (void)pip;
    return SG_RESOURCESTATE_VALID;
}

sg_resource_state sg_query_sampler_state(sg_sampler smp) {
    (void)smp;
    return SG_RESOURCESTATE_VALID;
}

// ==== ASSET SYSTEM STUBS ====

bool load_assets_from_metadata(void* registry) {
    (void)registry;
    printf("[TEST STUB] load_assets_from_metadata called\n");
    return true; // Always succeed in test mode
}

// ==== MATERIAL SYSTEM STUBS ====

MaterialProperties* material_get_by_id(uint32_t material_id) {
    (void)material_id;
    printf("[TEST STUB] material_get_by_id called with id: %u\n", material_id);
    return &dummy_material; // Return dummy material
}

#endif // TEST_MODE
