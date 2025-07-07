#ifndef GRAPHICS_HEALTH_H
#define GRAPHICS_HEALTH_H

#include <stdbool.h>
#include <stdint.h>

// Graphics health monitoring system
typedef struct {
    uint32_t context_invalidations;
    uint32_t pipeline_failures;
    uint32_t buffer_overflows;
    uint32_t draw_calls_dropped;
    uint32_t frames_rendered;
    uint32_t last_error_frame;
    char last_error_msg[256];
} GraphicsHealthStats;

// Initialize health monitoring
void gfx_health_init(void);

// Check graphics health after system operations
bool gfx_health_check(const char* system_name);

// Get current health statistics
const GraphicsHealthStats* gfx_health_get_stats(void);

// Reset health statistics
void gfx_health_reset_stats(void);

// Log draw call for tracking
void gfx_health_log_draw_call(const char* desc, int vertex_count);

// Attempt to recover from graphics errors
bool gfx_health_attempt_recovery(void);

// Dump diagnostic information
void gfx_health_dump_diagnostics(void);

#endif // GRAPHICS_HEALTH_H