#!/usr/bin/env python3
"""
Asset Compiler for CGame Engine

This script compiles source mesh assets (`.obj`, `.mesh`) into an optimized,
engine-ready format (`.cobj`). It performs the following key functions:

1.  **Mesh Processing**: Loads and validates source meshes, attempting to repair
    any corruption. It uses Delaunay triangulation for robust face generation.
2.  **Semantic Material Generation**: Creates `.mtl` files with physically-based
    properties derived from tags in the asset's `metadata.json`. This allows
    for an artist-friendly, data-driven material workflow.
3.  **Texture Generation**: Generates a spritesheet-style texture in SVG format
    with semantic colors based on the asset's tags. The SVG is then converted
    to a `.png` file.
4.  **UV Mapping**: Generates UV coordinates for each face, mapping them to the
    procedurally generated texture.
5.  **Metadata Compilation**: Creates a `metadata.json` file in the build
    directory, combining source metadata with build-specific information.
6.  **Index Creation**: Generates a central `index.json` file that lists all
    compiled assets, allowing the engine to efficiently discover and load them.

The compiler is designed to be integrated into the build system (via `make`)
and can be run manually for development and debugging.
"""
import argparse
import json
import math
import random
import re
import subprocess
import sys
from pathlib import Path
import numpy as np
from scipy.spatial import Delaunay
import os
import trimesh
from trimesh.exchange.export import export_mesh
from jsonschema import validate, ValidationError
import cairosvg

# --- Core Functions ---

def normalize(v):
    """Normalize a numpy vector."""
    norm = np.linalg.norm(v)
    if norm == 0:
        return v
    return v / norm

def get_random_color_hex():
    """Returns a random bright color in hex format."""
    return f"#{random.randint(128, 255):02X}{random.randint(128, 255):02X}{random.randint(128, 255):02X}"

# ============================================================================
# TAG-BASED SEMANTIC COLOR AND MATERIAL SYSTEM
# ============================================================================

# Global variable to cache loaded material definitions
TAG_MATERIAL_DEFINITIONS = None

def load_material_definitions(assets_dir="assets"):
    """Load material definitions from JSON file."""
    global TAG_MATERIAL_DEFINITIONS
    
    if TAG_MATERIAL_DEFINITIONS is not None:
        return TAG_MATERIAL_DEFINITIONS
    
    definitions_path = Path(assets_dir) / "material_definitions.json"
    
    try:
        with open(definitions_path, 'r') as f:
            data = json.load(f)
            TAG_MATERIAL_DEFINITIONS = data.get('definitions', {})
            print(f"✅ Loaded {len(TAG_MATERIAL_DEFINITIONS)} material definitions from {definitions_path}")
            return TAG_MATERIAL_DEFINITIONS
    except FileNotFoundError:
        print(f"⚠️  Material definitions file not found: {definitions_path}")
        print("   Using fallback default definitions.")
        # Provide minimal fallback definitions
        TAG_MATERIAL_DEFINITIONS = {
            'spacecraft': {
                'priority': 80,
                'colors': {'primary': '#8C9BAB', 'secondary': '#4A90E2', 'tertiary': '#FF6B35', 'accent': '#34495E'},
                'material': {'ambient': [0.2, 0.2, 0.3], 'diffuse': [0.6, 0.7, 0.9], 'specular': [0.8, 0.8, 0.8], 'shininess': 64.0, 'emission': [0.0, 0.0, 0.0]}
            },
            'building': {
                'priority': 60,
                'colors': {'primary': '#95A5A6', 'secondary': '#3498DB', 'tertiary': '#F1C40F', 'accent': '#2C3E50'},
                'material': {'ambient': [0.3, 0.3, 0.3], 'diffuse': [0.7, 0.7, 0.7], 'specular': [0.4, 0.4, 0.4], 'shininess': 16.0, 'emission': [0.0, 0.0, 0.0]}
            },
            'sun': {
                'priority': 100,
                'colors': {'primary': '#FFD700', 'secondary': '#FFA500', 'tertiary': '#FF6B35', 'accent': '#FFFF99'},
                'material': {'ambient': [1.0, 0.8, 0.4], 'diffuse': [1.0, 0.9, 0.6], 'specular': [1.0, 1.0, 1.0], 'shininess': 128.0, 'emission': [0.4, 0.3, 0.1]}
            }
        }
        return TAG_MATERIAL_DEFINITIONS
    except json.JSONDecodeError as e:
        print(f"❌ Error parsing material definitions JSON: {e}")
        print("   Using fallback default definitions.")
        TAG_MATERIAL_DEFINITIONS = {}
        return TAG_MATERIAL_DEFINITIONS

