#!/usr/bin/env python3
"""
CGame Asset Pipeline - Clean Architecture

PIPELINE STAGES:
1. Mesh Generation: Create geometry with proper UV coordinates
2. UV Layout Generation: Create SVG template from UV mapping  
3. Texture Creation: Apply colors to UV layout and convert to PNG
4. Build Compilation: Convert OBJ to binary format

This replaces the messy existing pipeline with a clean, systematic approach.
"""

import argparse
import json
import math
import random
import subprocess
import sys
from pathlib import Path
from typing import List, Tuple, Dict, Any

import numpy as np
from PIL import Image, ImageDraw

def generate_uv_layout_svg(mesh_name: str, vertices: List[List[float]], 
                          faces: List[List[int]], uvs: List[List[float]], 
                          output_path: Path, tags: List[str]) -> None:
    """
    Generate an SVG UV layout template from mesh UV coordinates.
    This creates the template that can be colored and converted to PNG.
    """
    # Load material color scheme based on tags
    material_defs = load_material_definitions()
    color_scheme = get_color_scheme_for_tags(tags, material_defs)
    
    # SVG canvas size
    canvas_size = 1024
    
    # Create SVG content
    svg_content = f'''<svg width="{canvas_size}" height="{canvas_size}" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <linearGradient id="primary_gradient" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="{color_scheme['primary']}" />
      <stop offset="100%" stop-color="{color_scheme['secondary']}" />
    </linearGradient>
    <linearGradient id="secondary_gradient" x1="0%" y1="100%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="{color_scheme['tertiary']}" />
      <stop offset="100%" stop-color="{color_scheme['accent']}" />
    </linearGradient>
  </defs>
  <rect width="100%" height="100%" fill="#111"/>
'''
    
    # Convert UV coordinates to SVG coordinates and create polygons for each face
    gradient_cycle = ["url(#primary_gradient)", "url(#secondary_gradient)"]
    
    for i, face in enumerate(faces):
        if len(face) < 3:
            continue
            
        # Get UV coordinates for this face (convert from 0-1 to SVG coordinates)
        face_uvs = []
        for vertex_idx in face:
            if vertex_idx < len(uvs):
                u, v = uvs[vertex_idx]
                # Convert UV (0-1) to SVG coordinates (0-canvas_size)
                # Flip V coordinate because SVG Y increases downward
                svg_x = u * canvas_size
                svg_y = (1.0 - v) * canvas_size
                face_uvs.append(f"{svg_x:.2f},{svg_y:.2f}")
        
        if len(face_uvs) >= 3:
            points_str = " ".join(face_uvs)
            gradient = gradient_cycle[i % len(gradient_cycle)]
            svg_content += f'  <polygon points="{points_str}" fill="{gradient}" stroke="#000" stroke-width="1.5" opacity="0.8"/>\n'
    
    # Add metadata text
    svg_content += f'''  <text x="10" y="30" fill="white" font-family="Arial" font-size="20" font-weight="bold">{mesh_name.replace('_', ' ').title()} UV Layout</text>
  <text x="10" y="50" fill="#fff" font-family="Arial" font-size="12">This is the UV layout for the mesh. Edit this file to apply textures.</text>
</svg>
'''
    
    # Write SVG file
    with open(output_path, 'w') as f:
        f.write(svg_content)
    
    print(f"   📐 Generated UV layout SVG: {output_path}")

def convert_svg_to_png(svg_path: Path, png_path: Path, size: int = 1024) -> None:
    """Convert SVG UV layout to PNG texture."""
    try:
        import cairosvg
        cairosvg.svg2png(url=str(svg_path), write_to=str(png_path), 
                        output_width=size, output_height=size)
        print(f"   🖼️  Converted SVG to PNG: {png_path}")
    except ImportError:
        # Fallback to rsvg-convert
        try:
            subprocess.run([
                'rsvg-convert', '-w', str(size), '-h', str(size), 
                str(svg_path), '-o', str(png_path)
            ], check=True)
            print(f"   🖼️  Converted SVG to PNG using rsvg-convert: {png_path}")
        except (subprocess.CalledProcessError, FileNotFoundError):
            print(f"   ❌ SVG conversion failed - need cairosvg or rsvg-convert")
            return False
    return True

