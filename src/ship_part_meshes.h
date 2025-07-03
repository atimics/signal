#ifndef SHIP_PART_MESHES_H
#define SHIP_PART_MESHES_H

#include "assets.h"

// Initialize and register procedural ship part meshes
bool ship_part_meshes_init(AssetRegistry* registry);

// Individual mesh generators (exposed for testing)
Mesh* generate_pod_hull_mesh(void);
Mesh* generate_engine_bell_mesh(void);
Mesh* generate_rcs_block_mesh(void);
Mesh* generate_fuel_tank_mesh(float radius, float length);
Mesh* generate_structural_beam_mesh(float length, float width);

#endif // SHIP_PART_MESHES_H