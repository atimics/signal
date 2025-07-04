/**
 * @file wasm_stubs.c
 * @brief Stub implementations for WASM build to replace platform-specific code
 */

#include "core.h"
#include "data.h"
#include "assets.h"
#include "scene_script.h"
#include <stdio.h>
#include <string.h>

// HID API stubs (no USB/Bluetooth controller support in WASM)
int hid_init(void) { return 0; }
void* hid_enumerate(unsigned short vendor_id, unsigned short product_id) { 
    (void)vendor_id; (void)product_id; 
    return NULL; 
}
void* hid_open_path(const char* path) { (void)path; return NULL; }
int hid_set_nonblocking(void* device, int nonblock) { 
    (void)device; (void)nonblock; 
    return 0; 
}
void hid_free_enumeration(void* devs) { (void)devs; }
void hid_close(void* device) { (void)device; }
void hid_exit(void) {}
int hid_read(void* device, unsigned char* data, size_t length) { 
    (void)device; (void)data; (void)length; 
    return 0; 
}

// YAML loader stubs (no YAML support in WASM for now)
void entity_yaml_loader_shutdown(void) {}
void scene_yaml_loader_shutdown(void) {}
bool load_entity_templates_yaml(const char* yaml_path, struct DataRegistry* registry) {
    (void)yaml_path; (void)registry;
    printf("⚠️  YAML loading not supported in WASM build\n");
    return false;
}
bool scene_load_from_yaml(struct World* world, struct AssetRegistry* assets, const char* scene_name) {
    (void)world; (void)assets; (void)scene_name;
    printf("⚠️  YAML scene loading not supported in WASM build\n");
    return false;
}

// Input processing stubs (simplified for WASM)
void production_input_processor_init(void* processor) {
    (void)processor;
}
void production_input_process(void* processor, void* input_data, void* output_data, float delta_time) {
    (void)processor;
    (void)delta_time;
    // Simple pass-through for WASM
    memcpy(output_data, input_data, sizeof(float) * 16);  // Assuming max 16 float inputs
}

// ODE test scene stub
const SceneScript ode_test_script = {
    .scene_name = "ode_test",
    .on_enter = NULL,
    .on_exit = NULL,
    .on_update = NULL,
    .on_input = NULL
};