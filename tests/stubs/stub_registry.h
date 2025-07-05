#ifndef STUB_REGISTRY_H
#define STUB_REGISTRY_H

/**
 * @file stub_registry.h
 * @brief Centralized registry for all test stub functions
 * 
 * This header provides a central location for declaring all stub functions
 * used in tests, preventing symbol conflicts and duplication.
 */

#ifdef TEST_MODE

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Forward declarations for types
typedef struct { uint32_t id; } sg_shader;
typedef struct { uint32_t id; } sg_pipeline;
typedef struct { uint32_t id; } sg_sampler;
typedef struct { uint32_t id; } sg_bindings;
typedef struct { uint32_t id; } sg_buffer;
typedef struct { uint32_t id; } sg_image;
typedef struct { uint32_t id; } sg_attachments;
typedef struct { uint32_t id; } sg_pass;

// Resource state enum
typedef enum {
    SG_RESOURCESTATE_INITIAL,
    SG_RESOURCESTATE_ALLOC,
    SG_RESOURCESTATE_VALID,
    SG_RESOURCESTATE_FAILED,
    SG_RESOURCESTATE_INVALID
} sg_resource_state;

// Range type for buffer updates
typedef struct {
    const void* ptr;
    size_t size;
} sg_range;

// ============================================================================
// RENDERING STUBS
// ============================================================================

// Offscreen rendering (NOTE: Implementation is in render_3d.c, not stubbed)
// bool render_is_offscreen_mode(void); // Do not stub - use real implementation

// Graphics API stubs
bool graphics_init(int width, int height);
void graphics_cleanup(void);
void graphics_begin_frame(void);
void graphics_end_frame(void);
void graphics_capture_screenshot(void);

// ============================================================================
// SOKOL GRAPHICS STUBS
// ============================================================================

// Setup and shutdown
void sg_setup(void* desc);
void sg_shutdown(void);

// Resource creation
sg_shader sg_make_shader(void* desc);
sg_pipeline sg_make_pipeline(void* desc);
sg_sampler sg_make_sampler(void* desc);
sg_buffer sg_make_buffer(void* desc);
sg_image sg_make_image(void* desc);
sg_attachments sg_make_attachments(void* desc);

// Resource management
void sg_destroy_shader(sg_shader shader);
void sg_destroy_pipeline(sg_pipeline pipeline);
void sg_destroy_sampler(sg_sampler sampler);
void sg_destroy_buffer(sg_buffer buffer);
void sg_destroy_image(sg_image image);
void sg_destroy_attachments(sg_attachments attachments);

// Resource queries
sg_resource_state sg_query_shader_state(sg_shader shader);
sg_resource_state sg_query_pipeline_state(sg_pipeline pipeline);
sg_resource_state sg_query_buffer_state(sg_buffer buffer);
sg_resource_state sg_query_image_state(sg_image image);

// Rendering operations
void sg_begin_pass(void* pass);
void sg_end_pass(void);
void sg_apply_pipeline(sg_pipeline pipeline);
void sg_apply_bindings(const sg_bindings* bindings);
void sg_apply_uniforms(int stage, int uniform_slot, const sg_range* data);
void sg_draw(int base_element, int num_elements, int num_instances);
void sg_update_buffer(sg_buffer buf, const sg_range* data);
void sg_update_image(sg_image img, const void* data_ptr, size_t data_size);

// ============================================================================
// UI SYSTEM STUBS
// ============================================================================

struct nk_context;
struct nk_context* get_nuklear_context(void);

// ============================================================================
// ASSET SYSTEM STUBS
// ============================================================================

bool load_assets_from_metadata(void* registry);

// Material system
typedef struct {
    uint32_t id;
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float shininess;
} MaterialProperties;

MaterialProperties* material_get_by_id(uint32_t material_id);

// ============================================================================
// SHADER SYSTEM STUBS
// ============================================================================

char* load_shader_source(const char* filename);
void free_shader_source(char* source);
const char* get_shader_path(const char* filename);

#endif // TEST_MODE

#endif // STUB_REGISTRY_H
