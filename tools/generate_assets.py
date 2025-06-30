#!/usr/bin/env python3
"""
CGame Asset Generator - Top-Level Script

This script serves as the main entry point for procedurally generating all
source assets for the CGame engine. It calls the main function of the
mesh_generator module.
"""

import sys
from pathlib import Path

# Add the tools directory to the Python path to allow importing the generator
sys.path.append(str(Path(__file__).parent))

from mesh_generator import main as mesh_generator_main

if __name__ == "__main__":
    # We can add more sophisticated argument parsing here later if needed,
    # for now, we just pass all arguments through to the generator.
    sys.exit(mesh_generator_main())
