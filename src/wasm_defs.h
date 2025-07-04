#ifndef WASM_DEFS_H
#define WASM_DEFS_H

// Common definitions for WASM builds
#ifdef WASM_BUILD

#include <stdlib.h>
#include <string.h>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Define strdup if not available
#ifndef strdup
static inline char* strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}
#endif

#endif // WASM_BUILD

#endif // WASM_DEFS_H