#ifndef AI_H
#define AI_H

#include "core.h"
#include "systems.h"

void ai_system_update(struct World* world, RenderConfig* render_config, float delta_time);

#endif // AI_H
