#!/usr/bin/env python3
"""
CGame Asset Pipeline - Binary Mesh Compiler

This script finds all source .obj files and compiles them into the binary
.cobj format for use by the engine.
"""

import argparse
import sys
from pathlib import Path
import subprocess

def main():
    parser = argparse.ArgumentParser(description="CGame Asset Pipeline - Binary Compiler")
    
    parser.add_argument("--source_dir", default="assets/meshes", help="Source directory containing .obj files")
    parser.add_argument("--build_dir", default="build/assets/meshes", help="Output directory for .cobj files")
    parser.add_argument("--force", action="store_true", help="Force re-compilation even if output exists")
    
    args = parser.parse_args()
    
    source_dir = Path(args.source_dir)
    build_dir = Path(args.build_dir)
    
    if not source_dir.exists():
        print(f"❌ Source directory not found: {source_dir}")
        return 1
        
    print(f"🚀 CGame Asset Pipeline")
    print(f"   Source: {source_dir}")
    print(f"   Build:  {build_dir}")
    print("-" * 40)
    
    # Find all .obj files in the source directory
    source_files = list(source_dir.glob("**/*.obj"))
    
    if not source_files:
        print("⚠️ No .obj files found to compile.")
        return 0
        
    compiled_count = 0
    skipped_count = 0
    error_count = 0
    
    for source_file in source_files:
        # Determine the output path by mirroring the directory structure
        relative_path = source_file.relative_to(source_dir)
        output_file = build_dir / relative_path.with_suffix(".cobj")
        
        # Check if compilation is needed
        if not args.force and output_file.exists() and output_file.stat().st_mtime > source_file.stat().st_mtime:
            # print(f"   Skipping {relative_path} (up-to-date)")
            skipped_count += 1
            continue
            
        # Ensure the output directory exists
        output_file.parent.mkdir(parents=True, exist_ok=True)
        
        # Compile the file by calling the compile_mesh.py script
        compiler_script = Path(__file__).parent / "compile_mesh.py"
        command = [
            sys.executable, 
            str(compiler_script),
            str(source_file),
            str(output_file)
        ]
        
        try:
            result = subprocess.run(command, check=True, capture_output=True, text=True)
            # print(result.stdout.strip()) # Can be noisy, enable for debug
            compiled_count += 1
        except subprocess.CalledProcessError as e:
            print(f"❌ Failed to compile {source_file}:")
            print(e.stderr)
            error_count += 1

    print("-" * 40)
    print("📊 Compilation Summary:")
    print(f"   Compiled: {compiled_count}")
    print(f"   Skipped:  {skipped_count}")
    print(f"   Errors:   {error_count}")
    
    if error_count > 0:
        print("\n💥 Asset pipeline failed!")
        return 1
    else:
        print("\n🎉 Asset pipeline completed successfully!")
        return 0

if __name__ == "__main__":
    sys.exit(main())