def load_material_definitions() -> Dict[str, Any]:
    """Load material definitions from assets/material_definitions.json"""
    material_file = Path("assets/material_definitions.json")
    if material_file.exists():
        with open(material_file, 'r') as f:
            return json.load(f)
    return {}

def get_color_scheme_for_tags(tags: List[str], material_defs: Dict[str, Any]) -> Dict[str, str]:
    """Get the best color scheme based on material tags."""
    definitions = material_defs.get('definitions', {})
    
    # Find best matching material definition
    best_match = None
    highest_priority = -1
    
    for tag in tags:
        if tag in definitions:
            priority = definitions[tag].get('priority', 0)
            if priority > highest_priority:
                highest_priority = priority
                best_match = definitions[tag]
    
    # Return colors or defaults
    if best_match and 'colors' in best_match:
        return best_match['colors']
    
    # Default color schemes based on common tags
    defaults = {
        'ship': {'primary': '#4A90E2', 'secondary': '#2C5AA0', 'tertiary': '#7BB3F0', 'accent': '#1E3A8A'},
        'building': {'primary': '#AAB7B7', 'secondary': '#2979AF', 'tertiary': '#D1DCE0', 'accent': '#1565C0'},
        'celestial': {'primary': '#FFDF33', 'secondary': '#CC8400', 'tertiary': '#FFF066', 'accent': '#B8860B'},
        'reference': {'primary': '#FFD700', 'secondary': '#FFA500', 'tertiary': '#FFDF33', 'accent': '#B8860B'},
    }
    
    for tag in tags:
        if tag in defaults:
            return defaults[tag]
    
    # Ultimate fallback
    return {'primary': '#FF6B35', 'secondary': '#4A90E2', 'tertiary': '#F1C40F', 'accent': '#9B59B6'}

# Mesh generation functions (cleaned up)
def generate_control_tower():
    """Generate a detailed control tower with proper UV mapping."""
    vertices = []
    faces = []
    uvs = []
    
    # Base cylinder (8 sides)
    base_radius, base_height = 1.0, 2.0
    sides = 8
    
    # Base center points
    vertices.extend([[0, 0, 0], [0, base_height, 0]])  # Bottom and top center
    uvs.extend([[0.5, 0.0], [0.5, 0.2]])
    
    # Base ring vertices
    for i in range(sides):
        angle = 2 * math.pi * i / sides
        x, z = base_radius * math.cos(angle), base_radius * math.sin(angle)
        vertices.extend([[x, 0, z], [x, base_height, z]])
        u = (i / sides)
        uvs.extend([[u, 0.0], [u, 0.2]])
    
    # Base faces
    for i in range(sides):
        i_next = (i + 1) % sides
        v1, v2 = 2 + i * 2, 2 + i_next * 2
        v3, v4 = v1 + 1, v2 + 1
        
        faces.extend([
            [0, v2, v1],           # Bottom triangle
            [1, v3, v4],           # Top triangle  
            [v1, v2, v4, v3]       # Side quad
        ])
    
    # Deck platform
    deck_radius, deck_height = 1.5, 0.3
    deck_y_start = base_height
    deck_base_start_idx = len(vertices)
    
    for i in range(sides):
        angle = 2 * math.pi * i / sides
        x, z = deck_radius * math.cos(angle), deck_radius * math.sin(angle)
        vertices.extend([[x, deck_y_start, z], [x, deck_y_start + deck_height, z]])
        u = (i / sides)
        uvs.extend([[u, 0.3], [u, 0.4]])
    
    # Deck faces
    for i in range(sides):
        i_next = (i + 1) % sides
        v1 = deck_base_start_idx + i * 2
        v2 = deck_base_start_idx + i_next * 2
        v3, v4 = v1 + 1, v2 + 1
        faces.extend([
            [deck_base_start_idx, v2, v1],    # Bottom
            [deck_base_start_idx + 1, v3, v4], # Top
            [v1, v2, v4, v3]                   # Side
        ])
    
    # Spire on top
    spire_base_radius, spire_height = 0.2, 3.0
    spire_y_start = deck_y_start + deck_height
    spire_base_idx = len(vertices)
    
    for i in range(4):
        angle = 2 * math.pi * i / 4
        x, z = spire_base_radius * math.cos(angle), spire_base_radius * math.sin(angle)
        vertices.append([x, spire_y_start, z])
        uvs.append([0.7 + 0.1 * math.cos(angle), 0.7 + 0.1 * math.sin(angle)])
    
    vertices.append([0, spire_y_start + spire_height, 0])
    uvs.append([0.7, 0.9])
    spire_top_idx = len(vertices) - 1
    
    # Spire faces
    for i in range(4):
        i_next = (i + 1) % 4
        faces.extend([
            [spire_base_idx + i, spire_base_idx + i_next, spire_top_idx]
        ])
    
    return vertices, faces, uvs

