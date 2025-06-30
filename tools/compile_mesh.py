# tools/compile_mesh.py

import struct
import numpy as np
import os

def calculate_tangents_bitangents(positions, normals, uvs, indices):
    """
    Calculates tangents and bitangents for a given mesh.
    This is a simplified implementation. For production, a more robust
    method like MikkTSpace would be preferable.
    """
    tangents = np.zeros_like(positions)
    bitangents = np.zeros_like(positions)

    for i in range(0, len(indices), 3):
        i1, i2, i3 = indices[i], indices[i+1], indices[i+2]

        p1, p2, p3 = positions[i1], positions[i2], positions[i3]
        uv1, uv2, uv3 = uvs[i1], uvs[i2], uvs[i3]

        edge1 = p2 - p1
        edge2 = p3 - p1
        delta_uv1 = uv2 - uv1
        delta_uv2 = uv3 - uv1

        f = 1.0 / (delta_uv1[0] * delta_uv2[1] - delta_uv2[0] * delta_uv1[1])

        tangent = f * (delta_uv2[1] * edge1 - delta_uv1[1] * edge2)
        bitangent = f * (-delta_uv2[0] * edge1 + delta_uv1[0] * edge2)

        # Accumulate tangents and bitangents for shared vertices
        tangents[i1] += tangent
        tangents[i2] += tangent
        tangents[i3] += tangent
        bitangents[i1] += bitangent
        bitangents[i2] += bitangent
        bitangents[i3] += bitangent

    # Gram-Schmidt orthogonalize and normalize
    for i in range(len(positions)):
        n = normals[i]
        t = tangents[i]
        
        # Project t onto n
        proj_t_n = np.dot(t, n) * n
        
        # Subtract the projection to get an orthogonal tangent
        t_ortho = t - proj_t_n
        if np.linalg.norm(t_ortho) > 1e-6:
            tangents[i] = t_ortho / np.linalg.norm(t_ortho)
        else:
            # If tangent is zero or parallel, create an arbitrary one
            # This is a fallback and might not be perfect
            if abs(n[0]) > abs(n[1]):
                inv_len = 1.0 / np.sqrt(n[0] * n[0] + n[2] * n[2])
                tangents[i] = np.array([-n[2] * inv_len, 0.0, n[0] * inv_len])
            else:
                inv_len = 1.0 / np.sqrt(n[1] * n[1] + n[2] * n[2])
                tangents[i] = np.array([0.0, n[2] * inv_len, -n[1] * inv_len])


    return tangents

def compile_mesh(source_path, output_path):
    """
    Compiles a .obj file into a binary .cobj format.
    """
    print(f"📦 Compiling {source_path} -> {output_path}")

    # --- 1. Parse the .obj file ---
    temp_positions = []
    temp_normals = []
    temp_uvs = []
    
    face_lines = []

    with open(source_path, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if not parts:
                continue
            if parts[0] == 'v':
                temp_positions.append(list(map(float, parts[1:4])))
            elif parts[0] == 'vn':
                temp_normals.append(list(map(float, parts[1:4])))
            elif parts[0] == 'vt':
                temp_uvs.append(list(map(float, parts[1:3])))
            elif parts[0] == 'f':
                face_lines.append(parts[1:])

    if not temp_positions or not face_lines:
        print(f"❌ ERROR: No vertices or faces found in {source_path}")
        return False

    # --- 2. Process faces and create final vertex/index lists ---
    final_vertices = []
    final_indices = []
    vertex_map = {}

    for face in face_lines:
        if len(face) != 3:
            # Simple triangulation for quads, could be improved
            if len(face) == 4:
                face = [face[0], face[1], face[2], face[0], face[2], face[3]]
            else:
                print(f"⚠️ WARNING: Non-triangular face with {len(face)} vertices found. Skipping.")
                continue
        
        for i in range(0, len(face), 3):
            tri_face = face[i:i+3]
            for vertex_str in tri_face:
                if vertex_str in vertex_map:
                    final_indices.append(vertex_map[vertex_str])
                else:
                    new_index = len(final_vertices)
                    vertex_map[vertex_str] = new_index
                    final_indices.append(new_index)

                    v_idx, vt_idx, vn_idx = [int(x) - 1 if x else -1 for x in (vertex_str.split('/') + [''] * 3)[:3]]

                    pos = temp_positions[v_idx] if v_idx != -1 else [0,0,0]
                    norm = temp_normals[vn_idx] if vn_idx != -1 else [0,1,0]
                    uv = temp_uvs[vt_idx] if vt_idx != -1 else [0,0]
                    
                    final_vertices.append({
                        'pos': pos,
                        'norm': norm,
                        'uv': uv
                    })

    # --- 3. Convert to NumPy arrays for calculation ---
    positions = np.array([v['pos'] for v in final_vertices], dtype=np.float32)
    normals = np.array([v['norm'] for v in final_vertices], dtype=np.float32)
    uvs = np.array([v['uv'] for v in final_vertices], dtype=np.float32)
    indices = np.array(final_indices, dtype=np.uint32)

    # --- 4. Calculate Tangents and AABB ---
    tangents = calculate_tangents_bitangents(positions, normals, uvs, indices)
    aabb_min = np.min(positions, axis=0)
    aabb_max = np.max(positions, axis=0)

    # --- 5. Pack data into binary format ---
    # Header format: 4s I I I 3f 3f 8I
    # magic, version, vertex_count, index_count, aabb_min, aabb_max, reserved
    header_format = '<4sIIIffffffIIIIIIII'
    header = struct.pack(
        '<4sIIIffffffIIIIIIII',
        b'CGMF',          # Magic number
        1,               # Version
        len(final_vertices),
        len(indices),
        aabb_min[0], aabb_min[1], aabb_min[2],
        aabb_max[0], aabb_max[1], aabb_max[2],
        0, 0, 0, 0, 0, 0, 0, 0 # Reserved
    )
    
    # Vertex format: 3f (pos) + 3f (norm) + 3f (tan) + 2f (uv) + 2f (padding) = 13 floats
    vertex_data = bytearray()
    for i in range(len(final_vertices)):
        # Pad to 48 bytes (12 floats)
        vertex_pack = struct.pack('<3f3f3f2f1f', 
            positions[i][0], positions[i][1], positions[i][2],
            normals[i][0], normals[i][1], normals[i][2],
            tangents[i][0], tangents[i][1], tangents[i][2],
            uvs[i][0], uvs[i][1],
            0.0 # Padding
        )
        vertex_data.extend(vertex_pack)

    index_data = indices.tobytes()

    # --- 6. Write to output file ---
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'wb') as f:
        f.write(header)
        f.write(vertex_data)
        f.write(index_data)
        
    print(f"✅ Successfully compiled {len(final_vertices)} vertices and {len(indices)} indices.")
    return True

if __name__ == '__main__':
    # Example usage for testing
    import sys
    if len(sys.argv) != 3:
        print("Usage: python compile_mesh.py <source.obj> <output.cobj>")
        sys.exit(1)
    
    source = sys.argv[1]
    output = sys.argv[2]
    
    if not os.path.exists(source):
        print(f"❌ Source file not found: {source}")
        sys.exit(1)
        
    compile_mesh(source, output)
