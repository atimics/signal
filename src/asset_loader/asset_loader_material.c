#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../assets.h"

bool parse_mtl_file(const char* filepath, AssetRegistry* registry) {
  if (!filepath || !registry) return false;

  FILE* file = fopen(filepath, "r");
  if (!file) {
    // Don't log missing MTL files as errors - they're optional
    return false;
  }

  Material* current_material = NULL;
  char line[256];

  while (fgets(line, sizeof(line), file)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

    // New material
    if (strncmp(line, "newmtl ", 7) == 0) {
      if (registry->material_count < 32) {
        current_material = &registry->materials[registry->material_count++];
        memset(current_material, 0, sizeof(Material));
        sscanf(line, "newmtl %63s", current_material->name);
        current_material->diffuse_color =
            (Vector3){0.8f, 0.8f, 0.8f};  // Default gray
        current_material->loaded = true;
      }
    }
    // Diffuse color
    else if (strncmp(line, "Kd ", 3) == 0 && current_material) {
      sscanf(line, "Kd %f %f %f", &current_material->diffuse_color.x,
             &current_material->diffuse_color.y,
             &current_material->diffuse_color.z);
    }
    // Ambient color
    else if (strncmp(line, "Ka ", 3) == 0 && current_material) {
      sscanf(line, "Ka %f %f %f", &current_material->ambient_color.x,
             &current_material->ambient_color.y,
             &current_material->ambient_color.z);
    }
    // Specular color
    else if (strncmp(line, "Ks ", 3) == 0 && current_material) {
      sscanf(line, "Ks %f %f %f", &current_material->specular_color.x,
             &current_material->specular_color.y,
             &current_material->specular_color.z);
    }
    // Emission color
    else if (strncmp(line, "Ke ", 3) == 0 && current_material) {
      sscanf(line, "Ke %f %f %f", &current_material->emission_color.x,
             &current_material->emission_color.y,
             &current_material->emission_color.z);
    }
    // Shininess
    else if (strncmp(line, "Ns ", 3) == 0 && current_material) {
      sscanf(line, "Ns %f", &current_material->shininess);
    }
    // Diffuse texture map (primary texture)
    else if (strncmp(line, "map_Kd ", 7) == 0 && current_material) {
      sscanf(line, "map_Kd %63s", current_material->diffuse_texture);
      // Also set legacy texture_name for compatibility
      strncpy(current_material->texture_name, current_material->diffuse_texture,
              sizeof(current_material->texture_name) - 1);
    }
    // Normal/bump map
    else if (strncmp(line, "map_Bump ", 9) == 0 && current_material) {
      sscanf(line, "map_Bump %63s", current_material->normal_texture);
    } else if (strncmp(line, "bump ", 5) == 0 && current_material) {
      sscanf(line, "bump %63s", current_material->normal_texture);
    }
    // Specular map
    else if (strncmp(line, "map_Ks ", 7) == 0 && current_material) {
      sscanf(line, "map_Ks %63s", current_material->specular_texture);
    }
    // Emission map
    else if (strncmp(line, "map_Ke ", 7) == 0 && current_material) {
      sscanf(line, "map_Ke %63s", current_material->emission_texture);
    }
  }

  fclose(file);
  return true;
}