def generate_wedge_ship_mk2():
    """Generate an advanced wedge ship with proper UV mapping."""
    vertices = [
        [0.0, 0.0, 2.0],      # Nose
        [-1.0, 0.4, -1.5],    # Top left wing
        [1.0, 0.4, -1.5],     # Top right wing  
        [-1.0, -0.4, -1.5],   # Bottom left wing
        [1.0, -0.4, -1.5],    # Bottom right wing
        [-0.4, -0.2, -2.0],   # Left engine
        [0.4, -0.2, -2.0],    # Right engine
        [0.0, 0.1, -1.0],     # Center top
        [0.0, -0.1, -1.0]     # Center bottom
    ]
    
    uvs = [
        [0.5, 1.0],   # Nose
        [0.0, 0.7],   # Top left wing
        [1.0, 0.7],   # Top right wing
        [0.0, 0.3],   # Bottom left wing  
        [1.0, 0.3],   # Bottom right wing
        [0.2, 0.0],   # Left engine
        [0.8, 0.0],   # Right engine
        [0.5, 0.6],   # Center top
        [0.5, 0.4]    # Center bottom
    ]
    
    faces = [
        [0, 2, 1],        # Top nose triangle
        [0, 3, 4],        # Bottom nose triangle  
        [1, 2, 7, 8],     # Center section
        [1, 8, 3],        # Left side
        [2, 4, 8],        # Right side
        [3, 8, 5],        # Left engine connection
        [4, 6, 8],        # Right engine connection
        [5, 6, 8, 3]      # Rear section
    ]
    
    return vertices, faces, uvs

def generate_sun():
    """Generate a low-poly sphere for the sun with proper UV mapping."""
    radius = 3.0
    latitudes = 6
    longitudes = 8
    vertices = []
    uvs = []
    faces = []
    
    # Generate vertices
    for lat in range(latitudes + 1):
        theta = math.pi * lat / latitudes
        for lon in range(longitudes):
            phi = 2 * math.pi * lon / longitudes
            
            x = radius * math.sin(theta) * math.cos(phi)
            y = radius * math.cos(theta)
            z = radius * math.sin(theta) * math.sin(phi)
            
            vertices.append([x, y, z])
            
            u = lon / longitudes
            v = lat / latitudes
            uvs.append([u, v])
    
    # Generate faces
    for lat in range(latitudes):
        for lon in range(longitudes):
            current = lat * longitudes + lon
            next_lon = lat * longitudes + (lon + 1) % longitudes
            next_lat = (lat + 1) * longitudes + lon
            next_both = (lat + 1) * longitudes + (lon + 1) % longitudes
            
            if lat < latitudes:
                faces.append([current, next_lat, next_both])
                faces.append([current, next_both, next_lon])
    
    return vertices, faces, uvs

