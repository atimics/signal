#ifndef SYSTEM_PHYSICS_H
#define SYSTEM_PHYSICS_H

#include "core.h"
#include "systems.h"

void physics_system_update(struct World* world, RenderConfig* render_config, float delta_time);

#endif // SYSTEM_PHYSICS_H
