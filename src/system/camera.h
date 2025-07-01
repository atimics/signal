#ifndef CAMERA_H
#define CAMERA_H

#include "core.h"
#include "systems.h"

void camera_system_update(struct World* world, RenderConfig* render_config, float delta_time);

#endif // CAMERA_H
