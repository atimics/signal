#!/usr/bin/env python3
"""
CGame Mesh Generator - Source Asset Creator

This module provides functions for procedurally generating source mesh assets
in the .obj format, complete with UVs, normals, and material files.
It is designed to be called by the main build pipeline.
"""

import json
import math
import sys
from pathlib import Path
import numpy as np
import argparse

def generate_wedge_ship_mk2():
    """Generate an improved wedge ship with better proportions and UVs."""
    vertices = [
        [0.0, 0.0, 2.0], [-1.0, 0.5, -1.0], [1.0, 0.5, -1.0],
        [-1.0, -0.5, -1.0], [1.0, -0.5, -1.0], [-2.0, 0.0, -0.5],
        [2.0, 0.0, -0.5], [-0.5, -0.2, -2.0], [0.5, -0.2, -2.0],
    ]
    uvs = [
        [0.5, 1.0], [0.0, 0.5], [1.0, 0.5], [0.0, 0.2], [1.0, 0.2],
        [0.0, 0.0], [1.0, 0.0], [0.25, 0.0], [0.75, 0.0]
    ]
    faces = [
        [0, 2, 1], [1, 2, 6, 5], [0, 3, 4], [3, 5, 6, 4],
        [0, 1, 5, 3], [0, 4, 6, 2], [1, 7, 8, 2],
        [3, 7, 8, 4], [5, 7, 8, 6]
    ]
    return vertices, faces, uvs

def generate_control_tower():
    """
    Generate a more detailed control tower with a cylindrical base,
    an octagonal observation deck, and an antenna.
    This version includes UV coordinates.
    """
    vertices, faces, uvs = [], [], []
    sides = 12
    base_radius, base_height = 1.5, 4.0
    for i in range(sides):
        angle = 2 * math.pi * i / sides
        x, z = base_radius * math.cos(angle), base_radius * math.sin(angle)
        vertices.extend([[x, 0, z], [x, base_height, z]])
        uvs.extend([[i / sides, 0], [i / sides, 0.5]])
    for i in range(sides):
        i_next = (i + 1) % sides
        v1, v2 = i * 2, i_next * 2
        v3, v4 = v1 + 1, v2 + 1
        faces.append([v1, v2, v4, v3])

    deck_sides, deck_radius, deck_height, deck_y_start = 8, 2.5, 1.0, base_height
    deck_base_start_idx = len(vertices)
    for i in range(deck_sides):
        angle = 2 * math.pi * i / deck_sides
        x, z = deck_radius * math.cos(angle), deck_radius * math.sin(angle)
        vertices.extend([[x, deck_y_start, z], [x, deck_y_start + deck_height, z]])
        uvs.extend([[0.125 * i, 0.55], [0.125 * i, 0.75]])
    for i in range(deck_sides):
        i_next = (i + 1) % deck_sides
        v1 = deck_base_start_idx + i * 2
        v2 = deck_base_start_idx + i_next * 2
        v3, v4 = v1 + 1, v2 + 1
        faces.extend([[v1, v2, v4, v3], [deck_base_start_idx, v2, v1], [v3, v4, deck_base_start_idx + 1]])

    spire_base_radius, spire_height, spire_y_start = 0.2, 3.0, deck_y_start + deck_height
    spire_base_idx = len(vertices)
    for i in range(4):
        angle = 2 * math.pi * i / 4
        x, z = spire_base_radius * math.cos(angle), spire_base_radius * math.sin(angle)
        vertices.append([x, spire_y_start, z])
        uvs.append([0.25 * i, 0.8])
    vertices.append([0, spire_y_start + spire_height, 0])
    uvs.append([0.5, 1.0])
    spire_top_idx = len(vertices) - 1
    for i in range(4):
        i_next = (i + 1) % 4
        faces.append([spire_base_idx + i, spire_base_idx + i_next, spire_top_idx])

    return vertices, faces, uvs

