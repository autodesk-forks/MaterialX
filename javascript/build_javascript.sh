#!/bin/bash

# This script builds MaterialX JavaScript on Unix-like systems (macOS, Linux).
# The final command starts a local server, allowing you to run the MaterialX Web Viewer 
# locally by entering 'http://localhost:8080' in the search bar of your browser.

set -e  # Exit on any error

echo "---------------------"
echo "Setup Emscripten"
echo "---------------------"

# Edit the following paths to match your local locations for the Emscripten and MaterialX projects.
# You can override these by setting environment variables before running this script:
# export EMSDK_LOCATION=/path/to/emsdk
# export MATERIALX_LOCATION=/path/to/MaterialX

EMSDK_LOCATION=${EMSDK_LOCATION:-${EMSDK:-"$HOME/emsdk"}}
# Set MATERIALX_LOCATION to the root of the project (parent of javascript folder)
MATERIALX_LOCATION=${MATERIALX_LOCATION:-"$(cd "$(dirname "$0")/.." && pwd)"}

echo "Using EMSDK_LOCATION: $EMSDK_LOCATION"
echo "Using MATERIALX_LOCATION: $MATERIALX_LOCATION"

# Check if emsdk exists
if [ ! -d "$EMSDK_LOCATION" ]; then
    echo "Error: Emscripten SDK not found at $EMSDK_LOCATION"
    echo "Please install Emscripten SDK or set EMSDK_LOCATION environment variable"
    echo "To install: git clone https://github.com/emscripten-core/emsdk.git"
fi

cd "$EMSDK_LOCATION"
./emsdk install 2.0.21
./emsdk activate 2.0.21

echo "---------------------"
echo "Build MaterialX With JavaScript"
echo "---------------------"

cd "$MATERIALX_LOCATION"

# Create build directory if it doesn't exist
mkdir -p javascript/build

# Configure and build with CMake
cmake -S . -B javascript/build -DMATERIALX_BUILD_JS=ON -DMATERIALX_EMSDK_PATH=$EMSDK_LOCATION
cmake --build javascript/build --target install --config Release --parallel 2 

echo "---------------------"
echo "Run JavaScript Tests"
echo "---------------------"

cd javascript/MaterialXTest

# Check if npm is available
if ! command -v npm &> /dev/null; then
    echo "Error: npm is not installed or not in PATH"
    echo "Please install Node.js and npm"
    exit 1
fi

npm install
npm run test
npm run test:browser

echo "---------------------"
echo "Run Interactive Viewer"
echo "---------------------"

cd ../MaterialXView
npm install
npm run build
echo "Starting the MaterialX Web Viewer..."
echo "Open http://localhost:8080 in your browser to view the MaterialX Web Viewer"
npm run start