def get_material_definition_from_tags(tags):
    """Get the highest priority material definition based on tags."""
    if not tags:
        return None
    
    # Ensure definitions are loaded
    definitions = load_material_definitions()
    
    best_match = None
    highest_priority = -1
    
    for tag in tags:
        tag_lower = tag.lower().strip()
        if tag_lower in definitions:
            priority = definitions[tag_lower].get('priority', 0)
            if priority > highest_priority:
                highest_priority = priority
                best_match = definitions[tag_lower]
    
    return best_match

def get_semantic_color_for_mesh(mesh_name, face_index=None, tags=None):
    """Returns semantic colors based on tags with fallback to mesh name."""
    # Try tags first (new system)
    if tags:
        material_def = get_material_definition_from_tags(tags)
        if material_def:
            colors = material_def.get('colors', {})
            color_keys = list(colors.keys())
            if face_index is not None and len(color_keys) > 1:
                return colors[color_keys[face_index % len(color_keys)]]
            return colors.get('primary', '#8C9BAB')  # Fallback to metallic gray
    
    # Fallback to old mesh_name-based system for compatibility
    mesh_lower = mesh_name.lower()
    
    # Spacecraft materials
    if any(term in mesh_lower for term in ['ship', 'wedge', 'fighter', 'craft', 'vessel']):
        colors = ['#8C9BAB', '#4A90E2', '#FF6B35', '#34495E']
        return colors[face_index % len(colors)] if face_index is not None else colors[0]
    
    # Architectural structures
    elif any(term in mesh_lower for term in ['tower', 'building', 'structure', 'base']):
        colors = ['#95A5A6', '#3498DB', '#F1C40F', '#2C3E50']
        return colors[face_index % len(colors)] if face_index is not None else colors[0]
    
    # Celestial bodies
    elif any(term in mesh_lower for term in ['sun', 'star', 'solar']):
        return '#FFA500'  # Bright orange
    elif any(term in mesh_lower for term in ['planet', 'moon', 'asteroid']):
        return '#8B7355'  # Rocky brown
    
    # Default fallback to varied metallics
    metallic_colors = ['#8C9BAB', '#A8B2C1', '#7A8B9C', '#95A5A6']
    return metallic_colors[hash(mesh_name) % len(metallic_colors)]

def generate_semantic_mtl_file_from_tags(mtl_path, material_name, texture_filename, tags):
    """Generate a semantic MTL file using tag-based material definitions."""
    material_def = get_material_definition_from_tags(tags) if tags else None
    
    if material_def and 'material' in material_def:
        # Use tag-based material properties
        mat = material_def['material']
        ambient = f"{mat['ambient'][0]:.1f} {mat['ambient'][1]:.1f} {mat['ambient'][2]:.1f}"
        diffuse = f"{mat['diffuse'][0]:.1f} {mat['diffuse'][1]:.1f} {mat['diffuse'][2]:.1f}"
        specular = f"{mat['specular'][0]:.1f} {mat['specular'][1]:.1f} {mat['specular'][2]:.1f}"
        shininess = f"{mat['shininess']:.1f}"
        emission = mat.get('emission', [0.0, 0.0, 0.0])
        
        with open(mtl_path, 'w') as f:
            f.write(f"# Material file generated from tags: {', '.join(tags) if tags else 'none'}\n")
            f.write(f"# Material definition: {material_def.get('description', 'No description')}\n\n")
            f.write(f"newmtl {material_name}\n")
            f.write(f"Ka {ambient}  # Ambient color\n")
            f.write(f"Kd {diffuse}  # Diffuse color\n")
            f.write(f"Ks {specular}  # Specular color\n")
            f.write(f"Ns {shininess}         # Shininess\n")
            f.write(f"map_Kd {texture_filename}\n")
            
            # Add emission for glowing objects
            if emission[0] > 0.0 or emission[1] > 0.0 or emission[2] > 0.0:
                emission_str = f"{emission[0]:.1f} {emission[1]:.1f} {emission[2]:.1f}"
                f.write(f"Ke {emission_str}  # Emission color\n")
                f.write(f"map_Ke {texture_filename}\n")
        
        return True
    
    return False

