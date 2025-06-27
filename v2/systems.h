#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "core.h"

// ============================================================================
// SYSTEM SCHEDULER
// ============================================================================

typedef enum {
    SYSTEM_PHYSICS,
    SYSTEM_COLLISION,
    SYSTEM_AI,
    SYSTEM_RENDER,
    SYSTEM_COUNT
} SystemType;

typedef struct {
    const char* name;
    float frequency;        // Hz - how often to run
    float last_update;      // When last ran
    bool enabled;
    void (*update_func)(struct World* world, float delta_time);
} SystemInfo;

struct SystemScheduler {
    SystemInfo systems[SYSTEM_COUNT];
    float total_time;
    uint32_t frame_count;
    
    // Performance tracking
    float system_times[SYSTEM_COUNT];
    uint32_t system_calls[SYSTEM_COUNT];
};

// ============================================================================
// SYSTEM FUNCTIONS
// ============================================================================

// Physics System - Runs every frame (60 FPS)
void physics_system_update(struct World* world, float delta_time);

// Collision System - Runs every 3 frames (20 FPS)
void collision_system_update(struct World* world, float delta_time);

// AI System - Runs with LOD (2-10 FPS based on distance)
void ai_system_update(struct World* world, float delta_time);

// Render System - Runs every frame (60 FPS)
void render_system_update(struct World* world, float delta_time);

// ============================================================================
// SCHEDULER API
// ============================================================================

bool scheduler_init(struct SystemScheduler* scheduler);
void scheduler_destroy(struct SystemScheduler* scheduler);
void scheduler_update(struct SystemScheduler* scheduler, struct World* world, float delta_time);
void scheduler_print_stats(struct SystemScheduler* scheduler);

// System control
void scheduler_enable_system(struct SystemScheduler* scheduler, SystemType type);
void scheduler_disable_system(struct SystemScheduler* scheduler, SystemType type);
void scheduler_set_frequency(struct SystemScheduler* scheduler, SystemType type, float frequency);

#endif // SYSTEMS_H
