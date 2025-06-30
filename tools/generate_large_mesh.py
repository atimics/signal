# tools/generate_large_mesh.py
import sys

def generate_large_mesh(filename, num_vertices):
    """Generates a large .cobj file with the specified number of vertices."""
    with open(filename, 'w') as f:
        f.write("# Large mesh generated for testing\n")
        # Write vertices
        for i in range(num_vertices):
            x = i / 100.0
            f.write(f"v {x} {x} {x}\n")
        
        # Write a single normal and tex_coord to be shared
        f.write("vn 0.0 1.0 0.0\n")
        f.write("vt 0.0 0.0\n")

        # Write faces
        # Create triangles using a sliding window of vertices
        for i in range(1, num_vertices - 1):
            f.write(f"f {i}/1/1 {i+1}/1/1 {i+2}/1/1\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python generate_large_mesh.py <output_filename> <num_vertices>")
        sys.exit(1)
    
    filename = sys.argv[1]
    num_vertices = int(sys.argv[2])
    generate_large_mesh(filename, num_vertices)
    print(f"Generated {filename} with {num_vertices} vertices.")

