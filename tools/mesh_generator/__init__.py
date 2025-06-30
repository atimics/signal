#!/usr/bin/env python3
"""
CGame Mesh Generator - Integrated Asset Pipeline

This module provides procedural mesh generation integrated with the asset compiler.
Generated meshes automatically get compiled with proper metadata, materials, and textures.
"""

import json
import math
import random
from pathlib import Path
import numpy as np

# Import from parent asset compiler
import sys
sys.path.append(str(Path(__file__).parent.parent))
from asset_compiler import (
    compile_mesh_asset, 
    validate_metadata, 
    create_build_metadata,
    write_build_metadata,
    generate_spritesheet_uvs_and_svg,
    convert_svg_to_png,
    write_mtl_file,
    triangulate
)

def generate_wedge_ship_mk2():
    """Generate an improved wedge ship with better proportions."""
    vertices = [
        # Front point
        [0.0, 0.0, 2.0],
        # Main body - top
        [-1.0, 0.5, -1.0],
        [1.0, 0.5, -1.0],
        # Main body - bottom
        [-1.0, -0.5, -1.0],
        [1.0, -0.5, -1.0],
        # Wing tips
        [-2.0, 0.0, -0.5],
        [2.0, 0.0, -0.5],
        # Engine exhausts
        [-0.5, -0.2, -2.0],
        [0.5, -0.2, -2.0],
    ]
    
    faces = [
        # Top surface
        [0, 2, 1],  # front triangle
        [1, 2, 6, 5],  # top wing span
        # Bottom surface  
        [0, 3, 4],  # front triangle
        [3, 5, 6, 4],  # bottom wing span
        # Sides
        [0, 1, 5, 3],  # left side
        [0, 4, 6, 2],  # right side
        # Back
        [1, 7, 8, 2],  # engine area
        [3, 7, 8, 4],  # engine bottom
        [5, 7, 8, 6],  # engine sides
    ]
    
    return vertices, faces

def generate_control_tower():
    """
    Generate a more detailed control tower with a cylindrical base,
    an octagonal observation deck, and an antenna.
    This version includes UV coordinates.
    """
    vertices = []
    faces = []
    uvs = []

    sides = 12  # A smoother cylinder for the base

    # --- 1. Cylindrical Base ---
    base_radius = 1.5
    base_height = 4.0
    
    # Bottom circle
    for i in range(sides):
        angle = 2 * math.pi * i / sides
        x, z = base_radius * math.cos(angle), base_radius * math.sin(angle)
        vertices.append([x, 0, z])
        uvs.append([i / sides, 0])
    # Top circle
    for i in range(sides):
        angle = 2 * math.pi * i / sides
        x, z = base_radius * math.cos(angle), base_radius * math.sin(angle)
        vertices.append([x, base_height, z])
        uvs.append([i / sides, 0.5]) # Use half the texture height for the base

    # Base faces
    for i in range(sides):
        i_next = (i + 1) % sides
        # Side faces
        v1, v2 = i, i_next
        v3, v4 = i + sides, i_next + sides
        faces.append([v1, v2, v4, v3])

    # --- 2. Octagonal Observation Deck ---
    deck_sides = 8
    deck_radius = 2.5
    deck_height = 1.0
    deck_y_start = base_height

    deck_base_start_idx = len(vertices)
    # Bottom octagon
    for i in range(deck_sides):
        angle = 2 * math.pi * i / deck_sides
        x, z = deck_radius * math.cos(angle), deck_radius * math.sin(angle)
        vertices.append([x, deck_y_start, z])
        uvs.append([0.125 * i, 0.55])
    # Top octagon
    for i in range(deck_sides):
        angle = 2 * math.pi * i / deck_sides
        x, z = deck_radius * math.cos(angle), deck_radius * math.sin(angle)
        vertices.append([x, deck_y_start + deck_height, z])
        uvs.append([0.125 * i, 0.75])

    # Deck faces
    for i in range(deck_sides):
        i_next = (i + 1) % deck_sides
        v1 = deck_base_start_idx + i
        v2 = deck_base_start_idx + i_next
        v3 = deck_base_start_idx + i + deck_sides
        v4 = deck_base_start_idx + i_next + deck_sides
        faces.append([v1, v2, v4, v3]) # Side panels
        faces.append([deck_base_start_idx, v2, v1]) # Bottom fan
        faces.append([deck_base_start_idx + deck_sides, v3, v4]) # Top fan

    # --- 3. Antenna Spire ---
    spire_base_radius = 0.2
    spire_height = 3.0
    spire_y_start = deck_y_start + deck_height
    
    spire_base_idx = len(vertices)
    # Spire base
    for i in range(4):
        angle = 2 * math.pi * i / 4
        x, z = spire_base_radius * math.cos(angle), spire_base_radius * math.sin(angle)
        vertices.append([x, spire_y_start, z])
        uvs.append([0.25 * i, 0.8])
        
    # Spire top
    vertices.append([0, spire_y_start + spire_height, 0])
    uvs.append([0.5, 1.0])
    spire_top_idx = len(vertices) - 1

    # Spire faces
    for i in range(4):
        i_next = (i + 1) % 4
        v1 = spire_base_idx + i
        v2 = spire_base_idx + i_next
        faces.append([v1, v2, spire_top_idx])

    return vertices, faces, uvs