def generate_sun_sphere():
    """Generate a low-poly icosphere for the sun with basic UVs."""
    phi = (1 + math.sqrt(5)) / 2
    verts = np.array([
        [-1, phi, 0], [1, phi, 0], [-1, -phi, 0], [1, -phi, 0],
        [0, -1, phi], [0, 1, phi], [0, -1, -phi], [0, 1, -phi],
        [phi, 0, -1], [phi, 0, 1], [-phi, 0, -1], [-phi, 0, 1],
    ])
    verts /= np.linalg.norm(verts, axis=1)[:, np.newaxis]
    faces = np.array([
        [0, 11, 5], [0, 5, 1], [0, 1, 7], [0, 7, 10], [0, 10, 11],
        [1, 5, 9], [5, 11, 4], [11, 10, 2], [10, 7, 6], [7, 1, 8],
        [3, 9, 4], [3, 4, 2], [3, 2, 6], [3, 6, 8], [3, 8, 9],
        [4, 9, 5], [2, 4, 11], [6, 2, 10], [8, 6, 7], [9, 8, 1],
    ])
    uvs = [[np.arctan2(v[0], v[2]) / (2 * np.pi) + 0.5, np.arcsin(v[1]) / np.pi + 0.5] for v in verts]
    return verts.tolist(), faces.tolist(), uvs

def generate_planet_surface():
    """Generate a large flat plane to represent the planet surface."""
    size = 50.0  # Large surface
    vertices = [
        [-size, 0, -size], [size, 0, -size], [size, 0, size], [-size, 0, size]
    ]
    uvs = [
        [0, 0], [10, 0], [10, 10], [0, 10]  # Tiled UVs for detail
    ]
    faces = [
        [0, 1, 2], [0, 2, 3]  # Two triangles forming a quad
    ]
    return vertices, faces, uvs

def generate_landing_pad():
    """Generate a hexagonal landing pad with markers."""
    vertices = []
    faces = []
    uvs = []
    
    # Central hexagon
    radius = 3.0
    height = 0.2
    sides = 6
    
    # Bottom center point
    vertices.append([0, 0, 0])
    uvs.append([0.5, 0.5])
    
    # Top center point  
    vertices.append([0, height, 0])
    uvs.append([0.5, 0.5])
    
    # Bottom and top ring vertices
    for i in range(sides):
        angle = 2 * math.pi * i / sides
        x = radius * math.cos(angle)
        z = radius * math.sin(angle)
        
        # Bottom vertex
        vertices.append([x, 0, z])
        uvs.append([0.5 + 0.5 * math.cos(angle), 0.5 + 0.5 * math.sin(angle)])
        
        # Top vertex
        vertices.append([x, height, z])
        uvs.append([0.5 + 0.5 * math.cos(angle), 0.5 + 0.5 * math.sin(angle)])
    
    # Create faces
    for i in range(sides):
        i_next = (i + 1) % sides
        
        # Bottom face triangles
        faces.append([0, 2 + i * 2, 2 + i_next * 2])
        
        # Top face triangles  
        faces.append([1, 3 + i_next * 2, 3 + i * 2])
        
        # Side faces (quads as two triangles)
        bottom_curr = 2 + i * 2
        top_curr = 3 + i * 2
        bottom_next = 2 + i_next * 2
        top_next = 3 + i_next * 2
        
        faces.append([bottom_curr, top_curr, top_next])
        faces.append([bottom_curr, top_next, bottom_next])
    
    return vertices, faces, uvs

def generate_wedge_ship():
    """Generate a simpler wedge ship for AI ships."""
    vertices = [
        [0.0, 0.0, 1.5],   # Nose
        [-0.8, 0.3, -1.0], [0.8, 0.3, -1.0],   # Top wing tips
        [-0.8, -0.3, -1.0], [0.8, -0.3, -1.0], # Bottom wing tips
        [-0.3, -0.1, -1.5], [0.3, -0.1, -1.5]  # Rear engines
    ]
    uvs = [
        [0.5, 1.0], [0.0, 0.5], [1.0, 0.5], [0.0, 0.2], [1.0, 0.2], [0.25, 0.0], [0.75, 0.0]
    ]
    faces = [
        [0, 2, 1], [1, 2, 4, 3], [0, 3, 4], [0, 4, 2], [0, 1, 3], [3, 5, 6, 4], [1, 5, 6, 2]
    ]
    return vertices, faces, uvs

