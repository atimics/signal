#!/usr/bin/env python3
import argparse
import math
import random
import re
import subprocess
import sys
from pathlib import Path

import numpy as np
from scipy.spatial import Delaunay

def normalize(v):
    """Normalize a numpy vector."""
    norm = np.linalg.norm(v)
    if norm == 0:
        return v
    return v / norm

def get_random_color_hex():
    """Returns a random bright color in hex format."""
    return f"#{random.randint(128, 255):02X}{random.randint(128, 255):02X}{random.randint(128, 255):02X}"

from scipy.spatial import Delaunay

def generate_platonic_solid(solid_type):
    """Generates vertices and faces for a given platonic solid."""
    if solid_type == 'tetrahedron':
        vertices = [
            (1, 1, 1), (-1, -1, 1), (-1, 1, -1), (1, -1, -1)
        ]
        faces = [
            (0, 1, 2), (0, 3, 1), (0, 2, 3), (1, 3, 2)
        ]
    elif solid_type == 'cube':
        v = 1
        vertices = [
            (-v, -v, -v), (v, -v, -v), (v, v, -v), (-v, v, -v),
            (-v, -v, v), (v, -v, v), (v, v, v), (-v, v, v)
        ]
        faces = [
            (0, 1, 2, 3), (4, 5, 1, 0), (7, 6, 5, 4),
            (3, 2, 6, 7), (1, 5, 6, 2), (4, 0, 3, 7)
        ]
    elif solid_type == 'octahedron':
        v = 1
        vertices = [
            (v, 0, 0), (-v, 0, 0), (0, v, 0), (0, -v, 0), (0, 0, v), (0, 0, -v)
        ]
        faces = [
            (0, 2, 4), (0, 4, 3), (0, 3, 5), (0, 5, 2),
            (1, 2, 5), (1, 5, 3), (1, 3, 4), (1, 4, 2)
        ]
    elif solid_type == 'dodecahedron':
        p = (1 + math.sqrt(5)) / 2
        vertices = [
            (-1, -1, -1), (-1, -1, 1), (-1, 1, -1), (-1, 1, 1),
            (1, -1, -1), (1, -1, 1), (1, 1, -1), (1, 1, 1),
            (0, -p, -1/p), (0, -p, 1/p), (0, p, -1/p), (0, p, 1/p),
            (-p, -1/p, 0), (-p, 1/p, 0), (p, -1/p, 0), (p, 1/p, 0),
            (-1/p, 0, -p), (1/p, 0, -p), (-1/p, 0, p), (1/p, 0, p)
        ]
        
        # Use Delaunay triangulation to find the convex hull faces
        points = np.array(vertices)
        hull = Delaunay(points)
        
        # Extract the faces (simplices)
        faces = hull.convex_hull
    elif solid_type == 'icosahedron':
        t = (1.0 + math.sqrt(5.0)) / 2.0
        vertices = [
            (-1, t, 0), (1, t, 0), (-1, -t, 0), (1, -t, 0),
            (0, -1, t), (0, 1, t), (0, -1, -t), (0, 1, -t),
            (t, 0, -1), (t, 0, 1), (-t, 0, -1), (-t, 0, 1)
        ]
        faces = [
            (0, 11, 5), (0, 5, 1), (0, 1, 7), (0, 7, 10), (0, 10, 11),
            (1, 5, 9), (5, 11, 4), (11, 10, 2), (10, 7, 6), (7, 1, 8),
            (3, 9, 4), (3, 4, 2), (3, 2, 6), (3, 6, 8), (3, 8, 9),
            (4, 9, 5), (2, 4, 11), (6, 2, 10), (8, 6, 7), (9, 8, 1)
        ]
    else:
        raise ValueError(f"Unknown solid type: {solid_type}")

    vertices = [normalize(np.array(v)) for v in vertices]
    return vertices, faces

def triangulate(faces):
    """Triangulates polygonal faces."""
    tri_faces = []
    for face in faces:
        if len(face) == 3:
            tri_faces.append(face)
        elif len(face) == 4:
            tri_faces.append((face[0], face[1], face[2]))
            tri_faces.append((face[0], face[2], face[3]))
        elif len(face) == 5: # Dodecahedron
            tri_faces.append((face[0], face[1], face[2]))
            tri_faces.append((face[0], face[2], face[3]))
            tri_faces.append((face[0], face[3], face[4]))
    return tri_faces

