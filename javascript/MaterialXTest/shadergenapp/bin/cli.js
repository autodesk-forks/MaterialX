#!/usr/bin/env node

/**
 * MaterialX Shader Generator CLI
 * Command-line interface for generating shaders from MaterialX documents
 */

const { Command } = require('commander');
const chalk = require('chalk');
const fs = require('fs-extra');
const path = require('path');

const MaterialXLoader = require('../src/MaterialXLoader');
const ShaderGenerator = require('../src/ShaderGenerator');

const program = new Command();

// Global variables
let loader = null;
let generator = null;

/**
 * Initialize MaterialX and shader generator
 */
async function initializeGenerator() {
    if (loader && generator) {
        return { loader, generator };
    }

    try {
        console.log(chalk.blue('🔄 Initializing MaterialX...'));
        
        loader = new MaterialXLoader();
        await loader.initialize();
        generator = new ShaderGenerator(loader);
        
        console.log(chalk.green('✅ MaterialX initialized successfully'));
        
        // Show available generators
        const generators = loader.getAvailableGenerators();
        console.log(chalk.blue(`📋 Available shader targets: ${generators.map(g => g.target).join(', ')}`));
        
        return { loader, generator };
        
    } catch (error) {
        console.error(chalk.red('❌ Failed to initialize MaterialX:'), error.message);
        console.error(chalk.yellow('💡 Make sure to run "npm run build" first to copy the MaterialX build files.'));
        process.exit(1);
    }
}

/**
 * Generate shaders from MaterialX file or create default material
 */
async function generateShaders(options) {
    const { loader, generator } = await initializeGenerator();
    
    let doc;
    
    try {
        if (options.input) {
            console.log(chalk.blue(`📂 Loading MaterialX document: ${options.input}`));
            doc = loader.loadDocumentFromFile(options.input);
            console.log(chalk.green('✅ Document loaded successfully'));
        } else {
            console.log(chalk.blue('🎨 Creating default standard surface material...'));
            doc = loader.createStandardSurfaceMaterial();
            console.log(chalk.green('✅ Default material created'));
        }
        
        // Generate shaders
        let results;
        if (options.targets) {
            const targetList = options.targets.split(',').map(t => t.trim());
            console.log(chalk.blue(`🎯 Generating shaders for targets: ${targetList.join(', ')}`));
            results = await generator.generateSpecificShaders(doc, targetList);
        } else {
            console.log(chalk.blue('🎯 Generating shaders for all available targets...'));
            results = await generator.generateAllShaders(doc);
        }
        
        // Show generation statistics
        const stats = generator.getStatistics(results);
        console.log(chalk.blue('\n📊 Generation Statistics:'));
        console.log(chalk.green(`✅ Successful: ${stats.successful}`));
        console.log(chalk.red(`❌ Failed: ${stats.failed}`));
        console.log(chalk.blue(`📋 Total: ${stats.total}`));
        
        // Show detailed results
        for (const result of results) {
            if (result.success) {
                console.log(chalk.green(`  ✅ ${result.target}: `), 
                           `vertex(${result.vertexShader ? result.vertexShader.length : 0}), `,
                           `pixel(${result.pixelShader ? result.pixelShader.length : 0})`);
            } else {
                console.log(chalk.red(`  ❌ ${result.target}: ${result.error}`));
            }
        }
        
        // Validate results
        const issues = generator.validateResults(results);
        if (issues.length > 0) {
            console.log(chalk.yellow('\n⚠️  Issues found:'));
            for (const issue of issues) {
                console.log(chalk.yellow(`  • ${issue}`));
            }
        }
        
        // Save results if output directory specified
        if (options.output) {
            console.log(chalk.blue(`\n💾 Saving results to: ${options.output}`));
            const summaryPath = await generator.saveResults(results, options.output);
            console.log(chalk.green(`✅ Results saved. Summary: ${summaryPath}`));
        }
        
        // Console output options
        if (options.console) {
            console.log(chalk.blue('\n📄 Shader Output:'));
            console.log('='.repeat(80));
            
            for (const result of results) {
                if (result.success) {
                    console.log(chalk.bold(`\n${result.target.toUpperCase()} SHADERS:`));
                    console.log('-'.repeat(40));
                    
                    if (result.vertexShader) {
                        console.log(chalk.cyan('VERTEX SHADER:'));
                        console.log(result.vertexShader);
                        console.log('');
                    }
                    
                    if (result.pixelShader) {
                        console.log(chalk.magenta('PIXEL SHADER:'));
                        console.log(result.pixelShader);
                        console.log('');
                    }
                }
            }
        }
        
        console.log(chalk.green('\n🎉 Shader generation completed!'));
        
    } catch (error) {
        console.error(chalk.red('💥 Error:'), error.message);
        process.exit(1);
    }
}

