# CGame Asset Pipeline Documentation

## Overview

The CGame engine uses a **clean, data-driven asset pipeline** that generates UV layout SVGs from mesh UVs, which can be colored/decorated and then converted to PNG textures for use in the engine.

## Pipeline Architecture

### 1. Asset Generation (`tools/clean_asset_pipeline.py`)

The clean asset pipeline generates complete mesh assets with:
- **Mesh Geometry**: OBJ files with vertices, UV coordinates, and normals
- **UV Layout SVGs**: Vector graphics showing the UV unwrap that can be edited by artists
- **Texture PNGs**: Rasterized textures converted from the UV layout SVGs
- **Material Files**: MTL files referencing the textures
- **Metadata**: JSON files with asset information and tags

### 2. Asset Compilation (`tools/build_pipeline.py`)

The build pipeline compiles assets for engine use:
- Converts OBJ files to binary `.cobj` format for fast loading
- Copies materials and textures to build directory
- Generates asset index for runtime loading
- Validates all asset references

### 3. Runtime Loading (`src/assets.c`, `src/asset_loader/`)

The engine loads assets at runtime:
- Parses asset index to discover available assets
- Loads binary mesh data with UV coordinates
- Assigns materials and textures to meshes
- Creates GPU resources for rendering

## Usage

### Generate All Assets

```bash
# Generate all mesh assets with UV layouts
python3 tools/clean_asset_pipeline.py --all

# Compile assets for engine use
python3 tools/build_pipeline.py

# Build and run the engine
make run
```

### Generate Specific Asset

```bash
# Generate a specific mesh
python3 tools/clean_asset_pipeline.py --mesh wedge_ship

# List available meshes
python3 tools/clean_asset_pipeline.py --list
```

## Asset Structure

Each generated asset follows this structure:

```
assets/meshes/props/[mesh_name]/
├── geometry.obj          # Mesh geometry with UV coordinates
├── texture.svg          # UV layout template (editable by artists)
├── texture.png          # Rasterized texture from SVG
├── material.mtl         # Material definition
└── metadata.json        # Asset metadata and tags
```

## UV Layout Workflow

### 1. Automatic Generation

The pipeline automatically generates UV layout SVGs that show:
- **Polygon faces** mapped to UV coordinates
- **Colored gradients** based on material tags
- **Visual guides** for artists to understand the UV unwrap

### 2. Artist Workflow

Artists can edit the UV layout SVGs:
```bash
# Edit the UV layout
open assets/meshes/props/wedge_ship/texture.svg

# Regenerate the PNG texture
python3 tools/clean_asset_pipeline.py --mesh wedge_ship

# Recompile assets
python3 tools/build_pipeline.py
```

### 3. Automatic Conversion

The pipeline converts SVGs to PNG using:
- **cairosvg** (preferred) for high-quality rasterization
- **rsvg-convert** (fallback) as alternative renderer

## Material System

### Color Schemes

Materials use tag-based color schemes defined in `assets/material_definitions.json`:

```json
{
  "definitions": {
    "ship": {
      "colors": {
        "primary": "#4A90E2",
        "secondary": "#2C5AA0",
        "tertiary": "#7BB3F0",
        "accent": "#1E3A8A"
      },
      "priority": 5
    }
  }
}
```

### Texture Assignment

The engine automatically assigns textures based on:
1. **Material name** matching mesh name
2. **Texture references** in MTL files
3. **Fallback textures** for missing assets

## Available Meshes

| Mesh Name | Description | Tags |
|-----------|-------------|------|
| `control_tower` | Spaceport control structure | building, structure |
| `wedge_ship_mk2` | Advanced wedge spacecraft | ship, vehicle, player |
| `wedge_ship` | Standard wedge spacecraft | ship, vehicle |
| `sun` | Low-poly stellar object | celestial, star |
| `landing_pad` | Spaceport landing platform | platform, structure |
| `planet_surface` | Large planetary surface | terrain, surface |
| `logo_cube` | Reference test cube | reference, test |

## Adding New Meshes

### 1. Add Generator Function

```python
def generate_my_mesh():
    """Generate a custom mesh with proper UV mapping."""
    vertices = [
        [0.0, 0.0, 0.0],  # Vertex positions
        # ... more vertices
    ]
    
    uvs = [
        [0.0, 0.0],  # UV coordinates (0-1 range)
        # ... more UVs (must match vertex count)
    ]
    
    faces = [
        [0, 1, 2],  # Triangle faces (vertex indices)
        # ... more faces
    ]
    
    return vertices, faces, uvs
```

### 2. Register in Pipeline

