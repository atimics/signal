#ifndef ASSET_LOADER_INDEX_H
#define ASSET_LOADER_INDEX_H

#include "../assets.h"

/**
 * @brief Load assets from the main index.json file
 * 
 * Loads all mesh assets listed in the meshes/index.json file.
 * This is the primary entry point for the asset loading system.
 * 
 * @param registry The asset registry to load assets into
 * @return true if all assets loaded successfully, false otherwise
 */
bool load_assets_from_metadata(AssetRegistry* registry);

/**
 * @brief Load a single mesh from its metadata.json file
 * 
 * Parses a metadata.json file for a specific mesh and loads the
 * associated geometry, material, and texture files.
 * 
 * @param registry The asset registry to load the mesh into
 * @param metadata_path Full path to the metadata.json file
 * @return true if mesh loaded successfully, false otherwise
 */
bool load_single_mesh_metadata(AssetRegistry* registry, const char* metadata_path);

#endif // ASSET_LOADER_INDEX_H
