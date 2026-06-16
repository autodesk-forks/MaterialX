/**
 * Bridge converter verification (no GPU required).
 * Run: npm run verify:wgsl
 */

import { convertToTslPortable, normalizeReflection, packLightDataToVec4Array, parseLightDataStruct } from '../../source/mxtsladapter.js';
import { wgsl, manifest } from './sample.js';
import WGSLNodeFunction from 'three/src/renderers/webgpu/nodes/WGSLNodeFunction.js';

let failures = 0;
function check( cond, msg ) {

	if ( cond ) {

		console.log( `  ok  - ${ msg }` );

	} else {

		failures ++;
		console.error( `  FAIL- ${ msg }` );

	}

}

console.log( 'Converting sample MaterialX WGSL -> TSL-portable...' );
const { name, entry, includes, params } = convertToTslPortable( wgsl, manifest );

check( ( entry.match( /\bfn\s/g ) || [] ).length === 1, 'entry contains exactly one fn' );
check( ! /@group\s*\(/.test( entry ), 'entry has no @group binding declarations' );
check( includes.includes( 'mx_srgb_texture_to_lin_rec709' ), 'helper function preserved in includes' );
check( ! /@group\s*\(/.test( includes ), 'includes have no @group binding declarations' );
check( ! /var\s*<\s*uniform/.test( includes ), 'uniform bindings stripped from includes' );

let parsed;
try {

	const nodeFunction = new WGSLNodeFunction( entry );
	parsed = nodeFunction;
	console.log( '\nParsed by WGSLNodeFunction:' );
	console.log( '  name   =', nodeFunction.name );
	console.log( '  type   =', nodeFunction.type );
	console.log( '  inputs =', nodeFunction.inputs.map( ( i ) => `${ i.name }:${ i.type }` ).join( ', ' ) );

} catch ( e ) {

	failures ++;
	console.error( '  FAIL- WGSLNodeFunction threw: ' + e.message );

}

if ( parsed ) {

	check( parsed.name === name, `parsed name is '${ name }'` );
	check( parsed.outputType === 'vec4f', 'parsed output type is vec4f' );

	const byName = Object.fromEntries( parsed.inputs.map( ( i ) => [ i.name, i.type ] ) );
	check( byName[ 'uv' ] === 'vec2', 'uv mapped to TSL vec2' );
	check( byName[ 'base_color' ] === 'vec3', 'base_color mapped to TSL vec3' );
	check( byName[ 'tint_strength' ] === 'float', 'tint_strength mapped to TSL float' );
	check( byName[ 'base_color_map_texture' ] === 'texture', 'texture param mapped to TSL texture' );
	check( byName[ 'base_color_map_sampler' ] === 'sampler', 'sampler param mapped to TSL sampler' );
	check( parsed.inputs.every( ( i ) => i.type !== undefined ), 'all parameters resolve to a TSL type' );

}

console.log( '\nNormalizing neutral generator reflection for property editor...' );
const neutralManifest = {
	entry: { pixel: 'material_main' },
	bindings: [
		{ name: 'SR_default_base', type: 'f32', role: 'uniform', value: 1.0 },
		{ name: 'SR_default_base_color', type: 'vec3f', role: 'uniform', value: [ 0.8, 0.8, 0.8 ] },
		{ name: 'u_envLightIntensity', type: 'f32', role: 'host', value: 1.0 }
	]
};
const guiManifest = normalizeReflection( neutralManifest );
const surfaceUniforms = ( guiManifest.uniforms || [] ).filter( ( u ) => u.semantic === 'uniform' );
check( surfaceUniforms.length === 2, 'bindings-format manifest exposes surface uniforms for GUI' );
check( surfaceUniforms.some( ( u ) => u.name === 'SR_default_base_color' ), 'base_color uniform normalized' );

console.log( '\nPacking LightData for uniformArray (i32 bitcast)...' );
const lightsWgsl = `
struct LightData {
    light_type: i32,
    direction: vec3f,
    color: vec3f,
    intensity: f32,
}`;
const lightLayout = parseLightDataStruct( lightsWgsl );
const packedLights = packLightDataToVec4Array( [ {
	type: 1,
	direction: { x: 0.5, y: -0.5, z: 0.707 },
	color: { x: 1, y: 0.9, z: 0.8 },
	intensity: 2.5
} ], 1, lightLayout );
const intBits = new DataView( new ArrayBuffer( 4 ) );
intBits.setFloat32( 0, packedLights[ 0 ].x, true );
check( intBits.getInt32( 0, true ) === 1, 'light_type stored as i32 bit pattern (not float 1.0)' );
check( Math.abs( packedLights[ 1 ].x - 0.5 ) < 1e-6, 'direction.x packed at vec4 stride offset' );
check( Math.abs( packedLights[ 2 ].w - 2.5 ) < 1e-6, 'intensity packed in struct layout' );

console.log( failures === 0 ? '\nALL CHECKS PASSED' : `\n${ failures } CHECK(S) FAILED` );
process.exit( failures === 0 ? 0 : 1 );