def generate_sun_sphere():
    """Generate a low-poly sphere for the sun."""
    vertices = []
    faces = []
    
    # Icosphere approach - start with icosahedron
    phi = (1 + math.sqrt(5)) / 2  # Golden ratio
    radius = 1.0
    
    # 12 vertices of icosahedron
    icosa_verts = [
        [-1, phi, 0], [1, phi, 0], [-1, -phi, 0], [1, -phi, 0],
        [0, -1, phi], [0, 1, phi], [0, -1, -phi], [0, 1, -phi],
        [phi, 0, -1], [phi, 0, 1], [-phi, 0, -1], [-phi, 0, 1],
    ]
    
    # Normalize to unit sphere
    for vert in icosa_verts:
        length = math.sqrt(sum(x*x for x in vert))
        vertices.append([x * radius / length for x in vert])
    
    # 20 faces of icosahedron
    icosa_faces = [
        [0, 11, 5], [0, 5, 1], [0, 1, 7], [0, 7, 10], [0, 10, 11],
        [1, 5, 9], [5, 11, 4], [11, 10, 2], [10, 7, 6], [7, 1, 8],
        [3, 9, 4], [3, 4, 2], [3, 2, 6], [3, 6, 8], [3, 8, 9],
        [4, 9, 5], [2, 4, 11], [6, 2, 10], [8, 6, 7], [9, 8, 1],
    ]
    faces.extend(icosa_faces)
    
    return vertices, faces

def write_obj_file(filepath, vertices, faces):
    """Write vertices and faces to OBJ file format."""
    with open(filepath, 'w') as f:
        f.write("# Generated by CGame Mesh Generator\n")
        f.write(f"# Vertices: {len(vertices)}, Faces: {len(faces)}\n\n")
        
        # Write vertices
        for v in vertices:
            f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        f.write("\n")
        
        # Write faces (OBJ uses 1-based indexing)
        for face in faces:
            if len(face) == 3:
                f.write(f"f {face[0]+1} {face[1]+1} {face[2]+1}\n")
            elif len(face) == 4:
                f.write(f"f {face[0]+1} {face[1]+1} {face[2]+1} {face[3]+1}\n")
            else:
                # For polygons with more than 4 vertices, triangulate
                for i in range(1, len(face) - 1):
                    f.write(f"f {face[0]+1} {face[i]+1} {face[i+1]+1}\n")

def create_mesh_metadata(name, description, tags):
    """Create standardized metadata for generated meshes."""
    return {
        "name": name,
        "filename": "geometry.mesh",  # Source filename (will be converted to .cobj in build)
        "tags": tags,
        "description": description,
        "mtl": "material.mtl",
        "texture": "texture.png"
    }

