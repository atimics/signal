#!/usr/bin/env python3
"""
Metadata Validator for CGame Engine

This script validates all `metadata.json` files within the `assets/meshes`
directory against the master `schema.json`. It serves as a crucial tool for
ensuring data consistency and integrity across all game assets.

The script performs the following checks:
1.  **Schema Existence**: Verifies that the `schema.json` file exists.
2.  **JSON Validity**: Ensures that each `metadata.json` file is a well-formed
    JSON document.
3.  **Schema Compliance**: Validates the structure and data types of each
    metadata file against the rules defined in the schema.

It can be run as a standalone script to check all assets and is intended to be
used as part of a pre-commit hook or CI/CD pipeline to prevent invalid
metadata from being introduced into the codebase.
"""
import json
import sys
from pathlib import Path
from jsonschema import validate, ValidationError

def validate_metadata(schema_path, data_path, base_dir):
    """Validates a metadata file against the schema."""
    try:
        with open(schema_path, 'r') as f:
            schema = json.load(f)
        with open(data_path, 'r') as f:
            data = json.load(f)
        
        validate(instance=data, schema=schema)
        print(f"OK: {data_path.relative_to(base_dir)}")
        return True
    except FileNotFoundError as e:
        print(f"ERROR: {e.filename} not found.", file=sys.stderr)
        return False
    except json.JSONDecodeError:
        print(f"ERROR: Invalid JSON in {data_path}", file=sys.stderr)
        return False
    except ValidationError as e:
        print(f"ERROR: Validation failed for {data_path}:", file=sys.stderr)
        print(f"  - {e.message} in {e.json_path}", file=sys.stderr)
        return False

def main():
    """
    Scans the assets/meshes directory and validates all metadata.json files.
    """
    base_dir = Path.cwd()
    schema_file = base_dir / "assets/meshes/schema.json"
    if not schema_file.exists():
        print(f"ERROR: Schema file not found at {schema_file}", file=sys.stderr)
        sys.exit(1)

    asset_dir = base_dir / "assets/meshes"
    all_valid = True
    
    for metadata_file in asset_dir.glob("**/metadata.json"):
        if not validate_metadata(schema_file, metadata_file, asset_dir):
            all_valid = False
            
    if all_valid:
        print("\nAll metadata files are valid.")
    else:
        print("\nSome metadata files failed validation.", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
