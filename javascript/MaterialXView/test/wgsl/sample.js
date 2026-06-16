/**
 * Representative MaterialX -> WGSL output for bridge converter tests.
 */

export const wgsl = /* wgsl */`
@group(0) @binding(0) var<uniform> base_color: vec3f;
@group(0) @binding(1) var<uniform> emissive: vec3f;
@group(0) @binding(2) var<uniform> tint_strength: f32;
@group(0) @binding(3) var base_color_map_texture: texture_2d<f32>;
@group(0) @binding(4) var base_color_map_sampler: sampler;

fn mx_srgb_texture_to_lin_rec709(c: vec3f) -> vec3f {
    let lo = c / 12.92;
    let hi = pow((c + vec3f(0.055)) / 1.055, vec3f(2.4));
    let cutoff = c <= vec3f(0.04045);
    return select(hi, lo, cutoff);
}

fn material_main(uv: vec2f) -> vec4f {
    let sampled = textureSample(base_color_map_texture, base_color_map_sampler, uv).rgb;
    let albedo = mx_srgb_texture_to_lin_rec709(sampled) * base_color;
    let tinted = mix(albedo, albedo * base_color, tint_strength);
    return vec4f(tinted + emissive, 1.0);
}
`;

export const manifest = {
	entry: 'material_main',
	output: 'vec4f',
	entryParams: [
		{ name: 'uv', type: 'vec2f', semantic: 'varying:uv' }
	],
	uniforms: [
		{ name: 'base_color', type: 'vec3f', value: [ 0.9, 0.55, 0.3 ] },
		{ name: 'emissive', type: 'vec3f', value: [ 0.0, 0.0, 0.0 ] },
		{ name: 'tint_strength', type: 'f32', value: 0.0 }
	],
	textures: [
		{ key: 'base_color_map', texture: 'base_color_map_texture', sampler: 'base_color_map_sampler', wgslType: 'texture_2d<f32>' }
	]
};
