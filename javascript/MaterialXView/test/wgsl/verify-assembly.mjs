/**
 * Full NodeMaterial assembly verification (no GPU required).
 * Run: npm run verify:wgsl-assembly
 */

import { createMxWgslMaterial } from '../../source/mxtsladapter.js';
import { wgsl, manifest } from './sample.js';
import * as TSL from 'three/tsl';

class Vec {

	constructor( ...a ) {

		this.elements = a;

	}

}

class Texture {

	constructor() {

		this.isTexture = true;

	}

}

const THREE = {
	MeshBasicNodeMaterial: class {

		constructor() {

			this.colorNode = null;
			this.userData = {};

		}

	},
	Vector2: Vec,
	Vector3: Vec,
	Vector4: Vec,
	Texture,
	DataTexture: class {

		constructor() {

			this.isTexture = true;
			this.needsUpdate = false;

		}

	}
};

let failures = 0;
function check( cond, msg ) {

	console[ cond ? 'log' : 'error' ]( `  ${ cond ? 'ok  ' : 'FAIL' }- ${ msg }` );
	if ( ! cond ) failures ++;

}

console.log( 'Assembling NodeMaterial from MaterialX WGSL with real TSL...' );

let material;
try {

	material = createMxWgslMaterial( {
		THREE,
		TSL,
		wgsl,
		manifest,
		textures: { base_color_map: new Texture() }
	} );
	console.log( '  (no exception thrown during assembly)' );

} catch ( e ) {

	failures ++;
	console.error( '  FAIL- assembly threw: ' + e.message );

}

if ( material ) {

	check( material.colorNode != null, 'material.colorNode was assigned' );
	check( typeof material.colorNode === 'object', 'colorNode is a node object' );
	check( material.colorNode.isNode === true || material.colorNode.nodeType !== undefined || typeof material.colorNode.build === 'function',
		'colorNode looks like a TSL Node' );
	check( material.userData.mxArgs?.base_color_map_texture != null, 'texture arg was bound' );
	check( material.userData.mxArgs?.base_color_map_sampler != null, 'sampler arg was bound' );

}

// Brass-like materials expose many uniforms, u_lightData (uniformArray), and multiple texture pairs.
console.log( '\nAssembling multi-texture + light-data fixture...' );
const multiWgsl = /* wgsl */`
fn mx_lightData_at(data: array<vec4f, 4>, index: i32) -> vec4f { return data[u32(index)]; }
fn material_main(uv: vec2f, tint: f32, u_lightData: array<vec4f, 4>, env_a: texture_2d<f32>, env_a_sampler: sampler, env_b: texture_2d<f32>, env_b_sampler: sampler, color_map: texture_2d<f32>, color_map_sampler: sampler, rough_map: texture_2d<f32>, rough_map_sampler: sampler) -> vec4f {
    let c = textureSample(color_map, color_map_sampler, uv).rgb;
    let r = textureSample(rough_map, rough_map_sampler, uv).r;
    return vec4f(c * tint + vec3f(r), 1.0);
}`;
const multiManifest = {
	entry: 'material_main',
	entryParams: [ { name: 'uv', type: 'vec2f', semantic: 'varying:uv' } ],
	uniforms: [
		{ name: 'tint', type: 'f32', semantic: 'uniform', value: 1 },
		{ name: 'u_lightData', type: 'array<vec4f, 4>', semantic: 'light:data', value: [] }
	],
	textures: [
		{ key: 'u_envRadiance', texture: 'env_a', sampler: 'env_a_sampler', wgslType: 'texture_2d<f32>', semantic: 'env:radiance' },
		{ key: 'u_envIrradiance', texture: 'env_b', sampler: 'env_b_sampler', wgslType: 'texture_2d<f32>', semantic: 'env:irradiance' },
		{ key: 'image_color', texture: 'color_map', sampler: 'color_map_sampler', wgslType: 'texture_2d<f32>' },
		{ key: 'image_roughness', texture: 'rough_map', sampler: 'rough_map_sampler', wgslType: 'texture_2d<f32>' }
	]
};
try {

	const multiMat = createMxWgslMaterial( {
		THREE,
		TSL,
		wgsl: multiWgsl,
		manifest: multiManifest,
		textures: { image_color: new Texture(), image_roughness: new Texture() },
		lightData: [],
		numLights: 0
	} );
	check( multiMat.colorNode != null, 'multi-texture material assembled' );
	check( multiMat.userData.mxArgs.color_map != null && multiMat.userData.mxArgs.color_map_sampler != null,
		'material texture/sampler pair bound' );
	check( multiMat.userData.mxArgs.env_a != null && multiMat.userData.mxArgs.env_a_sampler != null,
		'env texture/sampler pair bound' );

} catch ( e ) {

	failures ++;
	console.error( '  FAIL- multi-texture assembly threw: ' + e.message );

}

console.log( failures === 0 ? '\nASSEMBLY CHECKS PASSED' : `\n${ failures } CHECK(S) FAILED` );
process.exit( failures === 0 ? 0 : 1 );