# --- Geometry and UV Generation ---

def group_triangles_into_polygons(mesh):
    """Groups triangles into original polygonal faces based on face normals."""
    mesh.merge_vertices()
    try:
        # Use trimesh's built-in grouping which is more robust
        face_groups = trimesh.geometry.group_coplanar_faces(mesh)
        polygons = [trimesh.geometry.faces_to_polygon(mesh, faces) for faces in face_groups]
        return [tuple(p) for p in polygons]
    except Exception as e:
        print(f"Warning: Could not group faces into polygons: {e}. Falling back to triangles.", file=sys.stderr)
        return [tuple(f) for f in mesh.faces]

def triangulate(faces):
    """Triangulates polygonal faces."""
    tri_faces = []
    for face in faces:
        if len(face) < 3: continue
        if len(face) == 3:
            tri_faces.append(face)
        else: # Use trimesh for robust triangulation of complex polygons
            vertices = np.arange(max(max(f) for f in faces) + 1)
            polygon = trimesh.path.polygons.Polygon(face)
            tris = trimesh.path.polygons.triangulate(polygon)
            for tri in tris['faces']:
                tri_faces.append(tuple(tri))
    return tri_faces

def unwrap_and_generate_texture(mesh, svg_path, svg_width=1024, svg_height=1024, mesh_name="", tags=None):
    """
    Performs UV unwrapping on the mesh and generates an SVG texture
    that visualizes the unwrapped UV layout with semantic colors.
    """
    # Perform UV unwrapping using trimesh
    unwrapped = mesh.unwrap()

    # The unwrapped mesh now has visual.uv set
    uvs = unwrapped.visual.uv

    # Generate an SVG that visualizes the unwrapped UV layout
    with open(svg_path, 'w') as f:
        f.write(f'<svg width="{svg_width}" height="{svg_height}" xmlns="http://www.w3.org/2000/svg">\n')
        
        # Define semantic gradients
        material_def = get_material_definition_from_tags(tags)
        if material_def and 'colors' in material_def:
            colors = material_def['colors']
            primary_color = colors.get('primary', '#8C9BAB')
            secondary_color = colors.get('secondary', '#4A90E2')
        else:
            primary_color = '#8C9BAB'
            secondary_color = '#4A90E2'

        f.write('  <defs>\n')
        f.write(f'    <linearGradient id="semantic_gradient" x1="0%" y1="0%" x2="100%" y2="100%">\n')
        f.write(f'      <stop offset="0%" stop-color="{lighten_color(primary_color, 0.2)}" />\n')
        f.write(f'      <stop offset="100%" stop-color="{darken_color(secondary_color, 0.2)}" />\n')
        f.write('    </linearGradient>\n')
        f.write('  </defs>\n')
        
        # Background
        f.write('  <rect width="100%" height="100%" fill="#111"/>\n')

        # Draw the UV layout with semantic gradients
        for face_uv in uvs[unwrapped.faces]:
            points = " ".join([f"{uv[0] * svg_width},{uv[1] * svg_height}" for uv in face_uv])
            f.write(f'  <polygon points="{points}" fill="url(#semantic_gradient)" stroke="#000" stroke-width="1.5" opacity="0.8"/>\n')

        # Add title and instructions
        f.write('  <text x="10" y="30" fill="white" font-family="Arial" font-size="20" font-weight="bold">')
        f.write(f'{mesh_name.replace("_", " ").title()} UV Layout</text>\n')
        f.write('  <text x="10" y="50" fill="#fff" font-family="Arial" font-size="12">')
        f.write('This is the UV layout for the mesh. Edit this file to apply textures.</text>\n')
        
        f.write('</svg>\n')

    return unwrapped


def lighten_color(hex_color, factor):
    """Lighten a hex color by a factor (0.0-1.0)."""
    hex_color = hex_color.lstrip('#')
    rgb = tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))
    lightened = tuple(min(255, int(c + (255 - c) * factor)) for c in rgb)
    return f"#{lightened[0]:02X}{lightened[1]:02X}{lightened[2]:02X}"

