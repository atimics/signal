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

// Unified layer pass management helper
#define BEGIN_LAYER_PASS(layer) \
    do { \
        sg_pass layer_pass = { \
            .attachments = (layer)->attachments, \
            .action = { \
                .colors[0] = { \
                    .load_action = SG_LOADACTION_CLEAR, \
                    .clear_value = (layer)->clear_color \
                }, \
                .depth = { \
                    .load_action = (layer)->depth_target.id != SG_INVALID_ID ? SG_LOADACTION_CLEAR : SG_LOADACTION_DONTCARE, \
                    .clear_value = (layer)->clear_depth \
                }, \
                .stencil = { \
                    .load_action = (layer)->depth_target.id != SG_INVALID_ID ? SG_LOADACTION_CLEAR : SG_LOADACTION_DONTCARE, \
                    .clear_value = (layer)->clear_stencil \
                } \
            } \
        }; \
        PASS_BEGIN((layer)->name, &layer_pass); \
    } while(0)

#endif