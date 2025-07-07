#!/usr/bin/env python3
"""
Generate a perfect reference cube mesh for testing the asset pipeline.
This cube will serve as our "gold standard" for validating mesh correctness.
"""

import os
import math

def generate_reference_cube_obj(output_path):
    """Generate a perfect cube OBJ file with correct normals, UVs, and winding."""
    
    # Create a 2x2x2 cube centered at origin (same as the procedural one)
    vertices = [
        # Front face (z = 1)
        [-1, -1,  1],  # 0
        [ 1, -1,  1],  # 1
        [ 1,  1,  1],  # 2
        [-1,  1,  1],  # 3
        
        # Back face (z = -1)
        [-1, -1, -1],  # 4
        [ 1, -1, -1],  # 5
        [ 1,  1, -1],  # 6
        [-1,  1, -1],  # 7
    ]
    
    # Face normals (outward pointing)
    normals = [
        [ 0,  0,  1],  # Front
        [ 0,  0, -1],  # Back
        [-1,  0,  0],  # Left
        [ 1,  0,  0],  # Right
        [ 0,  1,  0],  # Top
        [ 0, -1,  0],  # Bottom
    ]
    
    # UV coordinates for cube mapping
    uvs = [
        [0, 0], [1, 0], [1, 1], [0, 1]  # Standard quad UVs
    ]
    
    # Faces with counter-clockwise winding (outward facing)
    faces = [
        # Front face (normal: 0, 0, 1)
        [(0, 0, 0), (1, 1, 0), (2, 2, 0)],
        [(0, 0, 0), (2, 2, 0), (3, 3, 0)],
        
        # Back face (normal: 0, 0, -1)  
        [(5, 0, 1), (4, 1, 1), (7, 2, 1)],
        [(5, 0, 1), (7, 2, 1), (6, 3, 1)],
        
        # Left face (normal: -1, 0, 0)
        [(4, 0, 2), (0, 1, 2), (3, 2, 2)],
        [(4, 0, 2), (3, 2, 2), (7, 3, 2)],
        
        # Right face (normal: 1, 0, 0)
        [(1, 0, 3), (5, 1, 3), (6, 2, 3)],
        [(1, 0, 3), (6, 2, 3), (2, 3, 3)],
        
        # Top face (normal: 0, 1, 0)
        [(3, 0, 4), (2, 1, 4), (6, 2, 4)],
        [(3, 0, 4), (6, 2, 4), (7, 3, 4)],
        
        # Bottom face (normal: 0, -1, 0)
        [(4, 0, 5), (5, 1, 5), (1, 2, 5)],
        [(4, 0, 5), (1, 2, 5), (0, 3, 5)],
    ]
    
    # Write OBJ file
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    with open(output_path, 'w') as f:
        f.write("# Reference Logo Cube - Gold Standard Mesh\n")
        f.write("# Generated for SIGNAL Asset Pipeline Testing\n")
        f.write("# Perfect geometry with correct normals and UVs\n\n")
        
        # Write vertices
        f.write("# Vertices\n")
        for v in vertices:
            f.write(f"v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}\n")
        f.write("\n")
        
        # Write texture coordinates
        f.write("# Texture coordinates\n")
        for uv in uvs:
            f.write(f"vt {uv[0]:.6f} {uv[1]:.6f}\n")
        f.write("\n")
        
        # Write normals
        f.write("# Normals\n")
        for n in normals:
            f.write(f"vn {n[0]:.6f} {n[1]:.6f} {n[2]:.6f}\n")
        f.write("\n")
        
        # Write faces
        f.write("# Faces (counter-clockwise winding)\n")
        for face in faces:
            face_str = "f"
            for v_idx, uv_idx, n_idx in face:
                face_str += f" {v_idx+1}/{uv_idx+1}/{n_idx+1}"
            f.write(face_str + "\n")
    
    print(f"✅ Generated reference cube: {output_path}")
    print(f"   - 8 vertices, 6 normals, 4 UV coordinates")
    print(f"   - 12 triangular faces with correct winding")
    print(f"   - Ready for asset pipeline compilation")

def generate_reference_material(output_path):
    """Generate a material file for the reference cube."""
    
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    with open(output_path, 'w') as f:
        f.write("# Reference Logo Cube Material\n")
        f.write("newmtl logo_cube_material\n")
        f.write("Ka 0.2 0.2 0.2\n")      # Ambient
        f.write("Kd 1.0 1.0 1.0\n")      # Diffuse (white)
        f.write("Ks 0.5 0.5 0.5\n")      # Specular
        f.write("Ns 32.0\n")             # Shininess
        f.write("map_Kd texture.png\n")   # Diffuse texture
    
    print(f"✅ Generated reference material: {output_path}")

def generate_reference_metadata(output_path):
    """Generate metadata for the reference cube."""
    
    import json
    
    metadata = {
        "name": "Logo Cube",
        "description": "Reference logo cube for asset pipeline testing",
        "category": "test_assets",
        "version": "1.0",
        "geometry": "geometry.obj",
        "material": "material.mtl",
        "texture": "texture.png",
        "tags": ["reference", "cube", "logo", "test"],
        "created_by": "SIGNAL Asset Generator",
        "validation": {
            "expected_vertices": 8,
            "expected_faces": 12,
            "expected_normals": 6,
            "expected_uvs": 4,
            "aabb_min": [-1.0, -1.0, -1.0],
            "aabb_max": [1.0, 1.0, 1.0]
        }
    }
    
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    with open(output_path, 'w') as f:
        json.dump(metadata, f, indent=2)
    
    print(f"✅ Generated reference metadata: {output_path}")

def main():
    # Create the reference cube asset
    asset_dir = "assets/meshes/props/logo_cube"
    
    generate_reference_cube_obj(f"{asset_dir}/geometry.obj")
    generate_reference_material(f"{asset_dir}/material.mtl") 
    generate_reference_metadata(f"{asset_dir}/metadata.json")
    
    # Copy the logo texture
    import shutil
    if os.path.exists("logo.png"):
        shutil.copy("logo.png", f"{asset_dir}/texture.png")
        print(f"✅ Copied logo texture to {asset_dir}/texture.png")
    else:
        print(f"⚠️  Logo texture not found - cube will use default texture")
    
    print("\n🎯 Reference cube asset created!")
    print("   Next steps:")
    print("   1. Run 'make assets' to compile the cube")
    print("   2. Create a logo_test scene")
    print("   3. Test end-to-end pipeline with screenshots")

if __name__ == "__main__":
    main()