def generate_uvs_and_svg(faces, svg_path, svg_width=1024, svg_height=1024):
    """Generates UV coordinates and an SVG texture."""
    uv_coords = []
    uv_faces = []
    num_faces = len(faces)
    cols = math.ceil(math.sqrt(num_faces))
    rows = math.ceil(num_faces / cols)
    col_width, row_height = 1.0 / cols, 1.0 / rows
    uv_template = [(0.0, 0.0), (0.5, 1.0), (1.0, 0.0)]

    for i, face in enumerate(faces):
        row, col = i // cols, i % cols
        cell_origin_u, cell_origin_v = col * col_width, row * row_height
        face_uvs = []
        for u_offset, v_offset in uv_template:
            u = cell_origin_u + u_offset * col_width
            v = cell_origin_v + v_offset * row_height
            uv_coords.append((u, v))
            face_uvs.append(len(uv_coords))
        uv_faces.append(face_uvs)

    with open(svg_path, 'w') as f:
        color1, color2, color3 = get_random_color_hex(), get_random_color_hex(), get_random_color_hex()
        f.write(f'<svg width="{svg_width}" height="{svg_height}" xmlns="http://www.w3.org/2000/svg">\n')
        f.write('  <defs>\n')
        f.write(f'    <radialGradient id="randomGradient" cx="50%" cy="50%" r="50%">\n')
        f.write(f'      <stop offset="0%" stop-color="{color1}" />\n')
        f.write(f'      <stop offset="50%" stop-color="{color2}" />\n')
        f.write(f'      <stop offset="100%" stop-color="{color3}" />\n')
        f.write('    </radialGradient>\n')
        f.write('  </defs>\n')
        f.write(f'  <rect width="{svg_width}" height="{svg_height}" fill="url(#randomGradient)"/>\n')

        # Draw triangle outlines for visualization
        for uv_face in uv_faces:
            points = []
            for uv_index in uv_face:
                u, v = uv_coords[uv_index - 1]
                # Scale UVs to SVG dimensions (and flip V for correct image orientation)
                x = u * svg_width
                y = (1.0 - v) * svg_height
                points.append(f"{x},{y}")
            
            f.write(f'  <polygon points="{" ".join(points)}" fill="none" stroke="rgba(0,0,0,0.2)" stroke-width="2"/>\n')

        f.write('</svg>\n')

    return uv_coords, uv_faces

def convert_svg_to_png(svg_path, png_path):
    """Converts SVG to PNG using macOS's `sips` command."""
    try:
        subprocess.run(
            ["sips", "-s", "format", "png", str(svg_path), "--out", str(png_path)],
            check=True, capture_output=True, text=True
        )
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"Error during SVG to PNG conversion: {e}", file=sys.stderr)
        if isinstance(e, subprocess.CalledProcessError):
            print(f"SIPS Stderr: {e.stderr}", file=sys.stderr)
        sys.exit(1)

def write_mtl_file(mtl_path, material_name, texture_filename):
    """Writes an emissive .mtl file."""
    with open(mtl_path, 'w') as f:
        f.write(f"newmtl {material_name}\n")
        f.write("Ke 1.0 1.0 1.0\n")
        f.write(f"map_Ke {texture_filename}\n")
        f.write("Kd 1.0 1.0 1.0\n")
        f.write(f"map_Kd {texture_filename}\n")

def write_obj_file(obj_path, vertices, uvs, faces, uv_faces, mtl_filename, material_name):
    """Writes the final .obj file with vertices, UVs, and normals."""
    vertices_np = np.array(vertices)
    face_normals = []
    corrected_faces = []

    for v_indices, uv_indices in zip(faces, uv_faces):
        v0, v1, v2 = vertices_np[v_indices[0]], vertices_np[v_indices[1]], vertices_np[v_indices[2]]
        face_normal = np.cross(v1 - v0, v2 - v0)
        if np.dot(face_normal, v0) < 0:
            face_normal = -face_normal
            corrected_faces.append(((v_indices[0], v_indices[2], v_indices[1]), (uv_indices[0], uv_indices[2], uv_indices[1])))
        else:
            corrected_faces.append(((v_indices[0], v_indices[1], v_indices[2]), (uv_indices[0], uv_indices[1], uv_indices[2])))
        face_normals.append(normalize(face_normal))

    vertex_normals = np.zeros_like(vertices_np)
    for i, (v_indices, _) in enumerate(corrected_faces):
        for v_idx in v_indices:
            vertex_normals[v_idx] += face_normals[i]
    
    for i in range(len(vertex_normals)):
        vertex_normals[i] = normalize(vertex_normals[i])

    with open(obj_path, 'w') as f:
        f.write(f"# Generated by mesh_generator\n")
        f.write(f"mtllib {mtl_filename}\n")
        f.write(f"usemtl {material_name}\n\n")
        for v in vertices:
            f.write(f"v {' '.join(f'{c:.6f}' for c in v)}\n")
        f.write("\n")
        for u, v_coord in uvs:
            f.write(f"vt {u:.6f} {v_coord:.6f}\n")
        f.write("\n")
        for vn in vertex_normals:
            f.write(f"vn {' '.join(f'{c:.6f}' for c in vn)}\n")
        f.write("\n")
        for (v_indices, vt_indices) in corrected_faces:
            f_parts = [f"{v_indices[i]+1}/{vt_indices[i]}/{v_indices[i]+1}" for i in range(3)]
            f.write(f"f {' '.join(f_parts)}\n")

