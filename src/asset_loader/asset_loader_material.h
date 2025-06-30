#ifndef ASSET_LOADER_MATERIAL_H
#define ASSET_LOADER_MATERIAL_H

#include "../assets.h"

/**
 * @brief Parse a .mtl (material) file
 * 
 * Parses a material file in MTL format and adds the materials to the asset registry.
 * MTL files define material properties including colors, textures, and lighting
 * parameters for 3D meshes.
 * 
 * @param filepath Path to the .mtl file to parse
 * @param registry Asset registry to store parsed materials in
 * @return true if parsing successful, false otherwise (note: missing files are not errors)
 */
bool parse_mtl_file(const char* filepath, AssetRegistry* registry);

#endif // ASSET_LOADER_MATERIAL_H