def write_obj_file(filepath, vertices, uvs, faces):
    """Write vertices, UVs, normals, and faces to an OBJ file."""
    normals = np.zeros((len(vertices), 3), dtype=np.float32)
    for face in faces:
        face_verts = np.array([vertices[i] for i in face[:3]])
        normal = np.cross(face_verts[1] - face_verts[0], face_verts[2] - face_verts[0])
        normal /= np.linalg.norm(normal)
        for i in face:
            normals[i] += normal
    
    for i in range(len(normals)):
        norm = np.linalg.norm(normals[i])
        if norm > 0:
            normals[i] /= norm

    with open(filepath, 'w') as f:
        f.write("# Generated by CGame Mesh Generator\n")
        for v in vertices:
            f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        for vt in uvs:
            f.write(f"vt {vt[0]:.6f} {vt[1]:.6f}\n")
        for vn in normals:
            f.write(f"vn {vn[0]:.6f} {vn[1]:.6f} {vn[2]:.6f}\n")
        f.write("usemtl default\n")
        for face in faces:
            f.write("f")
            for i in face:
                f.write(f" {i+1}/{i+1}/{i+1}")
            f.write("\n")

def create_mesh_metadata(name, description, tags):
    """Create standardized metadata for generated meshes."""
    return {
        "name": name,
        "geometry": "geometry.obj",
        "tags": tags,
        "description": description,
        "material": "material.mtl",
        "texture": "texture.png"
    }

def write_mtl_file(filepath, material_name, texture_filename):
    """Writes a simple .mtl file."""
    with open(filepath, 'w') as f:
        f.write(f"newmtl {material_name}\n")
        f.write("Ka 1.0 1.0 1.0\n")
        f.write("Kd 1.0 1.0 1.0\n")
        f.write("Ks 0.5 0.5 0.5\n")
        f.write("Ns 10.0\n")
        f.write(f"map_Kd {texture_filename}\n")

def generate_source_asset(mesh_name, generator_func, source_dir):
    """Generate all source files for a procedural asset."""
    print(f"🔧 Generating source asset: {mesh_name}")
    mesh_source_dir = Path(source_dir) / "props" / mesh_name
    mesh_source_dir.mkdir(parents=True, exist_ok=True)

    vertices, faces, uvs = generator_func()
    
    # Write .obj file
    obj_path = mesh_source_dir / "geometry.obj"
    write_obj_file(obj_path, vertices, uvs, faces)

    # Write metadata.json
    if mesh_name == "wedge_ship_mk2":
        metadata = create_mesh_metadata("Wedge Ship Mk2", "An improved wedge-shaped spacecraft.", ["ship", "vehicle", "player"])
    elif mesh_name == "control_tower":
        metadata = create_mesh_metadata("Control Tower", "A control tower for spaceport environments.", ["building", "structure"])
    elif mesh_name == "sun":
        metadata = create_mesh_metadata("Sun", "A low-poly sphere representing a star.", ["celestial", "star"])
    else:
        metadata = create_mesh_metadata(mesh_name.replace('_', ' ').title(), "A procedurally generated mesh.", ["procedural"])
    
    with open(mesh_source_dir / "metadata.json", 'w') as f:
        json.dump(metadata, f, indent=2)

    # Write .mtl file
    write_mtl_file(mesh_source_dir / "material.mtl", metadata["name"], metadata["texture"])

    # Create a placeholder texture.png
    from PIL import Image
    img = Image.new('RGB', (512, 512), color = 'grey')
    img.save(mesh_source_dir / "texture.png")
    
    print(f"   ✅ Created source files in {mesh_source_dir}")
    return True

def main():
    """Main entry point for mesh generation."""
    parser = argparse.ArgumentParser(description="Generate procedural source assets.")
    parser.add_argument("--source_dir", default="assets/meshes", help="Root source directory for assets")
    parser.add_argument("--mesh", help="Specific mesh to generate (wedge_ship_mk2, control_tower, sun)")
    parser.add_argument("--all", action="store_true", help="Generate all procedural meshes")
    args = parser.parse_args()
    
    generators = {
        "wedge_ship_mk2": generate_wedge_ship_mk2,
        "wedge_ship": generate_wedge_ship,
        "control_tower": generate_control_tower,
        "sun": generate_sun_sphere,
        "planet_surface": generate_planet_surface,
        "landing_pad": generate_landing_pad,
    }
    
    if args.all:
        for name, func in generators.items():
            generate_source_asset(name, func, args.source_dir)
    elif args.mesh:
        if args.mesh in generators:
            generate_source_asset(args.mesh, generators[args.mesh], args.source_dir)
        else:
            print(f"❌ Unknown mesh: {args.mesh}. Available: {', '.join(generators.keys())}")
            return 1
    else:
        print("Usage: --mesh <name> or --all")
    
    print("\n🎯 Mesh generation complete.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
