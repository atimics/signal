#include "asset_stubs.h"

#ifdef TEST_MODE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

char* load_shader_source(const char* filename) {
    (void)filename;
    char* dummy_source = malloc(32);
    if (dummy_source) {
        strcpy(dummy_source, "// Dummy shader source");
    }
    return dummy_source;
}

void free_shader_source(char* source) {
    if (source) {
        free(source);
    }
}

const char* get_shader_path(const char* filename) {
    (void)filename;
    return "dummy/path/shader.glsl";
}

// Additional asset system stubs
bool assets_init(void) {
    printf("[TEST STUB] assets_init called\n");
    return true;
}

void assets_cleanup(void) {
    printf("[TEST STUB] assets_cleanup called\n");
}

typedef struct { uint32_t id; } sg_image;

sg_image load_texture(const char* filename) {
    (void)filename;
    printf("[TEST STUB] load_texture called with: %s\n", filename);
    return (sg_image){.id = 42}; // Return dummy texture
}

#endif // TEST_MODE
