/**
 * MaterialX Loader Module
 * Handles MaterialX initialization and provides utilities for shader generation
 */

const path = require('path');
const fs = require('fs');

class MaterialXLoader {
    /**
     * Create a new MaterialX loader
     * @param {string} libPath - Path to MaterialX library files (defaults to '../lib')
     */
    constructor(libPath = null) {
        this.materialX = null;
        this.initialized = false;
        this.libPath = libPath || path.join(__dirname, '..', 'lib');
        
        console.log(`📂 MaterialX library path: ${this.libPath}`);
    }

    /**
     * Initialize MaterialX WASM module
     * @returns {Promise<Object>} MaterialX instance
     */
    async initialize() {
        if (this.materialX) {
            return this.materialX;
        }

        try {
            console.log('🔄 Loading MaterialX modules...');
            
            // In Node.js, use Core module only (GenShader requires web environment)
            let MaterialXModule = null;
            
            const coreFile = path.join(this.libPath, 'JsMaterialXCore.js');
            if (fs.existsSync(coreFile)) {
                console.log('📋 Loading JsMaterialXCore...');
                MaterialXModule = require(coreFile);
            } else {
                throw new Error('MaterialX Core module not found');
            }

            if (typeof MaterialXModule !== 'function') {
                throw new Error(`MaterialX module is not a function: ${typeof MaterialXModule}`);
            }

            console.log('🔄 Initializing MaterialX WASM...');
            
            // Initialize with timeout
            const mx = await Promise.race([
                MaterialXModule(),
                new Promise((_, reject) => 
                    setTimeout(() => reject(new Error('Initialization timeout')), 30000)
                )
            ]);
            
            if (!mx) {
                throw new Error('MaterialX initialization returned undefined');
            }

            console.log(`✅ MaterialX initialized: ${mx.getVersionString ? mx.getVersionString() : 'Unknown version'}`);
            
            this.materialX = mx;
            this.mx = mx; // Keep both for backward compatibility
            this.initialized = true;
            return mx;
            
        } catch (error) {
            console.error('❌ Failed to initialize MaterialX:', error);
            throw error;
        }
    }    /**
     * Create a standard surface material for testing
     */
    createStandardSurfaceMaterial() {
        if (!this.mx) {
            throw new Error('MaterialX not initialized. Call initialize() first.');
        }

        const doc = this.mx.createDocument();
        const ssName = 'SR_default';
        const ssNode = doc.addChildOfCategory('standard_surface', ssName);
        ssNode.setType('surfaceshader');
        
        const smNode = doc.addChildOfCategory('surfacematerial', 'Default');
        smNode.setType('material');
        
        const shaderElement = smNode.addInput('surfaceshader');
        shaderElement.setType('surfaceshader');
        shaderElement.setNodeName(ssName);
        
        if (!doc.validate()) {
            throw new Error('Document validation failed');
        }
        
        return doc;
    }

    /**
     * Load MaterialX document from file
     */
    loadDocumentFromFile(filePath) {
        if (!this.mx) {
            throw new Error('MaterialX not initialized. Call initialize() first.');
        }

        if (!fs.existsSync(filePath)) {
            throw new Error(`MaterialX file not found: ${filePath}`);
        }

        const doc = this.mx.createDocument();
        const content = fs.readFileSync(filePath, 'utf8');
        
        try {
            this.mx.readFromXmlString(doc, content);
        } catch (error) {
            throw new Error(`Failed to parse MaterialX file: ${error.message}`);
        }

        if (!doc.validate()) {
            throw new Error('Document validation failed');
        }

        return doc;
    }

    /**
     * Get all available shader generators
     */
    getAvailableGenerators() {
        if (!this.mx) {
            throw new Error('MaterialX not initialized. Call initialize() first.');
        }

        const generators = [];
        const generatorTypes = [
            { name: 'EsslShaderGenerator', target: 'essl', description: 'OpenGL ES Shading Language' },
            { name: 'GlslShaderGenerator', target: 'glsl', description: 'OpenGL Shading Language' },
            { name: 'MslShaderGenerator', target: 'msl', description: 'Metal Shading Language' },
            { name: 'OslShaderGenerator', target: 'osl', description: 'Open Shading Language' },
            { name: 'VkShaderGenerator', target: 'vulkan', description: 'Vulkan SPIR-V' },
            { name: 'WgslShaderGenerator', target: 'wgsl', description: 'WebGPU Shading Language' },
            { name: 'MdlShaderGenerator', target: 'mdl', description: 'Material Definition Language' }
        ];

        for (const genType of generatorTypes) {
            if (typeof this.mx[genType.name] !== 'undefined') {
                try {
                    const generator = this.mx[genType.name].create();
                    generators.push({
                        generator: generator,
                        name: genType.name,
                        target: genType.target,
                        description: genType.description
                    });
                } catch (error) {
                    // Generator creation failed, skip it
                }
            }
        }

        return generators;
    }

    /**
     * Generate shaders for a document using specified generator
     */
    generateShaders(doc, generatorInfo) {
        if (!this.mx) {
            throw new Error('MaterialX not initialized. Call initialize() first.');
        }

        const elem = this.mx.findRenderableElement(doc);
        if (!elem) {
            throw new Error('No renderable element found in document');
        }

        const genContext = new this.mx.GenContext(generatorInfo.generator);
        const stdlib = this.mx.loadStandardLibraries(genContext);
        doc.importLibrary(stdlib);

        try {
            const mxShader = generatorInfo.generator.generate(elem.getNamePath(), elem, genContext);
            
            const result = {
                target: generatorInfo.target,
                elementPath: elem.getNamePath(),
                vertexShader: null,
                pixelShader: null
            };

            // Get vertex shader if available
            try {
                result.vertexShader = mxShader.getSourceCode("vertex");
            } catch (e) {
                // Vertex shader not available for this target
            }

            // Get pixel/fragment shader
            try {
                result.pixelShader = mxShader.getSourceCode("pixel");
            } catch (e) {
                // Try "fragment" as fallback
                try {
                    result.pixelShader = mxShader.getSourceCode("fragment");
                } catch (e2) {
                    // No pixel shader available
                }
            }

            return result;

        } catch (errPtr) {
            const errorMessage = this.mx.getExceptionMessage ? 
                this.mx.getExceptionMessage(errPtr) : errPtr.toString();
            throw new Error(`Shader generation failed: ${errorMessage}`);
        }
    }

    /**
     * Get MaterialX instance
     */
    getMaterialX() {
        return this.mx;
    }

    /**
     * Check if MaterialX is initialized
     */
    isInitialized() {
        return this.initialized && this.mx !== null;
    }
}

module.exports = MaterialXLoader;