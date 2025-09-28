/**
 * MaterialX Shader Generator - Main Module
 * Provides programmatic access to shader generation functionality
 */

const MaterialXLoader = require('./src/MaterialXLoader');
const ShaderGenerator = require('./src/ShaderGenerator');

/**
 * Main MaterialX Shader Generator class
 */
class MaterialXShaderGen {
    constructor() {
        this.loader = null;
        this.generator = null;
        this.initialized = false;
    }

    /**
     * Initialize the shader generator
     */
    async initialize() {
        if (this.initialized) {
            return this;
        }

        this.loader = new MaterialXLoader();
        await this.loader.initialize();
        this.generator = new ShaderGenerator(this.loader);
        this.initialized = true;
        
        return this;
    }

    /**
     * Create a standard surface material
     */
    createStandardMaterial() {
        this._ensureInitialized();
        return this.loader.createStandardSurfaceMaterial();
    }

    /**
     * Load MaterialX document from file
     */
    loadDocument(filePath) {
        this._ensureInitialized();
        return this.loader.loadDocumentFromFile(filePath);
    }

    /**
     * Get available shader generators
     */
    getAvailableTargets() {
        this._ensureInitialized();
        return this.loader.getAvailableGenerators().map(g => ({
            target: g.target,
            name: g.name,
            description: g.description
        }));
    }

    /**
     * Generate shaders for all available targets
     */
    async generateAll(doc) {
        this._ensureInitialized();
        return this.generator.generateAllShaders(doc);
    }

    /**
     * Generate shaders for specific targets
     */
    async generateTargets(doc, targets) {
        this._ensureInitialized();
        return this.generator.generateSpecificShaders(doc, targets);
    }

    /**
     * Save results to files
     */
    async saveResults(results, outputDir) {
        this._ensureInitialized();
        return this.generator.saveResults(results, outputDir);
    }

    /**
     * Get statistics from results
     */
    getStatistics(results) {
        this._ensureInitialized();
        return this.generator.getStatistics(results);
    }

    /**
     * Validate results
     */
    validateResults(results) {
        this._ensureInitialized();
        return this.generator.validateResults(results);
    }

    /**
     * Get MaterialX instance
     */
    getMaterialX() {
        this._ensureInitialized();
        return this.loader.getMaterialX();
    }

    /**
     * Check if initialized
     */
    isInitialized() {
        return this.initialized;
    }

    /**
     * Internal helper to ensure initialization
     */
    _ensureInitialized() {
        if (!this.initialized) {
            throw new Error('MaterialX Shader Generator not initialized. Call initialize() first.');
        }
    }
}

// Export classes and main interface
module.exports = {
    MaterialXShaderGen,
    MaterialXLoader,
    ShaderGenerator
};

// Export main class as default
module.exports.default = MaterialXShaderGen;