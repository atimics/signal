// src/graphics_api.c

// This file is responsible for the implementation of the Sokol libraries.
// By defining SOKOL_IMPL here, we ensure that the implementation is
// compiled only once, preventing linker errors.

#define SOKOL_IMPL
#define SOKOL_NUKLEAR_IMPL
typedef int nk_bool;  // Define nk_bool for sokol_nuklear.h implementation
#include "graphics_api.h"