def generate_mesh_with_textures(mesh_name, generator_func, source_dir, build_dir, schema_path):
    """Generate a mesh with proper UV mapping and texture generation."""
    print(f"🔧 Generating mesh: {mesh_name}")
    
    # Create source directory
    mesh_source_dir = Path(source_dir) / "props" / mesh_name
    mesh_source_dir.mkdir(parents=True, exist_ok=True)
    
    # Create build directory
    mesh_build_dir = Path(build_dir) / "props" / mesh_name
    mesh_build_dir.mkdir(parents=True, exist_ok=True)
    
    # Generate mesh geometry
    vertices, faces = generator_func()
    
    # Write source mesh file (.mesh format for our pipeline)
    mesh_file = mesh_source_dir / "geometry.mesh"
    write_obj_file(mesh_file, vertices, faces)
    
    # Create metadata based on mesh type
    if mesh_name == "wedge_ship_mk2":
        metadata = create_mesh_metadata(
            "Wedge Ship Mk2",
            "An improved version of the wedge-shaped spacecraft with enhanced proportions",
            ["ship", "vehicle", "spacecraft", "improved", "player"]
        )
    elif mesh_name == "control_tower":
        metadata = create_mesh_metadata(
            "Control Tower", 
            "A detailed control tower with an observation deck and antenna.",
            ["building", "structure", "spaceport", "tower", "architectural"]
        )
    elif mesh_name == "sun":
        metadata = create_mesh_metadata(
            "Sun",
            "A low-poly sphere representing the sun or star",
            ["celestial", "sphere", "lighting"]
        )
    else:
        metadata = create_mesh_metadata(
            mesh_name.replace('_', ' ').title(),
            f"A procedurally generated {mesh_name}",
            ["generated", "procedural"]
        )
    
    # Write source metadata
    metadata_file = mesh_source_dir / "metadata.json"
    with open(metadata_file, 'w') as f:
        json.dump(metadata, f, indent=4)
    
    print(f"   📄 Created source files in {mesh_source_dir}")
    
    # Generate texture map using the same pipeline as asset compiler
    print(f"   🎨 Generating texture map for manual editing...")
    
    # Convert faces to the format expected by the texture generator
    face_tuples = [tuple(face) for face in faces]
    
    # Generate SVG texture with face outlines in SOURCE directory for manual editing
    svg_path = mesh_source_dir / f"{mesh_name}_texture.svg"
    uv_coords_per_face = generate_spritesheet_uvs_and_svg(face_tuples, svg_path)
    
    # Convert SVG to PNG in SOURCE directory for manual editing
    png_path = mesh_source_dir / "texture.png"
    convert_svg_to_png(svg_path, png_path)
    
    print(f"   ✅ Created texture files for manual editing:")
    print(f"      📄 {svg_path.relative_to(Path(source_dir))} (editable SVG)")
    print(f"      🖼️  {png_path.relative_to(Path(source_dir))} (base texture)")
    
    # Generate MTL file in SOURCE directory for manual editing
    mtl_path = mesh_source_dir / "material.mtl"
    material_name = mesh_name.replace('_', ' ').title()
    write_mtl_file(mtl_path, material_name, "texture.png")  # Reference the source texture
    
    print(f"   � Created material file: {mtl_path.relative_to(Path(source_dir))}")
    print(f"   � Created source files in {mesh_source_dir}")
    
    print(f"   💡 You can now manually edit the texture files before compilation:")
    print(f"      - Edit {svg_path.name} for custom face textures/decals")
    print(f"      - Edit {png_path.name} for final texture")
    print(f"      - Edit {mtl_path.name} for material properties")
    
    # Now compile through the normal asset pipeline
    try:
        # Import and call the compile function directly
        from asset_compiler import compile_mesh_asset
        
        # Create a temporary args namespace for the asset compiler
        import argparse
        temp_args = argparse.Namespace()
        temp_args.source_dir = str(source_dir)
        
        # Temporarily set the global args
        import asset_compiler
        original_args = getattr(asset_compiler, 'args', None)
        asset_compiler.args = temp_args
        
        try:
            result = compile_mesh_asset(mesh_file, build_dir, schema_path, overwrite=True)
            if result:
                print(f"   ✅ Compiled mesh to build directory")
                return True
            else:
                print(f"   ❌ Failed to compile mesh")
                return False
        finally:
            # Restore original args
            if original_args is not None:
                asset_compiler.args = original_args
            elif hasattr(asset_compiler, 'args'):
                delattr(asset_compiler, 'args')
            
    except Exception as e:
        print(f"   ❌ Compilation error: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Main entry point for mesh generation."""
    import argparse
    
    parser = argparse.ArgumentParser(description="Generate procedural meshes and compile them")
    parser.add_argument("--source_dir", default="assets/meshes", help="Source directory")
    parser.add_argument("--build_dir", default="build/assets/meshes", help="Build directory") 
    parser.add_argument("--mesh", help="Specific mesh to generate (wedge_ship_mk2, control_tower, sun)")
    parser.add_argument("--all", action="store_true", help="Generate all meshes")
    
    args = parser.parse_args()
    
    source_dir = Path(args.source_dir)
    build_dir = Path(args.build_dir)
    schema_path = source_dir / "schema.json"
    
    if not schema_path.exists():
        print(f"❌ Schema file not found: {schema_path}")
        return 1
    
    # Available mesh generators
    generators = {
        "wedge_ship_mk2": generate_wedge_ship_mk2,
        "control_tower": generate_control_tower,
        "sun": generate_sun_sphere,
    }
    
    success_count = 0
    
    if args.all:
        for mesh_name, generator in generators.items():
            if generate_mesh_with_textures(mesh_name, generator, source_dir, build_dir, schema_path):
                success_count += 1
    elif args.mesh:
        if args.mesh in generators:
            if generate_mesh_with_textures(args.mesh, generators[args.mesh], source_dir, build_dir, schema_path):
                success_count += 1
        else:
            print(f"❌ Unknown mesh: {args.mesh}")
            print(f"Available meshes: {', '.join(generators.keys())}")
            return 1
    else:
        print("Specify --mesh <name> or --all")
        print(f"Available meshes: {', '.join(generators.keys())}")
        return 1
    
    print(f"\n🎯 Generated and compiled {success_count} meshes")
    return 0

if __name__ == "__main__":
    sys.exit(main())
