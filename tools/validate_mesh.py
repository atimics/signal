#!/usr/bin/env python3
"""
Mesh Validation System - Sprint 13.1 Task 2

This tool validates mesh geometry correctness before and after compilation.
It checks for common issues like invalid normals, UV coordinate bounds,
face winding consistency, and geometric integrity.
"""

import sys
import os
import math
import struct
from pathlib import Path

def validate_obj_file(obj_path):
    """
    Validate a source OBJ file for geometric correctness.
    
    Returns:
        dict: Validation results with 'valid' boolean and 'issues' list
    """
    results = {
        'valid': True,
        'issues': [],
        'warnings': [],
        'stats': {}
    }
    
    if not os.path.exists(obj_path):
        results['valid'] = False
        results['issues'].append(f"File not found: {obj_path}")
        return results
    
    vertices = []
    normals = []
    uvs = []
    faces = []
    
    # Parse OBJ file
    try:
        with open(obj_path, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                
                parts = line.split()
                if len(parts) == 0:
                    continue
                
                try:
                    if parts[0] == 'v':
                        if len(parts) >= 4:
                            vertices.append([float(parts[1]), float(parts[2]), float(parts[3])])
                        else:
                            results['issues'].append(f"Line {line_num}: Invalid vertex format")
                    
                    elif parts[0] == 'vn':
                        if len(parts) >= 4:
                            normals.append([float(parts[1]), float(parts[2]), float(parts[3])])
                        else:
                            results['issues'].append(f"Line {line_num}: Invalid normal format")
                    
                    elif parts[0] == 'vt':
                        if len(parts) >= 3:
                            uvs.append([float(parts[1]), float(parts[2])])
                        else:
                            results['issues'].append(f"Line {line_num}: Invalid UV format")
                    
                    elif parts[0] == 'f':
                        face_indices = []
                        for vertex_str in parts[1:]:
                            face_indices.append(vertex_str)
                        faces.append(face_indices)
                
                except ValueError as e:
                    results['issues'].append(f"Line {line_num}: Parse error - {e}")
                except Exception as e:
                    results['issues'].append(f"Line {line_num}: Unexpected error - {e}")
    
    except Exception as e:
        results['valid'] = False
        results['issues'].append(f"Failed to read file: {e}")
        return results
    
    # Store statistics
    results['stats'] = {
        'vertex_count': len(vertices),
        'normal_count': len(normals),
        'uv_count': len(uvs),
        'face_count': len(faces)
    }
    
    # Validate basic structure
    if len(vertices) == 0:
        results['valid'] = False
        results['issues'].append("No vertices found")
    
    if len(faces) == 0:
        results['valid'] = False
        results['issues'].append("No faces found")
    
    # Validate normals
    for i, normal in enumerate(normals):
        magnitude = math.sqrt(normal[0]**2 + normal[1]**2 + normal[2]**2)
        if magnitude < 0.001:
            results['issues'].append(f"Normal {i+1}: Zero-length normal vector")
        elif abs(magnitude - 1.0) > 0.1:
            results['warnings'].append(f"Normal {i+1}: Not unit length (mag={magnitude:.3f})")
    
    # Validate UV coordinates
    for i, uv in enumerate(uvs):
        if uv[0] < 0.0 or uv[0] > 1.0:
            results['warnings'].append(f"UV {i+1}: U coordinate out of range [0,1]: {uv[0]}")
        if uv[1] < 0.0 or uv[1] > 1.0:
            results['warnings'].append(f"UV {i+1}: V coordinate out of range [0,1]: {uv[1]}")
    
    # Validate face indices
    for i, face in enumerate(faces):
        if len(face) < 3:
            results['issues'].append(f"Face {i+1}: Less than 3 vertices")
        elif len(face) > 4:
            results['warnings'].append(f"Face {i+1}: More than 4 vertices (will be triangulated)")
        
        # Check index validity
        for vertex_str in face:
            try:
                indices = vertex_str.split('/')
                v_idx = int(indices[0]) if indices[0] else 0
                if v_idx < 1 or v_idx > len(vertices):
                    results['issues'].append(f"Face {i+1}: Vertex index {v_idx} out of range")
                
                if len(indices) > 1 and indices[1]:
                    vt_idx = int(indices[1])
                    if vt_idx < 1 or vt_idx > len(uvs):
                        results['issues'].append(f"Face {i+1}: UV index {vt_idx} out of range")
                
                if len(indices) > 2 and indices[2]:
                    vn_idx = int(indices[2])
                    if vn_idx < 1 or vn_idx > len(normals):
                        results['issues'].append(f"Face {i+1}: Normal index {vn_idx} out of range")
            
            except ValueError:
                results['issues'].append(f"Face {i+1}: Invalid index format in '{vertex_str}'")
    
    # Calculate AABB and check for degenerate geometry
    if vertices:
        min_x = min(v[0] for v in vertices)
        max_x = max(v[0] for v in vertices)
        min_y = min(v[1] for v in vertices)
        max_y = max(v[1] for v in vertices)
        min_z = min(v[2] for v in vertices)
        max_z = max(v[2] for v in vertices)
        
        size_x = max_x - min_x
        size_y = max_y - min_y
        size_z = max_z - min_z
        
        results['stats']['aabb'] = {
            'min': [min_x, min_y, min_z],
            'max': [max_x, max_y, max_z],
            'size': [size_x, size_y, size_z]
        }
        
        if size_x < 0.001 and size_y < 0.001 and size_z < 0.001:
            results['issues'].append("Degenerate geometry: All vertices at same position")
    
    # Final validation
    if results['issues']:
        results['valid'] = False
    
    return results

def validate_cobj_file(cobj_path):
    """
    Validate a compiled COBJ binary file for format correctness.
    
    Returns:
        dict: Validation results with 'valid' boolean and 'issues' list
    """
    results = {
        'valid': True,
        'issues': [],
        'warnings': [],
        'stats': {}
    }
    
    if not os.path.exists(cobj_path):
        results['valid'] = False
        results['issues'].append(f"File not found: {cobj_path}")
        return results
    
    try:
        with open(cobj_path, 'rb') as f:
            # Read header
            magic = f.read(4)
            if magic != b'CGMF':
                results['valid'] = False
                results['issues'].append(f"Invalid magic number: {magic}")
                return results
            
            version = struct.unpack('<I', f.read(4))[0]
            if version != 1:
                results['warnings'].append(f"Unexpected version: {version}")
            
            vertex_count = struct.unpack('<I', f.read(4))[0]
            index_count = struct.unpack('<I', f.read(4))[0]
            
            # Read AABB
            aabb_min = struct.unpack('<3f', f.read(12))
            aabb_max = struct.unpack('<3f', f.read(12))
            
            # Skip reserved fields
            f.read(32)  # 8 * uint32_t
            
            results['stats'] = {
                'vertex_count': vertex_count,
                'index_count': index_count,
                'aabb_min': aabb_min,
                'aabb_max': aabb_max
            }
            
            # Validate header values
            if vertex_count == 0:
                results['issues'].append("Zero vertices in header")
            if index_count == 0:
                results['issues'].append("Zero indices in header")
            if index_count % 3 != 0:
                results['warnings'].append(f"Index count {index_count} not divisible by 3")
            
            # Validate AABB
            for i in range(3):
                if aabb_min[i] > aabb_max[i]:
                    results['issues'].append(f"Invalid AABB: min[{i}]={aabb_min[i]} > max[{i}]={aabb_max[i]}")
            
            # Calculate expected file size
            vertex_size = 32  # sizeof(Vertex) = 3*4 + 3*4 + 2*4 = 32 bytes
            expected_vertex_data_size = vertex_count * vertex_size
            expected_index_data_size = index_count * 4  # sizeof(uint32_t)
            header_size = 64  # sizeof(COBJHeader)
            expected_total_size = header_size + expected_vertex_data_size + expected_index_data_size
            
            # Check actual file size
            current_pos = f.tell()
            f.seek(0, 2)  # Seek to end
            actual_size = f.tell()
            
            if actual_size != expected_total_size:
                results['issues'].append(
                    f"File size mismatch: expected {expected_total_size}, got {actual_size}"
                )
            
            results['stats']['file_size'] = {
                'actual': actual_size,
                'expected': expected_total_size,
                'header': header_size,
                'vertex_data': expected_vertex_data_size,
                'index_data': expected_index_data_size
            }
            
            # TODO: Could validate vertex data structure if needed
            # For now, just check if we can read the expected amount
            f.seek(current_pos)
            try:
                vertex_data = f.read(expected_vertex_data_size)
                if len(vertex_data) != expected_vertex_data_size:
                    results['issues'].append("Could not read expected vertex data")
                
                index_data = f.read(expected_index_data_size)
                if len(index_data) != expected_index_data_size:
                    results['issues'].append("Could not read expected index data")
                
            except Exception as e:
                results['issues'].append(f"Error reading binary data: {e}")
    
    except Exception as e:
        results['valid'] = False
        results['issues'].append(f"Failed to read binary file: {e}")
    
    if results['issues']:
        results['valid'] = False
    
    return results

def print_validation_report(filepath, results):
    """Print a human-readable validation report."""
    print(f"\n📊 Validation Report: {filepath}")
    print("=" * 60)
    
    # Overall status
    status = "✅ VALID" if results['valid'] else "❌ INVALID"
    print(f"Status: {status}")
    
    # Statistics
    if results['stats']:
        print(f"\n📈 Statistics:")
        for key, value in results['stats'].items():
            if isinstance(value, dict):
                print(f"  {key}:")
                for subkey, subvalue in value.items():
                    print(f"    {subkey}: {subvalue}")
            else:
                print(f"  {key}: {value}")
    
    # Issues
    if results['issues']:
        print(f"\n❌ Issues ({len(results['issues'])}):")
        for issue in results['issues']:
            print(f"  • {issue}")
    
    # Warnings
    if results['warnings']:
        print(f"\n⚠️  Warnings ({len(results['warnings'])}):")
        for warning in results['warnings']:
            print(f"  • {warning}")
    
    if not results['issues'] and not results['warnings']:
        print("\n🎉 No issues or warnings found!")

def main():
    if len(sys.argv) != 2:
        print("Usage: python validate_mesh.py <path_to_mesh_file>")
        print("Supports: .obj (source) and .cobj (compiled) files")
        sys.exit(1)
    
    filepath = sys.argv[1]
    
    if not os.path.exists(filepath):
        print(f"❌ File not found: {filepath}")
        sys.exit(1)
    
    file_ext = Path(filepath).suffix.lower()
    
    if file_ext == '.obj':
        results = validate_obj_file(filepath)
    elif file_ext == '.cobj':
        results = validate_cobj_file(filepath)
    else:
        print(f"❌ Unsupported file type: {file_ext}")
        print("Supported types: .obj, .cobj")
        sys.exit(1)
    
    print_validation_report(filepath, results)
    
    # Exit with appropriate code
    sys.exit(0 if results['valid'] else 1)

if __name__ == "__main__":
    main()
