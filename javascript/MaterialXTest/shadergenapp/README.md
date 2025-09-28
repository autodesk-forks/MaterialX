# MaterialX Shader Generator CLI

A powerful Node.js command-line application for generating shaders from MaterialX documents. This tool supports multiple shader targets including GLSL, ESSL, MSL, OSL, WGSL, Vulkan SPIR-V, and MDL.

## Features

🎯 **Multi-target Support**: Generate shaders for ESSL, GLSL, MSL, OSL, Vulkan, WGSL, and MDL
🚀 **CLI Interface**: Easy-to-use command-line interface with multiple commands
📚 **Programmatic API**: Can be used as a Node.js module in other applications
💾 **File Output**: Saves generated shaders to organized directory structure
📊 **Detailed Reporting**: Comprehensive statistics and validation
🎨 **Default Materials**: Create standard surface materials for testing
📂 **MaterialX Support**: Load and process existing MaterialX documents

## Installation

```bash
# Navigate to the shadergenapp directory
cd shadergenapp

# Install dependencies
npm install

# Build MaterialX libraries (copy from parent build)
npm run build
```

## CLI Usage

### Generate Shaders

Generate shaders for all available targets using default material:
```bash
npm run cli generate --output ./output
```

Generate shaders from a MaterialX file:
```bash
npm run cli generate --input material.mtlx --output ./output
```

Generate shaders for specific targets only:
```bash
npm run cli generate --targets glsl,essl --output ./output
```

Output shaders to console (for debugging):
```bash
npm run cli generate --console
```

### List Available Generators

```bash
npm run cli list
```

### Validate MaterialX Document

```bash
npm run cli validate material.mtlx
```

### Show MaterialX Information

```bash
npm run cli info
```

## CLI Commands Reference

| Command | Alias | Description |
|---------|-------|-------------|
| `generate` | `gen` | Generate shaders from MaterialX document or default material |
| `list` | `ls` | List available shader generators |
| `validate` | `val` | Validate a MaterialX document |
| `info` | - | Show MaterialX information |

### Generate Command Options

| Option | Description |
|--------|-------------|
| `-i, --input <file>` | Input MaterialX file (.mtlx) |
| `-o, --output <dir>` | Output directory for generated shaders |
| `-t, --targets <targets>` | Comma-separated list of targets |
| `-c, --console` | Output shaders to console |

### Available Targets

- `essl` - OpenGL ES Shading Language
- `glsl` - OpenGL Shading Language  
- `msl` - Metal Shading Language
- `osl` - Open Shading Language
- `vulkan` - Vulkan SPIR-V
- `wgsl` - WebGPU Shading Language
- `mdl` - Material Definition Language

## Programmatic API

```javascript
const { MaterialXShaderGen } = require('./index');

async function generateShaders() {
    // Initialize the generator
    const shaderGen = new MaterialXShaderGen();
    await shaderGen.initialize();
    
    // Create a standard material or load from file
    const doc = shaderGen.createStandardMaterial();
    // or: const doc = shaderGen.loadDocument('material.mtlx');
    
    // Generate shaders
    const results = await shaderGen.generateAll(doc);
    // or: const results = await shaderGen.generateTargets(doc, ['glsl', 'essl']);
    
    // Get statistics
    const stats = shaderGen.getStatistics(results);
    console.log(`Generated ${stats.successful} shaders successfully`);
    
    // Save to files
    await shaderGen.saveResults(results, './output');
    
    // Validate results
    const issues = shaderGen.validateResults(results);
    if (issues.length > 0) {
        console.log('Issues found:', issues);
    }
}

generateShaders().catch(console.error);
```

## Output Structure

When shaders are saved to files, they are organized as follows:

```
output/
├── summary.json              # Overall generation summary
├── essl/                     # ESSL shaders
│   ├── vertex.shader
│   ├── pixel.shader
│   └── info.json
├── glsl/                     # GLSL shaders
│   ├── vertex.shader
│   ├── pixel.shader
│   └── info.json
└── [other targets]/
    ├── vertex.shader         # If available
    ├── pixel.shader          # If available
    └── info.json
```

