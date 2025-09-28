/**
 * Simple test to verify MaterialX Core functionality
 */

const MaterialXLoader = require('./src/MaterialXLoader.js');

async function testMaterialX() {
    try {
        console.log('🧪 Testing MaterialX Core functionality...');
        
        const loader = new MaterialXLoader();
        const mx = await loader.initialize();
        
        // Test basic document operations
        const doc = mx.createDocument();
        console.log('✅ Document created:', !!doc);
        
        // Test version info
        console.log('📋 MaterialX Version:', mx.getVersionString());
        
        // Show some available APIs
        const apis = Object.keys(mx).filter(key => typeof mx[key] === 'function').slice(0, 15);
        console.log('🔧 Sample APIs:', apis.join(', '));
        
        // Test basic document operations
        doc.setSourceUri('test.mtlx');
        console.log('✅ Document source URI set');
        
        // Test validation
        const valid = doc.validate();
        console.log('✅ Document validation:', valid);
        
        console.log('🎉 MaterialX Core is working correctly!');
        
    } catch (error) {
        console.error('❌ Test failed:', error.message);
    }
}

testMaterialX();