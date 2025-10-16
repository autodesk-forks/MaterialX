#!/usr/bin/env python3

"""
Script to copy ADSK materials to the main MaterialX libraries and resources directories.

This script:
1. Copies contrib/adsk/libraries/adsklib to libraries/adsklib
2. Copies contrib/adsk/resources to resources (merging directories)
3. Updates example_materials.json to include Fusion, Proceduralwood, and Revit materials
"""

import os
import sys
import shutil
import json
from pathlib import Path
from typing import Dict, List, Any


def get_project_root() -> Path:
    """Get the project root directory."""
    script_dir = Path(__file__).parent.absolute()
    return script_dir.parents[2]  # Go up from contrib/utilities/scripts to project root


def copy_directory_with_merge(src: Path, dest: Path, overwrite: bool = False) -> bool:
    """
    Copy a directory, merging with existing directories.
    
    Args:
        src: Source directory path
        dest: Destination directory path
        overwrite: Whether to overwrite existing files without asking
    
    Returns:
        bool: True if successful, False otherwise
    """
    try:
        if not src.exists():
            print(f"Error: Source directory does not exist: {src}")
            return False
            
        print(f"Copying {src.name} to {dest}")
        
        # Create destination parent directory if it doesn't exist
        dest.parent.mkdir(parents=True, exist_ok=True)
        
        # If destination exists and it's a directory, merge contents
        if dest.exists():
            if dest.is_file():
                if not overwrite:
                    response = input(f"File exists at destination: {dest}. Overwrite? (y/N): ")
                    if response.lower() != 'y':
                        print("Skipped.")
                        return True
                dest.unlink()
            elif dest.is_dir():
                print(f"Destination directory exists: {dest}")
                if not overwrite:
                    response = input("Do you want to merge/overwrite contents? (y/N): ")
                    if response.lower() != 'y':
                        print("Skipped.")
                        return True
        
        # Perform the copy
        if dest.exists() and dest.is_dir():
            # Merge directories
            for item in src.rglob('*'):
                if item.is_file():
                    rel_path = item.relative_to(src)
                    dest_file = dest / rel_path
                    dest_file.parent.mkdir(parents=True, exist_ok=True)
                    
                    if dest_file.exists() and not overwrite:
                        response = input(f"File exists: {dest_file}. Overwrite? (y/N): ")
                        if response.lower() != 'y':
                            continue
                    
                    shutil.copy2(item, dest_file)
                    print(f"  Copied: {rel_path}")
        else:
            # Copy entire directory
            shutil.copytree(src, dest, dirs_exist_ok=True)
            
        return True
        
    except Exception as e:
        print(f"Error copying {src} to {dest}: {e}")
        return False


def copy_adsk_libraries(project_root: Path) -> bool:
    """Copy ADSK libraries to the main libraries directory."""
    src_dir = project_root / "contrib" / "adsk" / "libraries" / "adsklib"
    dest_dir = project_root / "libraries" / "adsklib"
    
    print("=" * 60)
    print("Copying ADSK Libraries")
    print("=" * 60)
    
    return copy_directory_with_merge(src_dir, dest_dir)


def copy_adsk_resources(project_root: Path) -> bool:
    """Copy ADSK resources to the main resources directory."""
    src_dir = project_root / "contrib" / "adsk" / "resources"
    dest_dir = project_root / "resources"
    
    print("\n" + "=" * 60)
    print("Copying ADSK Resources")
    print("=" * 60)
    
    success = True
    
    # Copy each subdirectory in resources
    for subdir in ["Images", "Lights", "Materials", "Geometry"]:
        src_subdir = src_dir / subdir
        dest_subdir = dest_dir / subdir
        
        if src_subdir.exists():
            print(f"\nCopying {subdir}...")
            if not copy_directory_with_merge(src_subdir, dest_subdir):
                success = False
        else:
            print(f"Warning: {subdir} not found in source directory")
    
    return success


def update_example_materials_json(project_root: Path) -> bool:
    """Update example_materials.json to include Fusion, Proceduralwood, and Revit."""
    json_file = project_root / "javascript" / "MaterialXView" / "example_materials.json"
    
    print("\n" + "=" * 60)
    print("Updating example_materials.json")
    print("=" * 60)
    
    try:
        # Read existing JSON
        with open(json_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        # Check if materials already exist
        existing_names = {material['name'] for material in data['materials']}
        
        # Define new materials to add
        new_materials = [
            {
                "name": "Fusion",
                "path": "../../resources/Materials/Examples/Fusion",
                "baseURL": "Materials/Examples/Fusion"
            },
            {
                "name": "Proceduralwood",
                "path": "../../resources/Materials/Examples/Proceduralwood",
                "baseURL": "Materials/Examples/Proceduralwood"
            },
            {
                "name": "Revit",
                "path": "../../resources/Materials/Examples/Revit",
                "baseURL": "Materials/Examples/Revit"
            }
        ]
        
        # Add materials that don't already exist
        added_materials = []
        for material in new_materials:
            if material['name'] not in existing_names:
                data['materials'].append(material)
                added_materials.append(material['name'])
                print(f"  Added: {material['name']}")
            else:
                print(f"  Already exists: {material['name']}")
        
        # Write updated JSON back to file
        with open(json_file, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2)
        
        if added_materials:
            print(f"\nSuccessfully added {len(added_materials)} new materials to example_materials.json")
        else:
            print("\nNo new materials were added (all already existed)")
            
        return True
        
    except Exception as e:
        print(f"Error updating example_materials.json: {e}")
        return False


def main():
    """Main function to orchestrate the copying process."""
    print("ADSK Materials Integration Script")
    print("=" * 60)
    
    project_root = get_project_root()
    print(f"Project root: {project_root}")
    
    # Verify we're in the right place
    if not (project_root / "contrib" / "adsk").exists():
        print("Error: Could not find contrib/adsk directory. Make sure you're running this from the MaterialX project.")
        sys.exit(1)
    
    success = True
    
    # Copy ADSK libraries
    if not copy_adsk_libraries(project_root):
        success = False
    
    # Copy ADSK resources
    if not copy_adsk_resources(project_root):
        success = False
    
    # Update example materials JSON
    if not update_example_materials_json(project_root):
        success = False
    
    # Summary
    print("\n" + "=" * 60)
    if success:
        print("✅ All operations completed successfully!")
        print("\nNext steps:")
        print("1. The ADSK library has been copied to libraries/adsklib")
        print("2. ADSK resources have been merged into the resources directory")
        print("3. example_materials.json has been updated with new material categories")
        print("4. You may need to rebuild your project to use the new libraries")
    else:
        print("❌ Some operations failed. Please check the error messages above.")
        sys.exit(1)


if __name__ == "__main__":
    main()