def generate_wedge_ship():
    """Generate a simple wedge ship with proper UV mapping."""
    vertices = [
        [0.0, 0.0, 1.5],      # Nose
        [-0.8, 0.3, -1.0],    # Top left wing
        [0.8, 0.3, -1.0],     # Top right wing
        [-0.8, -0.3, -1.0],   # Bottom left wing
        [0.8, -0.3, -1.0],    # Bottom right wing
        [-0.3, -0.1, -1.5],   # Left engine
        [0.3, -0.1, -1.5]     # Right engine
    ]
    
    uvs = [
        [0.5, 1.0],   # Nose
        [0.0, 0.5],   # Top left wing
        [1.0, 0.5],   # Top right wing
        [0.0, 0.2],   # Bottom left wing
        [1.0, 0.2],   # Bottom right wing
        [0.25, 0.0],  # Left engine
        [0.75, 0.0]   # Right engine
    ]
    
    faces = [
        [0, 2, 1],        # Top triangle
        [0, 3, 4],        # Bottom triangle
        [0, 1, 3],        # Left side
        [0, 4, 2],        # Right side
        [1, 2, 4, 3],     # Body quad
        [3, 5, 6, 4]      # Engine section
    ]
    
    return vertices, faces, uvs

def generate_landing_pad():
    """Generate a hexagonal landing pad with proper UV mapping."""
    vertices = []
    faces = []
    uvs = []
    
    radius = 3.0
    height = 0.2
    sides = 6
    
    # Center points
    vertices.extend([[0, 0, 0], [0, height, 0]])
    uvs.extend([[0.5, 0.5], [0.5, 0.5]])
    
    # Ring vertices
    for i in range(sides):
        angle = 2 * math.pi * i / sides
        x, z = radius * math.cos(angle), radius * math.sin(angle)
        vertices.extend([[x, 0, z], [x, height, z]])
        
        u = 0.5 + 0.5 * math.cos(angle)
        v = 0.5 + 0.5 * math.sin(angle)
        uvs.extend([[u, v], [u, v]])
    
    # Generate faces
    for i in range(sides):
        i_next = (i + 1) % sides
        bottom_curr, top_curr = 2 + i * 2, 3 + i * 2
        bottom_next, top_next = 2 + i_next * 2, 3 + i_next * 2
        
        faces.extend([
            [0, bottom_curr, bottom_next],           # Bottom triangle
            [1, top_next, top_curr],                 # Top triangle
            [bottom_curr, top_curr, top_next, bottom_next]  # Side quad
        ])
    
    return vertices, faces, uvs

def generate_planet_surface():
    """Generate a large planet surface with proper UV mapping."""
    size = 50.0
    vertices = [
        [-size, 0, -size], [size, 0, -size], 
        [size, 0, size], [-size, 0, size]
    ]
    
    uvs = [
        [0, 0], [10, 0], [10, 10], [0, 10]  # Tiled UVs
    ]
    
    faces = [
        [0, 1, 2], [0, 2, 3]  # Two triangles
    ]
    
    return vertices, faces, uvs