def darken_color(hex_color, factor):
    """Darken a hex color by a factor (0.0-1.0)."""
    hex_color = hex_color.lstrip('#')
    rgb = tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))
    darkened = tuple(int(c * (1 - factor)) for c in rgb)
    return f"#{darkened[0]:02X}{darkened[1]:02X}{darkened[2]:02X}"

# --- File I/O and Conversion ---

def convert_svg_to_png(svg_path, png_path):
    """Converts SVG to PNG using the cairosvg library."""
    try:
        cairosvg.svg2png(url=str(svg_path), write_to=str(png_path))
    except Exception as e:
        print(f"Error during SVG to PNG conversion: {e}", file=sys.stderr)
        sys.exit(1)

def extract_material_name_from_obj(obj_path):
    """Extract the material name used in the OBJ file."""
    try:
        with open(obj_path, 'r') as f:
            for line in f:
                if line.startswith('usemtl '):
                    return line.strip().split(' ', 1)[1]
    except Exception as e:
        print(f"Warning: Could not read material from OBJ file: {e}", file=sys.stderr)
    return None

def copy_source_mtl_if_exists(source_dir, build_dir, mesh_name):
    """Copy existing MTL file from source to build directory, preserving materials."""
    # Look for material.mtl in the source mesh directory
    source_mtl = Path(source_dir) / "meshes" / "props" / mesh_name / "material.mtl"
    build_mtl = Path(build_dir) / "meshes" / "props" / mesh_name / "material.mtl"
    
    if source_mtl.exists():
        try:
            # Ensure build directory exists
            build_mtl.parent.mkdir(parents=True, exist_ok=True)
            import shutil
            shutil.copy2(source_mtl, build_mtl)
            print(f"✅ Preserved source material: {source_mtl.name}")
            return True
        except Exception as e:
            print(f"⚠️  Could not copy MTL file: {e}", file=sys.stderr)
    
    return False

def generate_semantic_mtl_file(mtl_path, material_name, texture_filename, mesh_name):
    """Generate a semantic MTL file with appropriate material properties."""
    mesh_lower = mesh_name.lower()
    
    # Determine material type and properties
    if any(term in mesh_lower for term in ['ship', 'wedge', 'fighter', 'craft', 'vessel']):
        # Spacecraft materials - metallic
        ambient = "0.2 0.2 0.3"
        diffuse = "0.6 0.7 0.9"
        specular = "0.8 0.8 0.8"
        shininess = "64.0"
    elif any(term in mesh_lower for term in ['tower', 'building', 'structure', 'base']):
        # Architectural materials - concrete/metal
        ambient = "0.3 0.3 0.3"
        diffuse = "0.7 0.7 0.7"
        specular = "0.4 0.4 0.4"
        shininess = "16.0"
    elif any(term in mesh_lower for term in ['sun', 'star', 'solar']):
        # Glowing materials
        ambient = "1.0 0.6 0.2"
        diffuse = "1.0 0.8 0.4"
        specular = "1.0 1.0 1.0"
        shininess = "100.0"
    else:
        # Default metallic material
        ambient = "0.2 0.2 0.3"
        diffuse = "0.6 0.7 0.8"
        specular = "0.8 0.8 0.8"
        shininess = "32.0"
    
    with open(mtl_path, 'w') as f:
        f.write(f"# Material file for {mesh_name}\n")
        f.write(f"# Generated with semantic properties\n\n")
        f.write(f"newmtl {material_name}\n")
        f.write(f"Ka {ambient}  # Ambient color\n")
        f.write(f"Kd {diffuse}  # Diffuse color\n")
        f.write(f"Ks {specular}  # Specular color\n")
        f.write(f"Ns {shininess}         # Shininess\n")
        f.write(f"map_Kd {texture_filename}\n")
        
        # Add emission for glowing objects
        if any(term in mesh_lower for term in ['sun', 'star', 'solar', 'engine']):
            f.write(f"Ke 0.3 0.2 0.1  # Emission color\n")
            f.write(f"map_Ke {texture_filename}\n")

