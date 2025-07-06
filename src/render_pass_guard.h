#pragma once
#include "sokol_gfx.h"
#include "sokol_app.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* global singleton (defined in render_pass_guard.c) */
typedef struct {
    bool        active;
    const char* tag;
} pass_guard_t;

extern pass_guard_t g_pass_guard;

/* PASS_BEGIN macro that works with current Sokol API */
#define PASS_BEGIN(tag, pass_ptr)                          \
    do {                                                   \
        assert(!g_pass_guard.active && "begin_pass while another pass open"); \
        sg_begin_pass((pass_ptr));                         \
        g_pass_guard = (pass_guard_t){ true, tag };        \
    } while (0)

/* PASS_END macro */
#define PASS_END() \
    do { \
        if (g_pass_guard.active) { \
            sg_end_pass(); \
            g_pass_guard.active = false; \
            g_pass_guard.tag = NULL; \
        } \
    } while(0)

/* Assert no pass active - works in both debug and release */
#if defined(NDEBUG)
    #define ASSERT_NO_PASS_ACTIVE() do { \
        if (g_pass_guard.active) { \
            printf("❌ CRITICAL: Encoder active in %s - aborting!\n", __func__); \
            abort(); \
        } \
    } while(0)
#else
    #define ASSERT_NO_PASS_ACTIVE() \
        assert(!g_pass_guard.active && "No render pass should be active")
#endif

/* Assert pass is active */
#define ASSERT_PASS_ACTIVE() \
    assert(g_pass_guard.active && "Render pass must be active")

/* Layer begin helper - simplified */
#define BEGIN_LAYER_PASS(layer) \
    PASS_BEGIN((layer)->name, &(sg_pass){ \
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
    })

/* Helper to check if encoder is active */
static inline bool layer_is_encoder_active(void) {
    return g_pass_guard.active;
}

#endif