def generate_logo_cube():
    """Generate a reference cube with proper UV mapping."""
    vertices = [
        # Front face
        [-1.0, -1.0,  1.0], [ 1.0, -1.0,  1.0], [ 1.0,  1.0,  1.0], [-1.0,  1.0,  1.0],
        # Back face  
        [-1.0, -1.0, -1.0], [-1.0,  1.0, -1.0], [ 1.0,  1.0, -1.0], [ 1.0, -1.0, -1.0],
        # Top face
        [-1.0,  1.0, -1.0], [-1.0,  1.0,  1.0], [ 1.0,  1.0,  1.0], [ 1.0,  1.0, -1.0],
        # Bottom face
        [-1.0, -1.0, -1.0], [ 1.0, -1.0, -1.0], [ 1.0, -1.0,  1.0], [-1.0, -1.0,  1.0],
        # Right face
        [ 1.0, -1.0, -1.0], [ 1.0,  1.0, -1.0], [ 1.0,  1.0,  1.0], [ 1.0, -1.0,  1.0],
        # Left face
        [-1.0, -1.0, -1.0], [-1.0, -1.0,  1.0], [-1.0,  1.0,  1.0], [-1.0,  1.0, -1.0]
    ]
    
    uvs = [
        # Front face
        [0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0],
        # Back face
        [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0],
        # Top face
        [0.0, 1.0], [0.0, 0.0], [1.0, 0.0], [1.0, 1.0],
        # Bottom face
        [1.0, 1.0], [0.0, 1.0], [0.0, 0.0], [1.0, 0.0],
        # Right face
        [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0],
        # Left face
        [0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]
    ]
    
    faces = [
        [0, 1, 2, 3],    # Front
        [4, 5, 6, 7],    # Back
        [8, 9, 10, 11],  # Top
        [12, 13, 14, 15], # Bottom
        [16, 17, 18, 19], # Right
        [20, 21, 22, 23]  # Left
    ]
    
    return vertices, faces, uvs

# Main pipeline functions
MESH_GENERATORS = {
    "control_tower": generate_control_tower,
    "wedge_ship_mk2": generate_wedge_ship_mk2,
    "wedge_ship": generate_wedge_ship,
    "sun": generate_sun,
    "landing_pad": generate_landing_pad,
    "planet_surface": generate_planet_surface,
    "logo_cube": generate_logo_cube,
}

MESH_METADATA = {
    "control_tower": {"description": "A control tower for spaceport environments.", "tags": ["building", "structure"]},
    "wedge_ship_mk2": {"description": "An improved wedge-shaped spacecraft.", "tags": ["ship", "vehicle", "player"]},
    "wedge_ship": {"description": "A standard wedge-shaped spacecraft.", "tags": ["ship", "vehicle"]},
    "sun": {"description": "A low-poly sphere representing a star.", "tags": ["celestial", "star"]},
    "landing_pad": {"description": "A spaceport landing platform.", "tags": ["platform", "structure"]},
    "planet_surface": {"description": "A large planetary surface.", "tags": ["terrain", "surface"]},
    "logo_cube": {"description": "Gold standard reference cube for testing.", "tags": ["reference", "test"]},
}

def write_obj_file(filepath: Path, vertices: List[List[float]], 
                  uvs: List[List[float]], faces: List[List[int]]) -> None:
    """Write vertices, UVs, and faces to an OBJ file with proper normals."""
    # Calculate normals
    normals = np.zeros((len(vertices), 3), dtype=np.float32)
    for face in faces:
        if len(face) >= 3:
            face_verts = np.array([vertices[i] for i in face[:3]])
            if len(face_verts) >= 3:
                v1, v2, v3 = face_verts[0], face_verts[1], face_verts[2]
                normal = np.cross(v2 - v1, v3 - v1)
                norm_length = np.linalg.norm(normal)
                if norm_length > 0:
                    normal /= norm_length
                    for i in face:
                        if i < len(normals):
                            normals[i] += normal
    
    # Normalize accumulated normals
    for i in range(len(normals)):
        norm = np.linalg.norm(normals[i])
        if norm > 0:
            normals[i] /= norm
    
    # Write OBJ file
    with open(filepath, 'w') as f:
        f.write("# Generated by CGame Clean Asset Pipeline\n")
        
        # Write vertices
        for v in vertices:
            f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        
        # Write UVs
        for vt in uvs:
            f.write(f"vt {vt[0]:.6f} {vt[1]:.6f}\n")
        
        # Write normals
        for vn in normals:
            f.write(f"vn {vn[0]:.6f} {vn[1]:.6f} {vn[2]:.6f}\n")
        
        f.write("usemtl default\n")
        
        # Write faces
        for face in faces:
            f.write("f")
            for i in face:
                f.write(f" {i+1}/{i+1}/{i+1}")
            f.write("\n")