def write_mtl_file(mtl_path, material_name, texture_filename):
    """Legacy MTL file generator - use generate_semantic_mtl_file instead."""
    with open(mtl_path, 'w') as f:
        f.write(f"newmtl {material_name}\n")
        f.write("Ke 1.0 1.0 1.0\n")
        f.write(f"map_Ke {texture_filename}\n")
        f.write("Kd 1.0 1.0 1.0\n")
        f.write(f"map_Kd {texture_filename}\n")

def write_compiled_obj(cobj_path, mesh, mtl_filename, material_name):
    """Writes the final compiled .cobj file with unwrapped UVs."""
    with open(cobj_path, 'w') as f:
        f.write(f"mtllib {Path(mtl_filename).name}\n")
        f.write(f"usemtl {material_name}\n\n")
        
        # Use the export_mesh function to write the OBJ data
        export_mesh(mesh, f, file_type='obj')


# --- Metadata and Validation ---

def validate_metadata(schema_path, data):
    # ... (This function remains the same)
    try:
        with open(schema_path, 'r') as f: schema = json.load(f)
        validate(instance=data, schema=schema)
        return True
    except FileNotFoundError:
        print(f"ERROR: Schema file not found at {schema_path}", file=sys.stderr)
        return False
    except ValidationError as e:
        print(f"ERROR: Metadata validation failed: {e.message} in {e.json_path}", file=sys.stderr)
        return False

def create_build_metadata(source_meta, schema_path, source_filename):
    """Creates build metadata from source metadata, adding build-specific fields."""
    # Start with a copy of source metadata to preserve all source information
    build_meta = source_meta.copy() if source_meta else {}
    
    # BUILD-SPECIFIC TRANSFORMATIONS:
    # 1. Always set the compiled mesh filename for the engine to load
    source_name = Path(source_filename).stem
    build_meta['geometry'] = f"{source_name}.cobj"  # Engine loads this file
    
    # 2. Track the original source file for reference/debugging
    build_meta['mesh'] = source_filename
    
    # 3. Add build timestamp for cache invalidation
    import time
    build_meta['build_timestamp'] = int(time.time())
    
    # ENSURE REQUIRED FIELDS (these should come from clean source metadata):
    if 'name' not in build_meta: 
        build_meta['name'] = Path(source_filename).parent.stem.replace('_', ' ').title()
    if 'tags' not in build_meta: 
        build_meta['tags'] = ["autogenerated"]
    if 'description' not in build_meta: 
        build_meta['description'] = "An automatically processed mesh."
    if 'material' not in build_meta: 
        build_meta['material'] = "material.mtl"
    if 'texture' not in build_meta: 
        build_meta['texture'] = "texture.png"
    
    # Validate the build metadata
    if not validate_metadata(schema_path, build_meta):
        print(f"ERROR: Build metadata validation failed for {source_filename}.", file=sys.stderr)
        return None
        
    return build_meta

def write_build_metadata(metadata_path, build_meta):
    """Writes the build metadata to file."""
    with open(metadata_path, 'w') as f:
        json.dump(build_meta, f, indent=4)
    return build_meta

# --- Build Pipeline ---

