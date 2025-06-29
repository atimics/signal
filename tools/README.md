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
- **Semantic Material Generation**: Creates materials based on metadata tags
- **SVG Template Processing**: Generates textures from SVG templates using tag-based colors
- **Tag-Based Pipeline**: Uses `assets/material_definitions.json` for semantic generation
- Validates mesh integrity and attempts repairs
- Generates per-mesh metadata in JSON format
- Creates central asset index for efficient loading
- **Material Property Assignment**: Applies ambient, diffuse, specular, and emission properties

**Semantic Material System**:
- Reads metadata tags from asset files (e.g., ["ship", "vehicle", "spacecraft"])
- Loads tag definitions with priorities, colors, and material properties
- Generates semantic colors for SVG texture templates
- Creates MTL files with physically-based material properties
- Supports artist-friendly workflow with JSON-based definitions

**Requirements**: `trimesh`, `cairosvg`, `numpy`, `scipy`, `jsonschema`

### 2. Semantic Material Mesh Viewer (`mesh_viewer/index.html`)

**Purpose**: Web-based 3D viewer for inspecting mesh assets with semantic material analysis.

**Usage**:
1. Start a local HTTP server from project root: `python3 -m http.server 8081`
2. Open `http://localhost:8081/tools/mesh_viewer/index.html` in a web browser
3. Use the Source/Compiled toggle to switch between asset modes
4. Browse and select meshes from the sidebar
5. View 3D models with semantic material information

**Features**:
- **Dual Mode Operation**: Switch between source and compiled asset viewing
- **Tag-Based Analysis**: Displays semantic tags with priorities and descriptions
- **Material Preview**: Shows dominant material colors and properties
- **Semantic Information**: Displays tag priorities, material coefficients, and color palettes
- **Asset Metadata**: Shows comprehensive asset information including descriptions
- **3D Rendering**: Orbital camera controls with material and texture support
- **Status Indicators**: Clear feedback on loading success/failure

**Semantic Material System**:
- Loads tag definitions from `assets/material_definitions.json`
- Analyzes asset tags to determine dominant materials
- Displays color swatches based on tag priorities
- Shows material properties (ambient, diffuse, specular, emission)
- Supports artist-friendly tag visualization

**Requirements**: 
- Local HTTP server (for CORS policy compliance)
- Modern web browser with WebGL support
- Three.js library (loaded via CDN)

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
   - Include corresponding `.mtl` and `.png` files (or `.svg` templates)
   - Add semantic tags to `metadata.json` files
   - Follow naming conventions (snake_case)

2. **Configure Semantic Materials**:
   - Edit `assets/material_definitions.json` to add/modify tags
   - Define priorities, colors, and material properties
   - Use high-priority tags for unique appearances (stars, engines)
   - Use low-priority tags for environmental modifiers

2. **Compile Assets**:
   ```bash
   make with-assets
   ```

3. **Preview in Semantic Viewer**:
   - Start HTTP server: `python3 -m http.server 8081`
   - Open `http://localhost:8081/tools/mesh_viewer/index.html`
   - Use Source/Compiled toggle to inspect both asset types
   - Review semantic material information and tag analysis

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
