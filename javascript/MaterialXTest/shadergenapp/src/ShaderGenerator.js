/**
 * Shader Generator Core
 * Main logic for generating shaders from MaterialX documents
 */

const fs = require('fs-extra');
const path = require('path');

class ShaderGenerator {
    constructor(materialXLoader) {
        this.loader = materialXLoader;
    }

    /**
     * Generate shaders for all available targets
     */
    async generateAllShaders(doc, options = {}) {
        const generators = this.loader.getAvailableGenerators();
        const results = [];

        for (const generatorInfo of generators) {
            try {
                const result = this.loader.generateShaders(doc, generatorInfo);
                results.push({
                    ...result,
                    success: true,
                    generatorInfo: {
                        name: generatorInfo.name,
                        description: generatorInfo.description
                    }
                });
            } catch (error) {
                results.push({
                    target: generatorInfo.target,
                    success: false,
                    error: error.message,
                    generatorInfo: {
                        name: generatorInfo.name,
                        description: generatorInfo.description
                    }
                });
            }
        }

        return results;
    }

    /**
     * Generate shaders for specific targets
     */
    async generateSpecificShaders(doc, targetList) {
        const generators = this.loader.getAvailableGenerators();
        const filteredGenerators = generators.filter(g => targetList.includes(g.target));
        const results = [];

        if (filteredGenerators.length === 0) {
            throw new Error(`No generators found for targets: ${targetList.join(', ')}`);
        }

        for (const generatorInfo of filteredGenerators) {
            try {
                const result = this.loader.generateShaders(doc, generatorInfo);
                results.push({
                    ...result,
                    success: true,
                    generatorInfo: {
                        name: generatorInfo.name,
                        description: generatorInfo.description
                    }
                });
            } catch (error) {
                results.push({
                    target: generatorInfo.target,
                    success: false,
                    error: error.message,
                    generatorInfo: {
                        name: generatorInfo.name,
                        description: generatorInfo.description
                    }
                });
            }
        }

        return results;
    }

    /**
     * Save shader results to files
     */
    async saveResults(results, outputDir) {
        await fs.ensureDir(outputDir);

        const summary = {
            timestamp: new Date().toISOString(),
            totalTargets: results.length,
            successCount: results.filter(r => r.success).length,
            failureCount: results.filter(r => !r.success).length,
            results: []
        };

        for (const result of results) {
            if (result.success) {
                const targetDir = path.join(outputDir, result.target);
                await fs.ensureDir(targetDir);

                // Save vertex shader if available
                if (result.vertexShader) {
                    const vertexPath = path.join(targetDir, 'vertex.shader');
                    await fs.writeFile(vertexPath, result.vertexShader);
                    result.vertexShaderFile = path.relative(outputDir, vertexPath);
                }

                // Save pixel shader if available
                if (result.pixelShader) {
                    const pixelPath = path.join(targetDir, 'pixel.shader');
                    await fs.writeFile(pixelPath, result.pixelShader);
                    result.pixelShaderFile = path.relative(outputDir, pixelPath);
                }

                // Create info file
                const infoPath = path.join(targetDir, 'info.json');
                const info = {
                    target: result.target,
                    elementPath: result.elementPath,
                    generatorInfo: result.generatorInfo,
                    timestamp: new Date().toISOString(),
                    vertexShaderSize: result.vertexShader ? result.vertexShader.length : 0,
                    pixelShaderSize: result.pixelShader ? result.pixelShader.length : 0
                };
                await fs.writeFile(infoPath, JSON.stringify(info, null, 2));
            }

            // Add to summary (without large shader content)
            summary.results.push({
                target: result.target,
                success: result.success,
                error: result.error,
                generatorInfo: result.generatorInfo,
                vertexShaderSize: result.vertexShader ? result.vertexShader.length : 0,
                pixelShaderSize: result.pixelShader ? result.pixelShader.length : 0,
                vertexShaderFile: result.vertexShaderFile,
                pixelShaderFile: result.pixelShaderFile
            });
        }

        // Save summary
        const summaryPath = path.join(outputDir, 'summary.json');
        await fs.writeFile(summaryPath, JSON.stringify(summary, null, 2));

        return summaryPath;
    }

    /**
     * Get statistics from results
     */
    getStatistics(results) {
        const stats = {
            total: results.length,
            successful: 0,
            failed: 0,
            targets: {},
            totalVertexShaderSize: 0,
            totalPixelShaderSize: 0
        };

        for (const result of results) {
            if (result.success) {
                stats.successful++;
                stats.totalVertexShaderSize += result.vertexShader ? result.vertexShader.length : 0;
                stats.totalPixelShaderSize += result.pixelShader ? result.pixelShader.length : 0;
                
                stats.targets[result.target] = {
                    success: true,
                    vertexShaderSize: result.vertexShader ? result.vertexShader.length : 0,
                    pixelShaderSize: result.pixelShader ? result.pixelShader.length : 0
                };
            } else {
                stats.failed++;
                stats.targets[result.target] = {
                    success: false,
                    error: result.error
                };
            }
        }

        return stats;
    }

    /**
     * Validate results and check for common issues
     */
    validateResults(results) {
        const issues = [];

        for (const result of results) {
            if (result.success) {
                if (!result.vertexShader && !result.pixelShader) {
                    issues.push(`${result.target}: No shaders generated`);
                }
                
                if (result.vertexShader && result.vertexShader.length < 10) {
                    issues.push(`${result.target}: Vertex shader unusually small`);
                }
                
                if (result.pixelShader && result.pixelShader.length < 10) {
                    issues.push(`${result.target}: Pixel shader unusually small`);
                }
            } else {
                issues.push(`${result.target}: Generation failed - ${result.error}`);
            }
        }

        return issues;
    }
}

module.exports = ShaderGenerator;