#ifndef SOKOL_WRAPPER_H
#define SOKOL_WRAPPER_H

// This is the single point of inclusion for the Sokol GFX library.
// All other headers in the project should include this file instead of
// including sokol_gfx.h directly.

#ifdef SOKOL_IMPL
#define SOKOL_LOG_IMPL
#endif

#include "sokol_gfx.h"
#include "sokol_log.h"

#endif // SOKOL_WRAPPER_H