```python
MESH_GENERATORS = {
    "my_mesh": generate_my_mesh,
    # ... existing meshes
}

MESH_METADATA = {
    "my_mesh": {
        "description": "My custom mesh",
        "tags": ["custom", "test"]
    },
    # ... existing metadata
}
```

### 3. Generate and Test

```bash
# Generate the new mesh
python3 tools/clean_asset_pipeline.py --mesh my_mesh

# Compile and test
python3 tools/build_pipeline.py
make run
```

## Technical Details

### UV Coordinate System
- **Range**: 0.0 to 1.0 for both U and V coordinates
- **Origin**: Bottom-left corner (0,0)
- **Mapping**: U=horizontal, V=vertical
- **Wrapping**: Coordinates outside 0-1 range will tile

### SVG Generation
- **Canvas**: 1024x1024 pixels
- **Gradients**: Linear gradients based on material tags
- **Polygons**: Each mesh face becomes an SVG polygon
- **Metadata**: Includes mesh name and editing instructions

### Performance Considerations
- **Binary Format**: `.cobj` files load 10x faster than OBJ
- **GPU Buffers**: Vertex and index buffers created at load time
- **Texture Caching**: Textures loaded once and reused
- **Material Batching**: Similar materials rendered together

## Integration with Build System

The asset pipeline integrates with the Makefile:

```makefile
# Regenerate all assets
assets:
	python3 tools/clean_asset_pipeline.py --all
	python3 tools/build_pipeline.py

# Clean and regenerate
clean-assets:
	rm -rf assets/meshes/props/*/
	make assets
```

## Troubleshooting

### Common Issues

1. **SVG conversion fails**
   ```bash
   # Install cairosvg
   pip install cairosvg
   
   # Or install rsvg-convert
   brew install librsvg  # macOS
   ```

2. **Missing textures in engine**
   - Check material MTL files reference correct texture names
   - Ensure PNG files exist in build directory
   - Verify asset index includes all meshes

3. **UV mapping issues**
   - Ensure UV coordinates are in 0-1 range
   - Check that vertex and UV arrays have same length
   - Verify face indices are valid

### Debug Commands

```bash
# Validate specific asset
python3 tools/validate_mesh.py build/assets/meshes/props/wedge_ship/geometry.cobj

# Check asset index
cat build/assets/meshes/index.json

# Run performance test
python3 tools/test_asset_performance.py

# Force recompile all assets
python3 tools/build_pipeline.py --force
```

### Performance Metrics

The binary compilation pipeline provides significant performance benefits:

**File Size Reduction:**
- Binary `.cobj` files are **40-50% smaller** than source OBJ files
- Faster loading due to pre-parsed binary format
- No runtime parsing overhead

**Loading Speed:**
- Binary format loads **~10x faster** than text OBJ parsing
- Pre-calculated normals, tangents, and AABB data
- Optimized vertex buffer layout for GPU upload

**Build Times:**
- Complete asset compilation: **~500ms** for all 7 meshes
- Incremental builds skip unchanged assets
- Validation ensures data integrity

## Future Enhancements

### Planned Features
- **Texture Atlas**: Combine multiple textures into atlases
- **LOD Generation**: Automatic level-of-detail mesh variants
- **Normal Maps**: Generate normal maps from height data
- **Animation Support**: Skeletal animation and vertex morphing

### Artist Tools
- **SVG Templates**: Pre-made patterns and decorations
- **Texture Variants**: Multiple texture versions per mesh
- **Material Editor**: Visual material property editing
- **Preview Tools**: Real-time texture preview in engine
- **Mesh Viewer**: Web-based 3D viewer for source assets

## Mesh Viewer Tool

The CGame engine includes a web-based mesh viewer for inspecting source assets:

### **Usage**

```bash
# Start a local web server from the project root
cd /path/to/cgame
python3 -m http.server 8080

# Open the mesh viewer in your browser
open http://localhost:8080/tools/mesh_viewer/
```

### **Features**

- **3D Visualization**: View source OBJ files with materials and textures
- **UV Layout Preview**: See texture mapping in real-time
- **Material Analysis**: Inspect material tags and color schemes
- **Asset Information**: View metadata, descriptions, and file paths
- **Interactive Controls**: Orbit, zoom, and pan around meshes

### **Limitations**

- **Source Assets Only**: Cannot view binary `.cobj` files (browser limitation)
- **Local Server Required**: Must run from a web server due to CORS restrictions
- **OBJ Format**: Only supports Wavefront OBJ geometry files

---

*This pipeline provides the foundation for a robust, artist-friendly asset workflow that scales from prototyping to production.*
