#!/usr/bin/env python3
"""
CGame Asset Pipeline - Unified Build System

This script combines mesh generation, asset compilation, and validation into a single pipeline.
It can generate procedural meshes, compile existing meshes, and validate all metadata.
"""

import argparse
import json
import sys
from pathlib import Path

# Import our tools
from mesh_generator import main as mesh_generator_main, generate_mesh_with_textures
from mesh_generator import generate_wedge_ship_mk2, generate_control_tower, generate_sun_sphere
import asset_compiler
from asset_compiler import validate_metadata

def validate_all_metadata(source_dir, schema_path):
    """Validate all metadata files in the source directory."""
    print("🔍 Validating all metadata files...")
    
    metadata_files = list(Path(source_dir).glob("**/metadata.json"))
    valid_count = 0
    invalid_count = 0
    
    for metadata_file in metadata_files:
        try:
            with open(metadata_file, 'r') as f:
                metadata = json.load(f)
            
            if validate_metadata(schema_path, metadata):
                print(f"   ✅ {metadata_file.relative_to(Path(source_dir))}")
                valid_count += 1
            else:
                print(f"   ❌ {metadata_file.relative_to(Path(source_dir))}")
                invalid_count += 1
                
        except json.JSONDecodeError as e:
            print(f"   ❌ {metadata_file.relative_to(Path(source_dir))} - Invalid JSON: {e}")
            invalid_count += 1
        except Exception as e:
            print(f"   ❌ {metadata_file.relative_to(Path(source_dir))} - Error: {e}")
            invalid_count += 1
    
    print(f"\n📊 Validation Results: {valid_count} valid, {invalid_count} invalid")
    return invalid_count == 0

def clean_build_directory(build_dir):
    """Clean the build directory."""
    print(f"🧹 Cleaning build directory: {build_dir}")
    
    import shutil
    if Path(build_dir).exists():
        shutil.rmtree(build_dir)
    Path(build_dir).mkdir(parents=True, exist_ok=True)
    
    print("   ✅ Build directory cleaned")

def generate_missing_meshes(source_dir, build_dir, schema_path):
    """Generate any missing procedural meshes."""
    print("🔧 Checking for missing procedural meshes...")
    
    # Define expected procedural meshes
    procedural_meshes = {
        "wedge_ship_mk2": generate_wedge_ship_mk2,
        "control_tower": generate_control_tower, 
        "sun": generate_sun_sphere,
    }
    
    generated_count = 0
    
    for mesh_name, generator in procedural_meshes.items():
        mesh_dir = Path(source_dir) / "props" / mesh_name
        mesh_file = mesh_dir / "geometry.mesh"
        
        if not mesh_file.exists():
            print(f"   🔧 Generating missing mesh: {mesh_name}")
            if generate_mesh_with_textures(mesh_name, generator, source_dir, build_dir, schema_path):
                generated_count += 1
        else:
            print(f"   ✓ {mesh_name} already exists")
    
    if generated_count > 0:
        print(f"   ✅ Generated {generated_count} missing meshes")
    else:
        print("   ✅ All procedural meshes exist")
    
    return generated_count

def main():
    parser = argparse.ArgumentParser(description="CGame Asset Pipeline - Complete build system")
    
    # Directories
    parser.add_argument("--source_dir", default="assets/meshes", help="Source directory")
    parser.add_argument("--build_dir", default="build/assets/meshes", help="Build directory")
    
    # Pipeline stages
    parser.add_argument("--validate", action="store_true", help="Validate all metadata")
    parser.add_argument("--generate", action="store_true", help="Generate missing procedural meshes")
    parser.add_argument("--compile", action="store_true", help="Compile all meshes")
    parser.add_argument("--clean", action="store_true", help="Clean build directory first")
    parser.add_argument("--all", action="store_true", help="Run complete pipeline (clean, generate, validate, compile)")
    
    # Asset compiler options
    parser.add_argument("--overwrite", action="store_true", help="Overwrite existing build files")
    
    # Specific operations
    parser.add_argument("--mesh", help="Generate specific mesh only")
    
    args = parser.parse_args()
    
    source_dir = Path(args.source_dir)
    build_dir = Path(args.build_dir)
    schema_path = source_dir / "schema.json"
    
    if not source_dir.exists():
        print(f"❌ Source directory not found: {source_dir}")
        return 1
        
    if not schema_path.exists():
        print(f"❌ Schema file not found: {schema_path}")
        return 1
    
    print(f"🚀 CGame Asset Pipeline")
    print(f"   Source: {source_dir}")
    print(f"   Build:  {build_dir}")
    print()
    
    # Run pipeline stages
    success = True
    
    try:
        # Stage 1: Clean (if requested)
        if args.clean or args.all:
            clean_build_directory(build_dir)
        
        # Stage 2: Generate missing meshes (if requested)
        if args.generate or args.all:
            generate_missing_meshes(source_dir, build_dir, schema_path)
        
        # Stage 3: Generate specific mesh (if requested)
        if args.mesh:
            generators = {
                "wedge_ship_mk2": generate_wedge_ship_mk2,
                "control_tower": generate_control_tower,
                "sun": generate_sun_sphere,
            }
            if args.mesh in generators:
                generate_and_compile_mesh(args.mesh, generators[args.mesh], source_dir, build_dir, schema_path)
            else:
                print(f"❌ Unknown mesh: {args.mesh}")
                return 1
        
        # Stage 4: Validate metadata (if requested)
        if args.validate or args.all:
            if not validate_all_metadata(source_dir, schema_path):
                print("❌ Metadata validation failed")
                success = False
        
        # Stage 5: Compile assets (if requested)
        if args.compile or args.all:
            print("🔨 Compiling all assets...")
            
            # Set up asset compiler arguments
            asset_compiler.args = argparse.Namespace()
            asset_compiler.args.source_dir = str(source_dir)
            asset_compiler.args.overwrite = args.overwrite or args.all
            
            # Run asset compiler
            original_argv = sys.argv
            try:
                sys.argv = [
                    "asset_compiler.py",
                    "--source_dir", str(source_dir),
                    "--build_dir", str(build_dir)
                ]
                if args.overwrite or args.all:
                    sys.argv.append("--overwrite")
                
                asset_compiler.main()
                print("   ✅ Asset compilation complete")
                
            finally:
                sys.argv = original_argv
    
    except Exception as e:
        print(f"❌ Pipeline error: {e}")
        success = False
    
    if success:
        print("\n🎉 Asset pipeline completed successfully!")
        return 0
    else:
        print("\n💥 Asset pipeline failed!")
        return 1

if __name__ == "__main__":
    sys.exit(main())
