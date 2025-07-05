#ifndef ASSET_STUBS_H
#define ASSET_STUBS_H

#ifdef TEST_MODE

// Asset system stubs for tests that don't link with assets.c
char* load_shader_source(const char* filename);
void free_shader_source(char* source);
const char* get_shader_path(const char* filename);

#endif // TEST_MODE

#endif // ASSET_STUBS_H
