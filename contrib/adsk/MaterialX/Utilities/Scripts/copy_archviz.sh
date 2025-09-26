#!/bin/bash

# Script to copy contrib/adsk/resources/Materials/TestSuite/adsklib/archviz to resources/Materials/Examples/archviz
# and move textures from the archviz folder to resources/Images

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../../../.." && pwd)"

SOURCE_DIR="$PROJECT_ROOT/contrib/adsk/resources/Materials/TestSuite/adsklib/archviz"
DEST_DIR="$PROJECT_ROOT/resources/Materials/Examples/archviz"
IMAGES_DIR="$PROJECT_ROOT/resources/Images"

echo "Copying archviz materials and reorganizing textures..."
echo "Source: $SOURCE_DIR"
echo "Destination: $DEST_DIR"
echo "Images directory: $IMAGES_DIR"

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory does not exist: $SOURCE_DIR"
    exit 1
fi

# Check if Images directory exists
if [ ! -d "$IMAGES_DIR" ]; then
    echo "Error: Images directory does not exist: $IMAGES_DIR"
    exit 1
fi

# Create Examples directory if it doesn't exist
mkdir -p "$(dirname "$DEST_DIR")"

# Check if destination already exists and warn user
if [ -d "$DEST_DIR" ]; then
    echo "Warning: Destination directory already exists: $DEST_DIR"
    read -p "Do you want to overwrite it? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Operation cancelled."
        exit 0
    fi
    echo "Removing existing destination directory..."
    rm -rf "$DEST_DIR"
fi

# Copy the archviz directory
echo "Copying archviz directory..."
cp -r "$SOURCE_DIR" "$DEST_DIR"

# Verify the copy was successful
if [ ! -d "$DEST_DIR" ]; then
    echo "Error: Copy operation failed!"
    exit 1
fi

echo "Successfully copied archviz to Examples directory!"

# Check if textures folder exists in the copied directory
TEXTURES_DIR="$DEST_DIR/textures"
if [ -d "$TEXTURES_DIR" ]; then
    echo "Moving textures from $TEXTURES_DIR to $IMAGES_DIR..."
    
    # Move all contents of textures folder to Images
    for item in "$TEXTURES_DIR"/*; do
        if [ -e "$item" ]; then
            filename=$(basename "$item")
            echo "Moving $filename to Images..."
            
            # Check if file already exists in Images directory
            if [ -e "$IMAGES_DIR/$filename" ]; then
                echo "Warning: $filename already exists in Images directory"
                read -p "Do you want to overwrite it? (y/N): " -n 1 -r
                echo
                if [[ $REPLY =~ ^[Yy]$ ]]; then
                    mv "$item" "$IMAGES_DIR/"
                    echo "Overwritten $filename in Images directory"
                else
                    echo "Skipped $filename"
                fi
            else
                mv "$item" "$IMAGES_DIR/"
                echo "Moved $filename to Images directory"
            fi
        fi
    done
    
    # Remove the now-empty textures folder
    echo "Removing empty textures folder..."
    rmdir "$TEXTURES_DIR"
    echo "Textures folder removed successfully"
else
    echo "No textures folder found in $DEST_DIR"
fi

echo ""
echo "Operation completed successfully!"
echo "Contents of $DEST_DIR:"
ls -la "$DEST_DIR"
echo ""
echo "Recent additions to $IMAGES_DIR:"
ls -lt "$IMAGES_DIR" | head -10