/**
 * List available shader generators
 */
async function listGenerators() {
    const { loader } = await initializeGenerator();
    
    const generators = loader.getAvailableGenerators();
    
    console.log(chalk.blue('📋 Available Shader Generators:'));
    console.log('='.repeat(60));
    
    if (generators.length === 0) {
        console.log(chalk.yellow('⚠️  No shader generators available.'));
        console.log(chalk.gray('   This is expected when using MaterialX Core module only.'));
        console.log(chalk.gray('   Core module provides document parsing and manipulation capabilities.'));
        console.log('');
        console.log(chalk.cyan('📋 MaterialX Core Features Available:'));
        console.log(chalk.green('   ✅ Document creation and parsing'));
        console.log(chalk.green('   ✅ MaterialX file loading'));  
        console.log(chalk.green('   ✅ Node graph manipulation'));
        console.log(chalk.green('   ✅ Material definition processing'));
        console.log(chalk.green('   ✅ XML import/export'));
        console.log('');
        console.log(chalk.blue('💡 For shader generation, the GenShader module would be needed'));
        console.log(chalk.gray('   (requires web environment for WASM preloaded data)'));
    } else {
        for (const gen of generators) {
            console.log(chalk.green(`${gen.target.padEnd(10)}`), chalk.white(`${gen.description}`));
            console.log(chalk.gray(`${''.padEnd(10)} Class: ${gen.name}`));
            console.log('');
        }
    }
    
    console.log('');
    console.log(chalk.blue(`Total generators available: ${generators.length}`));
}

/**
 * Validate MaterialX document
 */
async function validateDocument(inputFile) {
    const { loader } = await initializeGenerator();
    
    try {
        console.log(chalk.blue(`📂 Validating MaterialX document: ${inputFile}`));
        const doc = loader.loadDocumentFromFile(inputFile);
        console.log(chalk.green('✅ Document is valid!'));
        
        // Show document info
        const mx = loader.getMaterialX();
        const elem = mx.findRenderableElement(doc);
        if (elem) {
            console.log(chalk.blue(`📋 Renderable element: ${elem.getNamePath()}`));
        } else {
            console.log(chalk.yellow('⚠️  No renderable element found in document'));
        }
        
    } catch (error) {
        console.error(chalk.red('❌ Document validation failed:'), error.message);
        process.exit(1);
    }
}

// CLI Setup
program
    .name('materialx-shader-gen')
    .description('MaterialX Shader Generator CLI')
    .version('1.0.0');

program
    .command('generate')
    .alias('gen')
    .description('Generate shaders from MaterialX document or create default material')
    .option('-i, --input <file>', 'Input MaterialX file (.mtlx)')
    .option('-o, --output <dir>', 'Output directory for generated shaders')
    .option('-t, --targets <targets>', 'Comma-separated list of shader targets (essl,glsl,msl,osl,vulkan,wgsl,mdl)')
    .option('-c, --console', 'Output shaders to console')
    .action(generateShaders);

program
    .command('list')
    .alias('ls')
    .description('List available shader generators')
    .action(listGenerators);

program
    .command('validate')
    .alias('val')
    .description('Validate a MaterialX document')
    .argument('<file>', 'MaterialX file to validate')
    .action(validateDocument);

program
    .command('info')
    .description('Show MaterialX information')
    .action(async () => {
        const { loader } = await initializeGenerator();
        const mx = loader.getMaterialX();
        
        console.log(chalk.blue('📋 MaterialX Information:'));
        console.log('='.repeat(40));
        console.log(chalk.green('Version:'), mx.getVersionString ? mx.getVersionString() : 'Unknown');
        console.log(chalk.green('Initialized:'), loader.isInitialized() ? 'Yes' : 'No');
        
        const generators = loader.getAvailableGenerators();
        console.log(chalk.green('Available Generators:'), generators.length);
        
        for (const gen of generators) {
            console.log(chalk.gray(`  • ${gen.target} (${gen.description})`));
        }
    });

// Error handling
program.configureHelp({
    sortSubcommands: true,
});

program.on('command:*', () => {
    console.error(chalk.red('Invalid command: %s'), program.args.join(' '));
    console.log(chalk.yellow('See --help for a list of available commands.'));
    process.exit(1);
});

// Parse command line arguments
if (process.argv.length === 2) {
    program.outputHelp();
} else {
    program.parse();
}