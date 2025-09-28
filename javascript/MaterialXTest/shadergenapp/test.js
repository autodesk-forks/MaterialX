/**
 * Test script for MaterialX Shader Generator
 * Simple test to verify the functionality works
 */

const { MaterialXShaderGen } = require('./index');
const path = require('path');

async function runTest() {
    console.log('🧪 Running MaterialX Shader Generator Test');
    console.log('='.repeat(50));
    
    try {
        // Initialize the generator
        console.log('🔄 Initializing MaterialX Shader Generator...');
        const shaderGen = new MaterialXShaderGen();
        await shaderGen.initialize();
        console.log('✅ Initialization successful');
        
        // Show available targets
        const targets = shaderGen.getAvailableTargets();
        console.log(`📋 Available targets: ${targets.map(t => t.target).join(', ')}`);
        
        // Create a standard material
        console.log('🎨 Creating standard surface material...');
        const doc = shaderGen.createStandardMaterial();
        console.log('✅ Material created');
        
        // Generate shaders for all targets
        console.log('🎯 Generating shaders for all targets...');
        const results = await shaderGen.generateAll(doc);
        
        // Show statistics
        const stats = shaderGen.getStatistics(results);
        console.log('\n📊 Test Results:');
        console.log(`✅ Successful generations: ${stats.successful}`);
        console.log(`❌ Failed generations: ${stats.failed}`);
        console.log(`📋 Total targets: ${stats.total}`);
        
        // Show detailed results
        console.log('\n📋 Detailed Results:');
        for (const result of results) {
            if (result.success) {
                console.log(`  ✅ ${result.target}: vertex(${result.vertexShader ? result.vertexShader.length : 0}), pixel(${result.pixelShader ? result.pixelShader.length : 0})`);
            } else {
                console.log(`  ❌ ${result.target}: ${result.error}`);
            }
        }
        
        // Validate results
        const issues = shaderGen.validateResults(results);
        if (issues.length > 0) {
            console.log('\n⚠️  Issues found:');
            for (const issue of issues) {
                console.log(`  • ${issue}`);
            }
        } else {
            console.log('\n✅ All results validated successfully');
        }
        
        // Save results to output directory
        const outputDir = path.join(__dirname, 'test-output');
        console.log(`\n💾 Saving results to: ${outputDir}`);
        const summaryPath = await shaderGen.saveResults(results, outputDir);
        console.log(`✅ Results saved. Summary: ${summaryPath}`);
        
        console.log('\n🎉 Test completed successfully!');
        
    } catch (error) {
        console.error('\n💥 Test failed:', error.message);
        if (error.stack) {
            console.error('Stack trace:', error.stack);
        }
        process.exit(1);
    }
}

// Run the test if this file is executed directly
if (require.main === module) {
    runTest();
}

module.exports = runTest;