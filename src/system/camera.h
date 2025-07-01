#ifndef SYSTEM_CAMERA_H
#define SYSTEM_CAMERA_H

#include "../core.h"
#include "../render.h"

// Camera system interface
void camera_system_init(struct World* world, RenderConfig* render_config);
void camera_system_update(struct World* world, RenderConfig* render_config, float delta_time);
void camera_system_cleanup(void);

// Camera control functions
bool camera_switch_to_index(struct World* world, int camera_index);
bool camera_activate_entity(struct World* world, EntityID camera_entity);

#endif // SYSTEM_CAMERA_H
