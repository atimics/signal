#ifndef ASSET_LOADER_MESH_H
#define ASSET_LOADER_MESH_H

#include "../assets.h"

/**
 * @brief Parse a .cobj (compiled mesh) file
 *
 * Parses a compiled mesh file in OBJ format and populates the provided mesh structure.
 * This function performs a three-pass parsing for memory efficiency:
 * 1. Count elements to determine memory requirements
 * 2. Allocate exact memory needed
 * 3. Parse and populate the mesh data
 *
 * @param filepath Path to the .cobj file to parse
 * @param mesh Mesh structure to populate (name should be pre-set)
 * @return true if parsing successful, false otherwise
 */
bool parse_obj_file(const char* filepath, Mesh* mesh);

/**
 * @brief Load a compiled mesh from an absolute file path
 *
 * Loads a compiled mesh file (.cobj) from the given absolute path and
 * registers it in the asset registry with the given name. Also creates
 * GPU resources for immediate rendering use.
 *
 * @param registry The asset registry to store the mesh in
 * @param absolute_filepath Full absolute path to the .cobj file
 * @param mesh_name Name to register the mesh under
 * @return true if mesh loaded successfully, false otherwise
 */
bool load_mesh_from_file(AssetRegistry* registry, const char* absolute_filepath,
                         const char* mesh_name);

#endif  // ASSET_LOADER_MESH_H
