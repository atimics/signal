#ifndef SCENE_YAML_LOADER_H
#define SCENE_YAML_LOADER_H

#include "core.h"
#include <stdbool.h>

// Load a scene from a YAML file
bool scene_load_from_yaml(struct World* world, const char* filename);

// Initialize YAML scene loader
bool scene_yaml_loader_init(void);

// Shutdown YAML scene loader
void scene_yaml_loader_shutdown(void);

#endif // SCENE_YAML_LOADER_H