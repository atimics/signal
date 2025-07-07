#!/usr/bin/env python3
"""
SIGNAL Asset Generator - DEPRECATED

⚠️  DEPRECATION NOTICE ⚠️
This script is deprecated in favor of the clean asset pipeline.
Please use tools/clean_asset_pipeline.py instead.

NEW USAGE:
  python3 tools/clean_asset_pipeline.py --all
  python3 tools/clean_asset_pipeline.py --mesh [mesh_name]
  python3 tools/clean_asset_pipeline.py --list

The clean pipeline provides:
- Proper UV layout SVG generation
- Artist-friendly texture workflow
- Consistent material handling
- Better performance and maintainability
"""

import sys
import subprocess
from pathlib import Path

def main():
    print("⚠️  DEPRECATION WARNING ⚠️")
    print("This script (generate_assets.py) is deprecated.")
    print("Please use the clean asset pipeline instead:")
    print()
    print("  python3 tools/clean_asset_pipeline.py --all")
    print()
    print("Redirecting to clean asset pipeline...")
    print()
    
    # Redirect to clean asset pipeline
    try:
        result = subprocess.run([
            sys.executable, 
            str(Path(__file__).parent / "clean_asset_pipeline.py"),
            "--all"
        ], check=True)
        return result.returncode
    except subprocess.CalledProcessError as e:
        print(f"❌ Clean asset pipeline failed: {e}")
        return e.returncode

if __name__ == "__main__":
    sys.exit(main())
