#!/usr/bin/env python3
"""
CGame Asset Pipeline - Binary Mesh Compiler

This script finds all source .obj files, compiles them into the binary
.cobj format, and generates the necessary metadata and index files
for the engine to discover and load them.
"""

import argparse
import sys
import os
import json
from pathlib import Path
import subprocess

def generate_and_write_metadata(source_metadata_path, build_prop_dir):
    """
    Reads source metadata, adds binary format info, and writes it to the build directory.
    """
    if not source_metadata_path.exists():
        print(f"   ⚠️ WARNING: Source metadata not found at {source_metadata_path}")
        return None

    with open(source_metadata_path, 'r') as f:
        metadata = json.load(f)

    # Add binary format information
    metadata['format_version'] = "1.0"
    metadata['geometry_format'] = "binary_cobj_v1"
    metadata['geometry'] = "geometry.cobj" # Ensure it points to the binary file

    # Write the new metadata to the build directory
    output_metadata_path = build_prop_dir / "metadata.json"
    with open(output_metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)
        
    return metadata

def generate_asset_index(build_dir, compiled_assets):
    """
    Generates the main index.json file listing all compiled assets.
    """
    index_path = build_dir / "index.json"
    
    # The paths in the index must be relative to the index file itself
    metadata_paths = [
        f"props/{asset_name}/metadata.json"
        for asset_name in compiled_assets
    ]
    
    with open(index_path, 'w') as f:
        json.dump(metadata_paths, f, indent=4)
    
    print(f"✅ Generated asset index with {len(metadata_paths)} entries: {index_path}")

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
    
    # Find all source metadata files to determine what to compile
    source_metadata_files = list(source_dir.glob("**/metadata.json"))
    
    if not source_metadata_files:
        print("⚠️ No metadata.json files found in source directory. Nothing to compile.")
        return 0
        
    compiled_assets = []
    error_count = 0
    
    for source_meta_file in source_metadata_files:
        prop_dir = source_meta_file.parent
        asset_name = prop_dir.name
        
        # Determine source and output paths
        source_obj_file = prop_dir / "geometry.obj"
        output_cobj_file = build_dir / "props" / asset_name / "geometry.cobj"
        
        if not source_obj_file.exists():
            print(f"   ⚠️ WARNING: Skipping {asset_name}, geometry.obj not found.")
            continue

        # Check if compilation is needed
        if not args.force and output_cobj_file.exists() and output_cobj_file.stat().st_mtime > source_obj_file.stat().st_mtime:
            print(f"   Skipping {asset_name} (up-to-date)")
            compiled_assets.append(asset_name) # Still need to add to index
            continue
            
        # Ensure the output directory exists
        output_cobj_file.parent.mkdir(parents=True, exist_ok=True)
        
        # --- Compile the .obj to .cobj ---
        compiler_script = Path(__file__).parent / "compile_mesh.py"
        command = [
            sys.executable, 
            str(compiler_script),
            str(source_obj_file),
            str(output_cobj_file)
        ]
        
        try:
            subprocess.run(command, check=True, capture_output=True, text=True)
            
            # --- Generate the corresponding metadata.json in the build directory ---
            generate_and_write_metadata(source_meta_file, output_cobj_file.parent)
            
            compiled_assets.append(asset_name)
            
        except subprocess.CalledProcessError as e:
            print(f"❌ Failed to compile {source_obj_file}:")
            print(e.stderr)
            error_count += 1

    print("-" * 40)
    
    # --- Generate the final asset index ---
    if error_count == 0:
        generate_asset_index(build_dir, compiled_assets)
        print("\n🎉 Asset pipeline completed successfully!")
        return 0
    else:
        print(f"\n💥 Asset pipeline failed with {error_count} error(s).")
        return 1

if __name__ == "__main__":
    sys.exit(main())
