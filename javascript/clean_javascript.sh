#!/bin/bash

# Clean JavaScript build artifacts and dependencies
# This script removes build directories and node_modules

echo "Cleaning JavaScript build artifacts..."

# Remove build directory
if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
fi

# Remove MaterialXTest build directory
if [ -d "MaterialXTest/_build" ]; then
    echo "Removing MaterialXTest/_build directory..."
    rm -rf MaterialXTest/_build
fi

# Remove MaterialXTest node_modules
if [ -d "MaterialXTest/node_modules" ]; then
    echo "Removing MaterialXTest/node_modules directory..."
    rm -rf MaterialXTest/node_modules
fi

# Remove MaterialXView dist directory
if [ -d "MaterialXView/dist" ]; then
    echo "Removing MaterialXView/dist directory..."
    rm -rf MaterialXView/dist
fi

# Remove MaterialXView node_modules
if [ -d "MaterialXView/node_modules" ]; then
    echo "Removing MaterialXView/node_modules directory..."
    rm -rf MaterialXView/node_modules
fi

echo "JavaScript cleanup completed."