def write_mtl_file(filepath: Path, material_name: str, texture_filename: str) -> None:
    """Write a simple MTL file."""
    with open(filepath, 'w') as f:
        f.write(f"newmtl {material_name}\n")
        f.write("Ka 1.0 1.0 1.0\n")
        f.write("Kd 1.0 1.0 1.0\n")
        f.write("Ks 0.5 0.5 0.5\n")
        f.write("Ns 10.0\n")
        f.write(f"map_Kd {texture_filename}\n")

def create_mesh_metadata(name: str, description: str, tags: List[str]) -> Dict[str, Any]:
    """Create standardized metadata for generated meshes."""
    return {
        "name": name,
        "geometry": "geometry.obj",
        "tags": tags,
        "description": description,
        "material": "material.mtl",
        "texture": "texture.png"
    }

def generate_mesh_asset(mesh_name: str, source_dir: Path) -> bool:
    """Generate complete mesh asset with all files."""
    if mesh_name not in MESH_GENERATORS:
        print(f"❌ Unknown mesh type: {mesh_name}")
        return False
    
    print(f"🔧 Generating mesh asset: {mesh_name}")
    
    # Create directories
    mesh_dir = source_dir / "props" / mesh_name
    mesh_dir.mkdir(parents=True, exist_ok=True)
    
    # Get mesh data
    generator_func = MESH_GENERATORS[mesh_name]
    metadata_info = MESH_METADATA[mesh_name]
    
    # Generate geometry
    vertices, faces, uvs = generator_func()
    
    # 1. Write OBJ file
    obj_path = mesh_dir / "geometry.obj"
    write_obj_file(obj_path, vertices, uvs, faces)
    print(f"   📐 Generated OBJ: {obj_path}")
    
    # 2. Generate UV layout SVG
    svg_path = mesh_dir / "texture.svg"
    generate_uv_layout_svg(mesh_name, vertices, faces, uvs, svg_path, metadata_info["tags"])
    
    # 3. Convert SVG to PNG
    png_path = mesh_dir / "texture.png"
    if convert_svg_to_png(svg_path, png_path):
        print(f"   ✅ Generated texture: {png_path}")
    else:
        print(f"   ⚠️  Failed to generate texture: {png_path}")
        return False
    
    # 4. Write MTL file
    mtl_path = mesh_dir / "material.mtl"
    write_mtl_file(mtl_path, mesh_name, "texture.png")
    print(f"   📄 Generated MTL: {mtl_path}")
    
    # 5. Write metadata
    metadata = create_mesh_metadata(mesh_name, metadata_info["description"], metadata_info["tags"])
    metadata_path = mesh_dir / "metadata.json"
    with open(metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)
    print(f"   📋 Generated metadata: {metadata_path}")
    
    return True

def main():
    """Main entry point for the clean asset pipeline."""
    parser = argparse.ArgumentParser(description="CGame Clean Asset Pipeline")
    parser.add_argument("--source_dir", default="assets/meshes", help="Source asset directory")
    parser.add_argument("--mesh", help="Generate specific mesh")
    parser.add_argument("--all", action="store_true", help="Generate all meshes")
    parser.add_argument("--list", action="store_true", help="List available meshes")
    
    args = parser.parse_args()
    
    if args.list:
        print("Available meshes:")
        for mesh_name in MESH_GENERATORS.keys():
            info = MESH_METADATA[mesh_name]
            print(f"  {mesh_name}: {info['description']}")
        return 0
    
    source_dir = Path(args.source_dir)
    
    if args.mesh:
        success = generate_mesh_asset(args.mesh, source_dir)
        return 0 if success else 1
    elif args.all:
        print("🚀 Generating all mesh assets...")
        all_success = True
        for mesh_name in MESH_GENERATORS.keys():
            success = generate_mesh_asset(mesh_name, source_dir)
            all_success = all_success and success
        
        if all_success:
            print("✅ All mesh assets generated successfully!")
        else:
            print("❌ Some mesh assets failed to generate")
        return 0 if all_success else 1
    else:
        parser.print_help()
        return 1

if __name__ == "__main__":
    sys.exit(main())