def process_obj_file(input_path, output_dir):
    """Processes a given .obj file to generate texture and material."""
    base_name = Path(input_path).stem
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    obj_path = output_dir / f"{base_name}.obj"
    mtl_path = output_dir / f"{base_name}.mtl"
    svg_path = output_dir / f"{base_name}_texture.svg"
    png_path = output_dir / f"{base_name}_texture.png"
    material_name = base_name.capitalize()

    vertices = []
    faces = []
    with open(input_path, 'r') as f:
        for line in f:
            line = line.strip().split('#')[0]
            if not line:
                continue
            if line.startswith('v '):
                vertices.append(list(map(float, line.split()[1:])))
            elif line.startswith('f '):
                # Simple face parsing, assuming triangles
                face = [int(re.split('//|/', v)[0]) - 1 for v in line.split()[1:]]
                faces.append(tuple(face))
    
    faces = triangulate(faces)
    
    print(f"Processing '{input_path}'...")
    print("1. Generating UV map and SVG texture...")
    uv_coords, uv_faces = generate_uvs_and_svg(faces, svg_path)

    print(f"2. Converting SVG to PNG ('{svg_path}' -> '{png_path}')...")
    convert_svg_to_png(svg_path, png_path)

    print(f"3. Writing MTL file ('{mtl_path}')...")
    write_mtl_file(mtl_path, material_name, png_path.name)

    print(f"4. Writing final OBJ file ('{obj_path}')...")
    write_obj_file(obj_path, vertices, uv_coords, faces, uv_faces, mtl_path.name, material_name)
    print(f"\nSuccessfully processed '{base_name}'. Files created in '{output_dir}'.")


def main():
    parser = argparse.ArgumentParser(description="Generate a starter kit of platonic solids or process an existing .obj file.")
    parser.add_argument("--input_obj", help="Path to an existing .obj file to process.")
    parser.add_argument("--output_dir", help="The directory to save the files.", default=".")
    args = parser.parse_args()

    if args.input_obj:
        process_obj_file(args.input_obj, args.output_dir)
    else:
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        
        solids = ['tetrahedron', 'cube', 'octahedron', 'dodecahedron', 'icosahedron']
        for solid_name in solids:
            base_name = solid_name
            obj_path = output_dir / f"{base_name}.obj"
            mtl_path = output_dir / f"{base_name}.mtl"
            svg_path = output_dir / f"{base_name}_texture.svg"
            png_path = output_dir / f"{base_name}_texture.png"
            material_name = base_name.capitalize()

            print(f"--- Generating {solid_name.capitalize()} ---")
            vertices, faces = generate_platonic_solid(solid_name)
            faces = triangulate(faces)

            print("1. Generating UV map and SVG texture...")
            uv_coords, uv_faces = generate_uvs_and_svg(faces, svg_path)

            print(f"2. Converting SVG to PNG...")
            convert_svg_to_png(svg_path, png_path)

            print(f"3. Writing MTL file...")
            write_mtl_file(mtl_path, material_name, png_path.name)

            print(f"4. Writing final OBJ file...")
            write_obj_file(obj_path, vertices, uv_coords, faces, uv_faces, mtl_path.name, material_name)
            print(f"Successfully generated '{base_name}'.\n")

        print("\nPlatonic solids starter kit generated successfully.")

if __name__ == "__main__":
    main()