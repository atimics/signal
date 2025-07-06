#ifndef RENDER_PASS_GUARD_H
#define RENDER_PASS_GUARD_H

#include <stdbool.h>
#include <assert.h>
#include "sokol_gfx.h"

typedef struct {
    bool active;
    const char* name;
} pass_guard_t;

extern pass_guard_t g_pass_guard;

#define PASS_BEGIN(name, pass_desc) \
    do { \
        assert(!g_pass_guard.active && "Render pass already active"); \
        sg_begin_pass(pass_desc); \
        g_pass_guard = (pass_guard_t){true, name}; \
    } while(0)

#define PASS_END() \
    do { \
        if (g_pass_guard.active) { \
            sg_end_pass(); \
            g_pass_guard.active = false; \
            g_pass_guard.name = NULL; \
        } \
    } while(0)

#define ASSERT_NO_PASS_ACTIVE() \
    assert(!g_pass_guard.active && "No render pass should be active")

#define ASSERT_PASS_ACTIVE() \
    assert(g_pass_guard.active && "Render pass must be active")

#endif