def compile_mesh_asset(source_path, build_dir, schema_path, overwrite=False):
    """
    Compiles a single mesh asset from a source file to the build directory.

    This function orchestrates the entire compilation process for a single
    asset, including loading the mesh, generating materials and textures,
    creating UV maps, and writing the final compiled object and metadata.

    Args:
        source_path (str or Path): The path to the source mesh file (.obj or .mesh).
        build_dir (str or Path): The root directory for compiled assets.
        schema_path (str or Path): The path to the metadata JSON schema.
        overwrite (bool, optional): If True, overwrite existing compiled files.
            Defaults to False.

    Returns:
        str or None: The relative path to the compiled metadata file if
        successful, otherwise None.
    """
    # Define source and build paths
    source_path = Path(source_path)
    mesh_name = source_path.stem
    
    # Define source and build paths
    source_dir = source_path.parent
    relative_path = source_dir.relative_to(Path(args.source_dir))
    build_dir_path = Path(build_dir) / relative_path
    build_dir_path.mkdir(parents=True, exist_ok=True)

    source_filename = source_path.name
    cobj_path = build_dir_path / f"{source_path.stem}.cobj"
    mtl_path = build_dir_path / "material.mtl"
    build_png_path = build_dir_path / "texture.png"
    metadata_path = build_dir_path / "metadata.json"

    # Source asset paths
    source_svg_path = source_dir / "texture.svg"
    source_png_path = source_dir / "texture.png"

    if not overwrite and cobj_path.exists():
        print(f"Skipping '{mesh_name}', compiled asset already exists.")
        return str(metadata_path.relative_to(build_dir))

    print(f"--- Compiling {source_path.name} ---")
    
    # 0. Load metadata FIRST to get tags for semantic color generation
    source_meta_path = source_path.parent / "metadata.json"
    source_meta = {}
    asset_tags = []
    asset_name = mesh_name.replace('_', ' ').title()
    
    if source_meta_path.exists():
        with open(source_meta_path, 'r') as f:
            try:
                source_meta = json.load(f)
                asset_tags = source_meta.get('tags', [])
                asset_name = source_meta.get('name', asset_name)
                print(f"Loaded source metadata from {source_meta_path.relative_to(Path(args.source_dir))}")
                print(f"   Asset: {asset_name} | Tags: {', '.join(asset_tags)}")
            except json.JSONDecodeError:
                print(f"Warning: Invalid JSON in {source_meta_path}. Starting with empty metadata.", file=sys.stderr)
    
    # Initialize material definitions (loads from JSON file)
    load_material_definitions()
    
    # 1. Create .obj copy in source directory if it's a .mesh file
    obj_copy_path = source_path.parent / "geometry.obj"
    if source_path.suffix == '.mesh' and not obj_copy_path.exists():
        print(f"Creating .obj copy for mesh viewer: {obj_copy_path.relative_to(Path(args.source_dir))}")
        try:
            import shutil
            shutil.copy2(source_path, obj_copy_path)
        except Exception as e:
            print(f"Warning: Could not create .obj copy: {e}", file=sys.stderr)
    
    # 2. Load Mesh
    try:
        file_type = 'obj' if source_path.suffix in ['.obj', '.mesh'] else None
        mesh = trimesh.load(str(source_path), file_type=file_type, force='mesh')
    except Exception as e:
        if "unpack_from requires a buffer of at least" in str(e) or "string or file could not be read to its end" in str(e):
            print(f"Warning: Mesh file {source_path} may be corrupt. Attempting repair.", file=sys.stderr)
            try:
                # Attempt to load with more lenient processing and re-export to fix it
                mesh = trimesh.load(str(source_path), file_type=file_type, process=False)
                mesh.export(str(source_path), file_type='obj')
                # Try loading again after repair
                mesh = trimesh.load(str(source_path), file_type=file_type, force='mesh')
                print("Successfully repaired and reloaded mesh.", file=sys.stderr)
            except Exception as repair_e:
                print(f"Error: Failed to repair mesh {source_path}: {repair_e}", file=sys.stderr)
                return None
        else:
            print(f"Error loading mesh {source_path}: {e}", file=sys.stderr)
            return None

    mesh.fix_normals() # Ensure winding is correct
    
    # 3. UV Unwrap and Texture Generation
    unwrapped_mesh = unwrap_and_generate_texture(mesh, source_svg_path, mesh_name=asset_name, tags=asset_tags)
    print(f"🎨 Generated artist-editable texture: {source_svg_path.relative_to(Path(args.source_dir).parent)}")

    # Generate PNG from SVG if it doesn't exist
    if not source_png_path.exists():
        print(f"🖼️ Generating default PNG texture from SVG...")
        convert_svg_to_png(source_svg_path, source_png_path)
    else:
        print(f"✅ Using existing artist-provided texture: {source_png_path.relative_to(Path(args.source_dir).parent)}")

    # Copy texture to build directory
    import shutil
    shutil.copy2(source_png_path, build_png_path)

    # 4. Create semantic MTL file
    source_mtl_path = source_dir / "material.mtl"
    material_name = asset_name
    if not source_mtl_path.exists():
        print(f"🎨 Generating default material: {source_mtl_path.relative_to(Path(args.source_dir).parent)}")
        if not (asset_tags and generate_semantic_mtl_file_from_tags(source_mtl_path, material_name, "texture.png", asset_tags)):
            generate_semantic_mtl_file(source_mtl_path, material_name, "texture.png", mesh_name)
    else:
        print(f"✅ Using existing artist-provided material: {source_mtl_path.relative_to(Path(args.source_dir).parent)}")

    # Copy material to build directory
    shutil.copy2(source_mtl_path, mtl_path)
    
    # Extract material name from the OBJ file or use the generated one
    material_name = extract_material_name_from_obj(source_path) or asset_name

    # 5. Write Compiled Mesh
    write_compiled_obj(cobj_path, unwrapped_mesh, "material.mtl", material_name)

    # 6. Create and Write Build Metadata
    build_meta = create_build_metadata(source_meta, schema_path, source_filename)
    if not build_meta:
        print(f"❌ Failed to create build metadata for {source_filename}")
        return None
        
    # Write the build metadata
    write_build_metadata(metadata_path, build_meta)
    print(f"Successfully compiled '{asset_name}'.\n")
    return str(metadata_path.relative_to(build_dir))

