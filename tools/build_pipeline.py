#!/usr/bin/env python3
"""
CGame Asset Pipeline - Binary Mesh Compiler

This script finds all source .obj files, compiles them into the binary
.cobj format, and generates the necessary metadata and index files
for the engine to discover and load them.
"""

import argparse
import json
import struct
from pathlib import Path
from datetime import datetime
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
    
    # Load all metadata files and aggregate into a single index
    meshes = []
    source_dir = Path("assets/meshes")
    
    for asset_name in compiled_assets:
        metadata_path = source_dir / "props" / asset_name / "metadata.json"
        if metadata_path.exists():
            try:
                with open(metadata_path, 'r') as f:
                    metadata = json.load(f)
                meshes.append(metadata)
            except json.JSONDecodeError as e:
                print(f"⚠️  Warning: Invalid JSON in {metadata_path}: {e}")
                continue
        else:
            print(f"⚠️  Warning: Missing metadata for {asset_name}")
    
    # Create index structure expected by mesh viewer
    index_data = {
        "meshes": meshes,
        "generated": str(datetime.now().isoformat()),
        "version": "1.0"
    }
    
    with open(index_path, 'w') as f:
        json.dump(index_data, f, indent=4)
    
    # Also copy to source directory for mesh viewer
    source_index_path = Path("assets/meshes/index.json")
    with open(source_index_path, 'w') as f:
        json.dump(index_data, f, indent=4)
    
    print(f"✅ Generated asset index with {len(meshes)} meshes: {index_path}")
    print(f"✅ Copied asset index to source directory: {source_index_path}")

import shutil

def copy_supplementary_files(source_prop_dir, build_prop_dir):
    """Copies .mtl and .png files from source to build."""
    for ext in ["*.mtl", "*.png"]:
        for source_file in source_prop_dir.glob(ext):
            shutil.copy(source_file, build_prop_dir / source_file.name)
            print(f"   Copied {source_file.name} to {build_prop_dir}")

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
        build_prop_dir = build_dir / "props" / asset_name
        output_cobj_file = build_prop_dir / "geometry.cobj"
        
        if not source_obj_file.exists():
            print(f"   ⚠️ WARNING: Skipping {asset_name}, geometry.obj not found.")
            continue

        # Check if compilation is needed
        if not args.force and output_cobj_file.exists() and output_cobj_file.stat().st_mtime > source_obj_file.stat().st_mtime:
            print(f"   Skipping {asset_name} (up-to-date)")
            compiled_assets.append(asset_name) # Still need to add to index
            continue
            
        # Ensure the output directory exists
        build_prop_dir.mkdir(parents=True, exist_ok=True)
        
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
            
            # --- Validate the compiled binary mesh ---
            validator_script = Path(__file__).parent / "validate_mesh.py"
            validate_command = [
                sys.executable,
                str(validator_script),
                str(output_cobj_file)
            ]
            
            try:
                subprocess.run(validate_command, check=True, capture_output=True, text=True)
                print(f"   ✅ Validation passed for {asset_name}")
            except subprocess.CalledProcessError as e:
                print(f"   ❌ Validation failed for {asset_name}:")
                if e.stdout:
                    print(f"      {e.stdout.strip()}")
                if e.stderr:
                    print(f"      {e.stderr.strip()}")
                error_count += 1
                continue  # Skip metadata generation for invalid assets
            
            # --- Generate the corresponding metadata.json in the build directory ---
            generate_and_write_metadata(source_meta_file, build_prop_dir)
            
            # --- Copy supplementary files ---
            copy_supplementary_files(prop_dir, build_prop_dir)
            
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
