#ifndef ENTITY_YAML_LOADER_H
#define ENTITY_YAML_LOADER_H

#include "data.h"
#include <stdbool.h>

// Load entity templates from a YAML file
bool load_entity_templates_yaml(DataRegistry* registry, const char* filename);

// Initialize YAML entity template loader
bool entity_yaml_loader_init(void);

// Shutdown YAML entity template loader
void entity_yaml_loader_shutdown(void);

#endif // ENTITY_YAML_LOADER_H
