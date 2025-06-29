# CGame Development Tools

This directory contains various tools for developing and managing assets in the CGame engine.

## Tools Overview

### 1. Asset Compiler (`asset_compiler.py`)

**Purpose**: Compiles source mesh assets into optimized `.cobj` files for the game engine.

**Usage**:
```bash
# Compile all assets (integrated into build system)
make with-assets

# Manual compilation
python3 tools/asset_compiler.py --source_dir assets/meshes --build_dir build/assets/meshes
```

**Features**:
- Converts `.obj` and `.mesh` files to `.cobj` format
- Validates mesh integrity and attempts repairs
- Generates per-mesh metadata in JSON format
- Creates central asset index for efficient loading
- Processes materials and textures

**Requirements**: `trimesh`, `cairosvg`, `numpy`, `scipy`, `jsonschema`

### 2. Mesh Viewer (`mesh_viewer/index.html`)

**Purpose**: Web-based 3D viewer for inspecting mesh assets during development.

**Usage**:
1. Compile assets first: `make with-assets`
2. Open `tools/mesh_viewer/index.html` in a web browser
3. Browse and select meshes from the sidebar
4. View 3D models with materials and textures

**Features**:
- Reads from compiled asset index to discover available meshes
- Attempts to load source files (`.obj`/`.mesh`) for viewing
- Falls back gracefully when source files are missing
- Displays materials, textures, and lighting
- Orbital camera controls for inspection
- Status indicators for source vs. compiled files

**Limitations**:
- Cannot display `.cobj` files (compiled format only readable by C engine)
- Requires source `.obj` or `.mesh` files to be present
- Some meshes may show as "compiled only"

### 3. Metadata Validator (`validate_metadata.py`)

**Purpose**: Validates mesh metadata files against the JSON schema.

**Usage**:
```bash
python3 tools/validate_metadata.py
```

**Features**:
- Validates all `metadata.json` files in the assets directory
- Checks compliance with the JSON schema
- Reports validation errors and warnings
- Ensures asset metadata consistency

### 4. Mesh Generator (`mesh_generator/`)

**Status**: Currently minimal/placeholder
**Purpose**: Intended for procedural mesh generation (future development)

## Development Workflow

### Asset Development Process

1. **Create Source Assets**:
   - Place `.obj` or `.mesh` files in `assets/meshes/category/`
   - Include corresponding `.mtl` and `.png` files
   - Follow naming conventions (snake_case)

2. **Compile Assets**:
   ```bash
   make with-assets
   ```

3. **Preview in Viewer**:
   - Open `tools/mesh_viewer/index.html`
   - Select mesh from sidebar to inspect

4. **Validate Metadata**:
   ```bash
   python3 tools/validate_metadata.py
   ```

5. **Test in Engine**:
   ```bash
   make run
   ```

### Troubleshooting

**"No source meshes found to compile"**:
- Check that files have `.obj` or `.mesh` extensions
- Verify files are in the correct directory structure
- Ensure `schema.json` exists in `assets/meshes/`

**"Cannot display compiled mesh" in viewer**:
- Source files (`.obj`/`.mesh`) are missing
- Only compiled `.cobj` files are available
- Copy/regenerate source files for viewing

**Asset compilation fails**:
- Check Python dependencies are installed
- Verify mesh file format and syntax
- Review console output for specific errors

## File Formats

### Source Formats
- **`.mesh`**: Standard Wavefront OBJ format with `.mesh` extension (preferred)
- **`.obj`**: Standard Wavefront OBJ format
- **`.mtl`**: Material definition files
- **`.png`**: Texture files (power-of-2 dimensions recommended)

### Compiled Formats
- **`.cobj`**: Compiled/optimized mesh format (engine-specific)
- **`.json`**: Per-mesh metadata files
- **`index.json`**: Central asset registry

## Integration Points

### Build System
- `Makefile` targets: `assets`, `with-assets`, `clean-assets`
- Automatic asset compilation during build process
- Optional compilation (won't fail build if Python deps missing)

### Engine Integration
- Assets loaded via `assets.c` asset registry
- Case-insensitive mesh name lookup
- Support for both source and compiled formats during development
- Production builds use only compiled assets

## Future Enhancements

### Planned Features
- **Mesh Generator**: Procedural geometry generation
- **Texture Pipeline**: Automated texture processing and optimization
- **Asset Browser**: Enhanced web-based asset management interface
- **Performance Profiler**: Asset loading and rendering performance analysis
- **Batch Operations**: Bulk asset processing and conversion tools

### Tool Improvements
- **Live Reload**: Auto-refresh mesh viewer when assets change
- **Error Reporting**: Better visualization of compilation errors
- **Asset Statistics**: Memory usage, polygon counts, performance metrics
- **Version Control**: Asset versioning and diff visualization
