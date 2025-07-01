#ifndef SYSTEM_COLLISION_H
#define SYSTEM_COLLISION_H

#include "core.h"
#include "systems.h"

void collision_system_update(struct World* world, RenderConfig* render_config, float delta_time);

#endif // SYSTEM_COLLISION_H
