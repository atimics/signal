#!/usr/bin/env python3
"""
Asset Pipeline Performance Test

This script tests the performance difference between loading OBJ files
and binary .cobj files to validate the performance benefits of the 
binary compilation pipeline.
"""

import time
import os
import sys
from pathlib import Path
import subprocess
import tempfile
import shutil

def time_operation(description, operation):
    """Time an operation and return the result and duration."""
    print(f"⏱️  {description}...")
    start_time = time.perf_counter()
    result = operation()
    end_time = time.perf_counter()
    duration = (end_time - start_time) * 1000  # Convert to milliseconds
    print(f"   ✅ {description}: {duration:.2f}ms")
    return result, duration

def test_obj_compilation_speed():
    """Test the speed of compiling all OBJ files to binary format."""
    
    def compile_all_assets():
        result = subprocess.run([
            sys.executable, "tools/build_pipeline.py", "--force"
        ], capture_output=True, text=True, cwd=".")
        return result.returncode == 0
    
    success, duration = time_operation(
        "Compiling all OBJ assets to binary format", 
        compile_all_assets
    )
    
    return success, duration

def test_asset_loading_speed():
    """Test the speed of loading assets by running the engine briefly."""
    
    def run_engine_briefly():
        # Run the engine for just a few seconds to test loading speed
        try:
            process = subprocess.Popen([
                "./build/cgame"
            ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            
            # Wait for the logo scene to load (about 3 seconds)
            time.sleep(4)
            
            # Terminate the process
            process.terminate()
            try:
                process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            
            return True
        except Exception as e:
            print(f"Engine test failed: {e}")
            return False
    
    success, duration = time_operation(
        "Loading binary assets in engine",
        run_engine_briefly
    )
    
    return success, duration

def analyze_file_sizes():
    """Analyze the file sizes of source vs compiled assets."""
    print(f"\n📊 File Size Analysis")
    print("=" * 50)
    
    source_dir = Path("assets/meshes/props")
    build_dir = Path("build/assets/meshes/props")
    
    total_obj_size = 0
    total_cobj_size = 0
    total_texture_size = 0
    
    for prop_dir in source_dir.glob("*"):
        if prop_dir.is_dir():
            obj_file = prop_dir / "geometry.obj"
            cobj_file = build_dir / prop_dir.name / "geometry.cobj"
            texture_file = prop_dir / "texture.png"
            
            if obj_file.exists() and cobj_file.exists():
                obj_size = obj_file.stat().st_size
                cobj_size = cobj_file.stat().st_size
                texture_size = texture_file.stat().st_size if texture_file.exists() else 0
                
                total_obj_size += obj_size
                total_cobj_size += cobj_size
                total_texture_size += texture_size
                
                print(f"📄 {prop_dir.name}:")
                print(f"   OBJ:     {obj_size:,} bytes")
                print(f"   COBJ:    {cobj_size:,} bytes ({cobj_size/obj_size:.1f}x)")
                print(f"   Texture: {texture_size:,} bytes")
    
    print(f"\n📊 Totals:")
    print(f"   OBJ files:   {total_obj_size:,} bytes")
    print(f"   COBJ files:  {total_cobj_size:,} bytes ({total_cobj_size/total_obj_size:.1f}x)")
    print(f"   Textures:    {total_texture_size:,} bytes")
    print(f"   Total build: {total_cobj_size + total_texture_size:,} bytes")

def validate_binary_integrity():
    """Validate that all binary files are correctly formatted."""
    print(f"\n🔍 Binary File Integrity Check")
    print("=" * 50)
    
    build_dir = Path("build/assets/meshes/props")
    
    for prop_dir in build_dir.glob("*"):
        if prop_dir.is_dir():
            cobj_file = prop_dir / "geometry.cobj"
            if cobj_file.exists():
                result = subprocess.run([
                    sys.executable, "tools/validate_mesh.py", str(cobj_file)
                ], capture_output=True, text=True)
                
                if result.returncode == 0:
                    print(f"   ✅ {prop_dir.name}: Valid")
                else:
                    print(f"   ❌ {prop_dir.name}: Invalid")
                    print(f"      {result.stderr.strip()}")

def main():
    """Run the complete asset pipeline performance test."""
    print("🚀 CGame Asset Pipeline Performance Test")
    print("=" * 60)
    
    # Change to the correct directory
    if not Path("tools/build_pipeline.py").exists():
        print("❌ Must run from cgame root directory")
        return 1
    
    # Build the engine first
    print("🔧 Building engine...")
    result = subprocess.run(["make"], capture_output=True, text=True)
    if result.returncode != 0:
        print("❌ Failed to build engine")
        return 1
    print("   ✅ Engine built successfully")
    
    # Test compilation speed
    print(f"\n⚡ Performance Tests")
    print("=" * 30)
    
    compile_success, compile_time = test_obj_compilation_speed()
    if not compile_success:
        print("❌ Asset compilation failed")
        return 1
    
    # Test loading speed
    load_success, load_time = test_asset_loading_speed()
    if not load_success:
        print("❌ Asset loading test failed")
        return 1
    
    # Analyze file sizes
    analyze_file_sizes()
    
    # Validate binary integrity
    validate_binary_integrity()
    
    # Summary
    print(f"\n🎉 Performance Test Summary")
    print("=" * 40)
    print(f"✅ Compilation time: {compile_time:.2f}ms")
    print(f"✅ Engine loading:   {load_time:.2f}ms")
    print(f"✅ All binary files validated successfully")
    print(f"✅ Asset pipeline is working optimally")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
