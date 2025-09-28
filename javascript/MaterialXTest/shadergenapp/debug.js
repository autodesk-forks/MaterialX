/**
 * Debug MaterialX Loading
 * Simple script to test MaterialX initialization
 */

const path = require('path');
const fs = require('fs');

async function testMaterialXLoading() {
    console.log('🔍 Debug: Testing MaterialX Loading');
    console.log('='.repeat(50));
    
    const libPath = path.join(__dirname, 'lib');
    console.log(`📂 Library path: ${libPath}`);
    
    // Check available files
    if (fs.existsSync(libPath)) {
        const files = fs.readdirSync(libPath);
        console.log(`📋 Available files: ${files.join(', ')}`);
    } else {
        console.log('❌ Library directory not found');
        return;
    }
    
    // Try loading Core module first (simpler)
    const coreFile = path.join(libPath, 'JsMaterialXCore.js');
    if (fs.existsSync(coreFile)) {
        try {
            console.log('🔄 Loading JsMaterialXCore.js...');
            const MaterialXCore = require(coreFile);
            console.log(`📋 Core module type: ${typeof MaterialXCore}`);
            
            if (typeof MaterialXCore === 'function') {
                console.log('🔄 Initializing Core module...');
                const mx = await MaterialXCore();
                console.log(`✅ Core initialized: ${mx ? 'Success' : 'Failed'}`);
                
                if (mx) {
                    console.log(`📋 MaterialX version: ${mx.getVersionString ? mx.getVersionString() : 'Unknown'}`);
                    
                    // Test basic functionality
                    try {
                        const doc = mx.createDocument();
                        console.log(`✅ Document creation: Success`);
                        console.log(`📋 Document valid: ${doc.validate()}`);
                    } catch (e) {
                        console.log(`❌ Document creation failed: ${e.message}`);
                    }
                }
            }
        } catch (error) {
            console.log(`❌ Core module failed: ${error.message}`);
        }
    }
    
    // Try loading GenShader module
    const genFile = path.join(libPath, 'JsMaterialXGenShader.js');
    if (fs.existsSync(genFile)) {
        try {
            console.log('\n🔄 Loading JsMaterialXGenShader.js...');
            const MaterialXGen = require(genFile);
            console.log(`📋 GenShader module type: ${typeof MaterialXGen}`);
            
            if (typeof MaterialXGen === 'function') {
                console.log('🔄 Initializing GenShader module...');
                const mx = await MaterialXGen();
                console.log(`✅ GenShader initialized: ${mx ? 'Success' : 'Failed'}`);
                
                if (mx) {
                    console.log(`📋 MaterialX version: ${mx.getVersionString ? mx.getVersionString() : 'Unknown'}`);
                    
                    // Test shader generator availability
                    const generators = [];
                    const generatorTypes = ['EsslShaderGenerator', 'GlslShaderGenerator', 'MslShaderGenerator'];
                    
                    for (const genType of generatorTypes) {
                        if (typeof mx[genType] !== 'undefined') {
                            console.log(`✅ ${genType}: Available`);
                            generators.push(genType);
                        } else {
                            console.log(`❌ ${genType}: Not available`);
                        }
                    }
                    
                    console.log(`📊 Total generators: ${generators.length}`);
                }
            }
        } catch (error) {
            console.log(`❌ GenShader module failed: ${error.message}`);
        }
    }
}

testMaterialXLoading().catch(error => {
    console.error('💥 Debug script failed:', error);
});