### File Contents

- **`vertex.shader`** / **`pixel.shader`**: Generated shader source code
- **`info.json`**: Metadata about the generation (target, sizes, timestamps, etc.)
- **`summary.json`**: Complete summary of all generation results

## Example Usage

### Basic Generation
```bash
# Generate all shaders with default material
npm run cli generate --output ./my-shaders

# List what was generated
npm run cli list
```

### Advanced Generation
```bash
# Generate specific targets from MaterialX file
npm run cli generate \
  --input materials/copper.mtlx \
  --targets glsl,essl,wgsl \
  --output ./copper-shaders
```

### Validation and Debugging
```bash
# Validate a MaterialX file first
npm run cli validate materials/copper.mtlx

# Generate with console output for debugging
npm run cli generate \
  --input materials/copper.mtlx \
  --console
```

## Testing

Run the built-in test suite:
```bash
npm test
```

This will:
1. Initialize MaterialX
2. Create a standard surface material
3. Generate shaders for all available targets
4. Validate the results
5. Save output to `test-output/` directory

## Troubleshooting

### MaterialX build files not found
```bash
npm run build
```

### No shader generators available
This usually means MaterialX wasn't built with shader generation support. Check your MaterialX build configuration.

### Permission errors
```bash
chmod +x bin/cli.js
```

### Module loading errors
Ensure you're using Node.js 14+ and all dependencies are installed:
```bash
npm install
```

## Development

### Project Structure

```
shadergenapp/
├── bin/
│   └── cli.js              # CLI entry point
├── src/
│   ├── MaterialXLoader.js  # MaterialX initialization and utilities
│   └── ShaderGenerator.js  # Core shader generation logic
├── lib/                    # MaterialX build files (copied)
├── index.js               # Main programmatic API
├── test.js                # Test script
└── package.json           # Project configuration
```

### Adding New Features

1. **New CLI Commands**: Add to `bin/cli.js`
2. **Core Functionality**: Extend `src/ShaderGenerator.js`
3. **MaterialX Utilities**: Extend `src/MaterialXLoader.js`
4. **API Methods**: Add to `index.js`

### Building MaterialX Libraries

The application requires MaterialX build files. These are copied from the parent build directory:

```bash
# Copy MaterialX build files
npm run build

# Clean build files
npm run clean
```

## API Reference

### MaterialXShaderGen Class

#### Methods

- `initialize()`: Initialize MaterialX and shader generators
- `createStandardMaterial()`: Create a standard surface material for testing
- `loadDocument(filePath)`: Load MaterialX document from file
- `getAvailableTargets()`: Get list of available shader targets
- `generateAll(doc)`: Generate shaders for all available targets
- `generateTargets(doc, targets)`: Generate shaders for specific targets
- `saveResults(results, outputDir)`: Save generation results to files
- `getStatistics(results)`: Get statistics from generation results
- `validateResults(results)`: Validate generation results
- `getMaterialX()`: Get raw MaterialX instance
- `isInitialized()`: Check if generator is initialized

### Result Object Format

```javascript
{
  target: 'glsl',              // Target name
  success: true,               // Generation success
  elementPath: '/Default',     // MaterialX element path
  vertexShader: '...',         // Vertex shader code (if available)
  pixelShader: '...',          // Pixel shader code (if available)
  error: '...',                // Error message (if failed)
  generatorInfo: {             // Generator metadata
    name: 'GlslShaderGenerator',
    description: 'OpenGL Shading Language'
  }
}
```

## License

This project follows the same license as MaterialX (Apache 2.0).

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Update documentation
6. Submit a pull request

## Support

For issues and questions:
1. Check the troubleshooting section
2. Verify MaterialX build files are present
3. Check that Node.js version is 14+
4. Review the MaterialX documentation