#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "core.h"
#include "data.h"
#include "render.h"

// Forward declarations for the scheduler
struct AssetRegistry;
struct DataRegistry;

// Include modular system headers
#include "system/physics.h"
#include "system/collision.h"  
#include "system/ai.h"
#include "system/camera.h"
#include "system/lod.h"
#include "system/performance.h"
#include "system/memory.h"
#include "system/material.h"
#include "system/thrusters.h"

// ============================================================================
// ENUMS AND STRUCTS
// ============================================================================

typedef enum
{
    SYSTEM_PHYSICS,
    SYSTEM_COLLISION,
    SYSTEM_AI,
    SYSTEM_CAMERA,
    SYSTEM_LOD,
    SYSTEM_PERFORMANCE,
    SYSTEM_MEMORY,
    SYSTEM_THRUSTERS,
    SYSTEM_COUNT
} SystemType;

typedef void (*SystemUpdateFunc)(struct World* world, RenderConfig* render_config,
                                 float delta_time);

typedef struct
{
    const char* name;
    float frequency;
    float last_update;
    bool enabled;
    SystemUpdateFunc update_func;
} SystemInfo;

typedef struct SystemScheduler
{
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
bool scheduler_init(SystemScheduler* scheduler, RenderConfig* config);
void scheduler_destroy(SystemScheduler* scheduler, RenderConfig* config);
void scheduler_update(SystemScheduler* scheduler, struct World* world, RenderConfig* render_config, float delta_time);
void scheduler_print_stats(SystemScheduler* scheduler);

// System Control
void scheduler_enable_system(SystemScheduler* scheduler, SystemType type);
void scheduler_disable_system(SystemScheduler* scheduler, SystemType type);
void scheduler_set_frequency(SystemScheduler* scheduler, SystemType type, float frequency);

// Data Access - These will be removed once the refactor is complete
DataRegistry* get_data_registry(void);
AssetRegistry* get_asset_registry(void);

#endif  // SYSTEMS_H