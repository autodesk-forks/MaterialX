#!/bin/bash

# Script to copy contrib/adsk/libraries/adsklib to libraries/adsklib
# This integrates the Autodesk contribution library into the main libraries directory

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../../../.." && pwd)"

SOURCE_DIR="$PROJECT_ROOT/contrib/adsk/libraries/adsklib"
DEST_DIR="$PROJECT_ROOT/libraries/adsklib"

echo "Copying Autodesk library from contrib to main libraries..."
echo "Source: $SOURCE_DIR"
echo "Destination: $DEST_DIR"

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory does not exist: $SOURCE_DIR"
    exit 1
fi

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

# Copy the directory
echo "Copying adsklib directory..."
cp -r "$SOURCE_DIR" "$DEST_DIR"

# Verify the copy was successful
if [ -d "$DEST_DIR" ]; then
    echo "Successfully copied adsklib to libraries directory!"
    echo "Contents of $DEST_DIR:"
    ls -la "$DEST_DIR"
else
    echo "Error: Copy operation failed!"
    exit 1
fi

echo "Copy operation completed successfully."