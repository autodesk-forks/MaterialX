# MaterialX Node.js CLI Application

A command-line interface for MaterialX document processing and manipulation, built on the MaterialX Core WASM module.

## Features

Currently available with MaterialX Core module:

- ✅ **Document Processing**: Create, load, and manipulate MaterialX documents
- ✅ **MaterialX File I/O**: Import and export MaterialX (.mtlx) files
- ✅ **Node Graph Operations**: Access and modify node graph structures  
- ✅ **Material Definitions**: Work with material and node definitions
- ✅ **Document Validation**: Validate MaterialX document structure
- ✅ **XML Processing**: Full XML import/export capabilities

### Shader Generation Status

⚠️ **Shader generation is not available in this Node.js CLI** due to MaterialX GenShader module requiring a web environment for WASM preloaded data. The application currently uses MaterialX Core module which provides full document processing capabilities but not shader generation.

For shader generation, the MaterialX GenShader module would need to be used in a web browser environment.

## Installation

1. **Build MaterialX with JavaScript support**:
   ```bash
   # From MaterialX root directory
   cd javascript/MaterialXTest
   ./build_javascript_unix.sh
   ```

2. **Install dependencies**:
   ```bash
   cd shadergenapp
   npm install
   ```

3. **Copy MaterialX build files**:
   ```bash
   npm run build
   ```

## Usage

### List Available Features
```bash
npm run cli list
```

### Document Operations (Planned)
```bash
# Create and process a basic MaterialX document
npm run cli process --input material.mtlx --output processed.mtlx

# Validate a MaterialX document  
npm run cli validate --input material.mtlx
```

### Development Commands
```bash
# Run tests
npm test

# Clean build files
npm run clean

# Copy MaterialX libraries
npm run build
```

## Project Structure

```
shadergenapp/
├── bin/
│   └── cli.js              # Main CLI entry point
├── src/
│   ├── MaterialXLoader.js  # MaterialX WASM loading and initialization
│   └── ShaderGenerator.js  # Document processing logic (shader gen disabled)
├── lib/                    # MaterialX WASM files (copied from build)
├── test/                   # Test files
└── package.json           # Project configuration
```

## Architecture

- **MaterialXLoader**: Handles MaterialX Core WASM module initialization
- **Document Processing**: Provides MaterialX document manipulation capabilities
- **CLI Interface**: Commander.js-based command-line interface with colorized output

## Current Status

✅ **Working**: MaterialX Core v1.39.4 successfully initializes and provides full document processing APIs

❌ **Not Available**: Shader generation (requires GenShader module + web environment)

## Example Output

```bash
$ npm run cli list

🔄 Initializing MaterialX...
📂 MaterialX library path: /path/to/lib
🔄 Loading MaterialX modules...
📋 Loading JsMaterialXCore...
🔄 Initializing MaterialX WASM...
✅ MaterialX initialized: 1.39.4
✅ MaterialX initialized successfully

📋 Available Shader Generators:
============================================================
⚠️  No shader generators available.
   This is expected when using MaterialX Core module only.
   Core module provides document parsing and manipulation capabilities.

📋 MaterialX Core Features Available:
   ✅ Document creation and parsing
   ✅ MaterialX file loading
   ✅ Node graph manipulation
   ✅ Material definition processing
   ✅ XML import/export

💡 For shader generation, the GenShader module would be needed
   (requires web environment for WASM preloaded data)

Total generators available: 0
```

## Development Notes

This application demonstrates successful MaterialX WASM integration in Node.js using the Core module. While shader generation is not available, it provides a solid foundation for MaterialX document manipulation and analysis tools.

## Future Enhancements

- Document processing commands
- MaterialX file format utilities
- Node graph analysis tools
- Material property extraction  
- Document validation with detailed reporting

## MaterialX Version

- **Version**: 1.39.4
- **Module**: Core (WASM)
- **Environment**: Node.js
- **Build**: JavaScript/WASM bindings