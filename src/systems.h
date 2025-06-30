#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "core.h"
#include "data.h"
#include "render.h"

// ============================================================================
// ENUMS AND STRUCTS
// ============================================================================

typedef enum {
    SYSTEM_PHYSICS,
    SYSTEM_COLLISION,
    SYSTEM_AI,
    SYSTEM_CAMERA,
    SYSTEM_COUNT
} SystemType;

typedef void (*SystemUpdateFunc)(struct World* world, RenderConfig* render_config, float delta_time);

typedef struct {
    const char* name;
    float frequency;
    float last_update;
    bool enabled;
    SystemUpdateFunc update_func;
} SystemInfo;

typedef struct SystemScheduler {
    SystemInfo systems[SYSTEM_COUNT];
    float total_time;
    int frame_count;
    float system_times[SYSTEM_COUNT];
    int system_calls[SYSTEM_COUNT];
} SystemScheduler;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// System Scheduler
bool scheduler_init(SystemScheduler* scheduler);
void scheduler_destroy(SystemScheduler* scheduler);
void scheduler_update(SystemScheduler* scheduler, struct World* world, RenderConfig* render_config, float delta_time);
void scheduler_print_stats(SystemScheduler* scheduler);

// System Control
void scheduler_enable_system(SystemScheduler* scheduler, SystemType type);
void scheduler_disable_system(SystemScheduler* scheduler, SystemType type);
void scheduler_set_frequency(SystemScheduler* scheduler, SystemType type, float frequency);

// Individual Systems
void physics_system_update(struct World* world, RenderConfig* render_config, float delta_time);
void collision_system_update(struct World* world, RenderConfig* render_config, float delta_time);
void ai_system_update(struct World* world, RenderConfig* render_config, float delta_time);
void camera_system_update(struct World* world, RenderConfig* render_config, float delta_time);

// Data Access
DataRegistry* get_data_registry(void);
AssetRegistry* get_asset_registry(void);  // Access to global asset registry

#endif // SYSTEMS_H