def clean_source_assets(source_dir):
    """
    Deletes generated files from the source asset directories.
    """
    print(f"Cleaning source assets in '{source_dir}'...")
    files_to_delete = [
        "texture.png",
        "texture.svg",
        "material.mtl",
        "geometry.obj"
    ]
    
    for file_name in files_to_delete:
        for path in Path(source_dir).glob(f"**/{file_name}"):
            try:
                path.unlink()
                print(f"Deleted {path}")
            except OSError as e:
                print(f"Error deleting {path}: {e}", file=sys.stderr)

def main():
    """
    Main function to run the asset compiler.

    Parses command-line arguments, discovers all mesh assets in the source
    directory, and orchestrates the compilation process for each asset.
    Finally, it generates the master asset index file.
    """
    parser = argparse.ArgumentParser(description="Compile mesh assets from the source directory to the build directory.")
    
    subparsers = parser.add_subparsers(dest="command", required=True)

    # Compile command
    parser_compile = subparsers.add_parser("compile", help="Compile all assets.")
    parser_compile.add_argument("--source_dir", default="assets/meshes", help="Source directory for mesh assets.")
    parser_compile.add_argument("--build_dir", default="build/assets/meshes", help="Build directory for compiled assets.")
    parser_compile.add_argument("--overwrite", action="store_true", help="Overwrite existing files in the build directory.")

    # Clean command
    parser_clean = subparsers.add_parser("clean", help="Clean generated files from source directories.")
    parser_clean.add_argument("--source_dir", default="assets/meshes", help="Source directory for mesh assets.")

    global args
    args = parser.parse_args()

    if args.command == "clean":
        clean_source_assets(args.source_dir)
        return

    source_dir = Path(args.source_dir)
    build_dir = Path(args.build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)
    
    schema_path = source_dir / "schema.json"
    if not schema_path.exists():
        print(f"ERROR: schema.json not found in '{source_dir}'.", file=sys.stderr)
        sys.exit(1)

    print(f"Scanning '{source_dir}' for source meshes (.obj, .mesh)...")
    mesh_files = list(source_dir.glob("**/*.obj")) + list(source_dir.glob("**/*.mesh"))
    
    if not mesh_files:
        print("No source meshes found to compile.")
        # Create an empty index if no meshes are found
        index_path = build_dir / "index.json"
        with open(index_path, 'w') as f:
            json.dump([], f)
        return

    index_paths = []
    processed_meshes = set()  # Track processed mesh directories to avoid duplicates
    
    for mesh_path in mesh_files:
        # Skip metadata files
        if mesh_path.suffix == '.json':
            continue
            
        # Create a unique identifier for this mesh based on its directory and stem
        mesh_id = f"{mesh_path.parent.relative_to(source_dir)}/{mesh_path.stem}"
        
        if mesh_id in processed_meshes:
            print(f"Skipping duplicate mesh: {mesh_path.name} (already processed from same directory)")
            continue
            
        processed_meshes.add(mesh_id)
        meta_path = compile_mesh_asset(mesh_path, build_dir, schema_path, args.overwrite)
        if meta_path:
            index_paths.append(meta_path)
        
    # Write the index file (remove duplicates just in case)
    unique_index_paths = sorted(list(set(index_paths)))
    index_path = build_dir / "index.json"
    with open(index_path, 'w') as f:
        json.dump(unique_index_paths, f, indent=4)
        
    print(f"Asset compilation complete. Index written to {index_path}")

if __name__ == "__main__":
    main()

