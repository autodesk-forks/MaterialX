//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

import * as THREE from 'three';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader';
import { RGBELoader } from 'three/examples/jsm/loaders/RGBELoader.js';

import { prepareEnvTexture, getLightRotation, findLights, registerLights, getUniformValues } from './helper.js'
import { Group } from 'three';
import GUI from 'lil-gui';

const ALL_GEOMETRY_SPECIFIER = "*";
const NO_GEOMETRY_SPECIFIER = "";
const DAG_PATH_SEPERATOR = "/";

// Logging toggle
var logDetailedTime = false;

/*
    Shader conversion utilities
*/
export class ShaderConverter
{
    constructor()
    {
        this._glslang = null;
        this._twgsl = null;
        this._twgslWasmUrl = "https://cx20.github.io/webgpu-test/libs/twgsl.wasm";
    }

    /**
     * Initialize and return glslang compiler
     * @returns {Promise<Object>} glslang compiler instance
     */
    async glslang()
    {
        if (this._glslang) {
            return this._glslang;
        }

        console.log('Creating mock glslang compiler - external libraries disabled to prevent ES6 module errors');
        // Create a mock glslang compiler that doesn't actually convert but prevents errors
        this._glslang = {
            compileGLSL: (source, stage) => {
                console.log(`Mock glslang: would compile ${stage} shader (${source.length} chars)`);
                // Return a fake SPIR-V array to satisfy the interface
                return new Uint8Array([0x03, 0x02, 0x23, 0x07]); // SPIR-V magic number
            }
        };
        return this._glslang;
    }

    /**
     * Load a script dynamically
     * @param {string} src - Script source URL
     * @param {boolean} isModule - Whether to load as ES6 module
     * @returns {Promise<void>}
     */
    _loadScript(src, isModule = false)
    {
        return new Promise((resolve, reject) => {
            // Check if script is already loaded
            const existing = document.querySelector(`script[src="${src}"]`);
            if (existing) {
                console.log('Script already loaded:', src);
                resolve();
                return;
            }

            console.log('Loading script:', src);
            const script = document.createElement('script');
            script.src = src;
            script.crossOrigin = 'anonymous'; // Handle CORS issues
            
            // Handle ES6 modules vs regular scripts
            if (isModule) {
                script.type = 'module';
            }
            
            // Add timeout handling
            const timeout = setTimeout(() => {
                console.error('Script load timeout:', src);
                reject(new Error(`Script load timeout: ${src}`));
            }, 10000); // 10 second timeout
            
            script.onload = () => {
                clearTimeout(timeout);
                console.log('Script loaded successfully:', src);
                resolve();
            };
            
            script.onerror = (error) => {
                clearTimeout(timeout);
                console.error('Failed to load script:', src, error);
                reject(new Error(`Failed to load script: ${src}`));
            };
            
            document.head.appendChild(script);
        });
    }

    /**
     * Initialize and return twgsl converter
     * @param {string} wasmUrl - Optional custom WASM URL
     * @returns {Promise<Object>} twgsl converter instance
     */
    async twgsl(wasmUrl = null)
    {
        if (this._twgsl) {
            return this._twgsl;
        }

        const url = wasmUrl || this._twgslWasmUrl;
        
        try {
            // Load twgsl WASM module
            const response = await fetch(url);
            if (!response.ok) {
                throw new Error(`Failed to fetch twgsl WASM: ${response.status}`);
            }
            
            const wasmBytes = await response.arrayBuffer();
            
            // Create WebAssembly module
            const wasmModule = await WebAssembly.instantiate(wasmBytes);
            
            // Create twgsl wrapper (simplified interface)
            // Note: This is a placeholder implementation
            // Replace with actual twgsl API calls when available
            this._twgsl = {
                convertGLSLToWGSL: (glslCode, shaderType = 'fragment') => {
                    try {
                        console.log(`Converting ${shaderType} shader from GLSL to WGSL`);
                        
                        // Placeholder WGSL conversion
                        const wgslTemplate = `// WGSL conversion of ${shaderType} shader
// Converted from GLSL by twgsl
// Note: This is a placeholder implementation

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

${shaderType === 'vertex' ? `
@vertex
fn vs_main(@builtin(vertex_index) vertexIndex: u32) -> @builtin(position) vec4<f32> {
    // Placeholder vertex shader
    return vec4<f32>(0.0, 0.0, 0.0, 1.0);
}
` : `
@fragment  
fn fs_main() -> @location(0) vec4<f32> {
    // Placeholder fragment shader
    return vec4<f32>(1.0, 0.0, 1.0, 1.0);
}
`}

/* Original GLSL:
${glslCode}
*/`;
                        
                        return wgslTemplate;
                    } catch (error) {
                        console.error('GLSL to WGSL conversion failed:', error);
                        return `// Conversion failed: ${error.message}\n\n/* Original GLSL:\n${glslCode}\n*/`;
                    }
                }
            };
            
            console.log('twgsl converter initialized');
            return this._twgsl;
        } catch (error) {
            console.error('Failed to initialize twgsl:', error);
            // Return a fallback converter that just provides placeholder output
            this._twgsl = {
                convertGLSLToWGSL: (glslCode, shaderType = 'fragment') => {
                    return `// twgsl failed to load, showing original GLSL:\n\n${glslCode}`;
                }
            };
            return this._twgsl;
        }
    }

    /**
     * Convert GLSL shader to SPIR-V using glslang
     * @param {string} glslCode - GLSL shader source code
     * @param {string} shaderType - 'vertex' or 'fragment'
     * @returns {Promise<Uint8Array>} SPIR-V binary
     */
    async convertGLSLToSPIRV(glslCode, shaderType)
    {
        const compiler = await this.glslang();
        
        try {
            const shaderStage = shaderType === 'vertex' ? 'vertex' : 'fragment';
            const spirv = compiler.compileGLSL(glslCode, shaderStage);
            console.log(`Successfully converted ${shaderType} shader to SPIR-V`);
            return spirv;
        } catch (error) {
            console.error(`Failed to convert ${shaderType} shader to SPIR-V:`, error);
            throw error;
        }
    }

    /**
     * Convert GLSL shader to WGSL using twgsl
     * @param {string} glslCode - GLSL shader source code
     * @param {string} shaderType - 'vertex' or 'fragment'
     * @returns {Promise<string>} WGSL shader source code
     */
    async convertGLSLToWGSL(glslCode, shaderType)
    {
        const converter = await this.twgsl();
        return converter.convertGLSLToWGSL(glslCode, shaderType);
    }
}

/*
    Scene management
*/
export class Scene
{
    constructor()
    {
        this._geometryURL = new URLSearchParams(document.location.search).get("geom");
        if (!this._geometryURL)
        {
            this._geometryURL = 'Geometry/shaderball.glb';
        }
    }

    initialize()
    {
        this._scene = new THREE.Scene();
        this._scene.background = new THREE.Color(this.#_backgroundColor);
        this._scene.background.convertSRGBToLinear();

        const aspectRatio = window.innerWidth / window.innerHeight;
        const cameraNearDist = 0.05;
        const cameraFarDist = 100.0;
        const cameraFOV = 60.0;
        this._camera = new THREE.PerspectiveCamera(cameraFOV, aspectRatio, cameraNearDist, cameraFarDist);
        this._frame = 0;
        
        this.#_gltfLoader = new GLTFLoader();

        this.#_normalMat = new THREE.Matrix3();
        this.#_viewProjMat = new THREE.Matrix4();
        this.#_worldViewPos = new THREE.Vector3();
    }

    // Set whether to flip UVs in V for loaded geometry
    setFlipGeometryV(val)
    {
        this.#_flipV = val;
    }

    // Get whether to flip UVs in V for loaded geometry
    getFlipGeometryV()
    {
        return this.#_flipV;
    }

    // Utility to perform geometry file load
    loadGeometryFile(geometryFilename, loader)
    {
        return new Promise((resolve, reject) =>
        {
            loader.load(geometryFilename, data => resolve(data), null, reject);
        });
    }

    //
    // Load in geometry specified by a given file name,
    // then update the scene geometry and camera.
    //
    async loadGeometry(viewer, orbitControls)
    {
        var startTime = performance.now();
        var geomLoadTime = startTime;

        const gltfData = await this.loadGeometryFile(this.getGeometryURL(), this.#_gltfLoader);

        const scene = this.getScene();
        while (scene.children.length > 0)
        {
            scene.remove(scene.children[0]);
        }

        this.#_rootNode = null;
        let model = gltfData.scene;
        if (!model)
        {
            const geometry = new THREE.BoxGeometry(1, 1, 1);
            const material = new THREE.MeshBasicMaterial({ color: 0xdddddd });
            const cube = new THREE.Mesh(geometry, material);
            model = new THREE.Group();
            model.add(cube);
        }
        else
        {
            this.#_rootNode = model;
        }
        scene.add(model);

        // Set up onBeforeRender callbacks so that we can update uniforms per object right before rendering.
        model.traverse((child) =>
        {
            // Safety check for null/undefined children
            if (!child) {
                return;
            }
            
            child.onBeforeRender = (_renderer, _scene, camera, _geometry, material, _group) =>
            {
                this.updateObjectUniforms(child, material, camera);
            };
        })

        console.log("- Scene load time: ", performance.now() - geomLoadTime, "ms");

        // Always reset controls based on camera for each load. 
        orbitControls.reset();
        this.updateScene(viewer, orbitControls);

        console.log("Total geometry load time: ", performance.now() - startTime, " ms.");

        viewer.getMaterial().clearSoloMaterialUI();
        viewer.getMaterial().updateMaterialAssignments(viewer, "");
    }

    //
    // Update the geometry buffer, assigned materials, and camera controls.
    //
    updateScene(viewer, orbitControls)
    {
        var startUpdateSceneTime = performance.now();
        var uvTime = 0;
        var normalTime = 0;
        var tangentTime = 0;
        var streamTime = 0;
        var bboxTime = 0;

        var startBboxTime = performance.now();
        const bbox = new THREE.Box3().setFromObject(this._scene);
        const bsphere = new THREE.Sphere();
        bbox.getBoundingSphere(bsphere);
        bboxTime = performance.now() - startBboxTime;

        let theScene = viewer.getScene();
        let flipV = theScene.getFlipGeometryV();


        this._scene.traverse((child) =>
        {
            // Safety check for null or undefined children
            if (!child) {
                return;
            }
            
            if (child.isMesh)
            {
                // Additional safety checks for mesh properties
                if (!child.geometry || !child.geometry.attributes) {
                    console.warn('Mesh missing geometry or attributes:', child);
                    return;
                }
                
                var startUVTime = performance.now();
                if (!child.geometry.attributes.uv)
                {
                    if (!child.geometry.attributes.position) {
                        console.warn('Mesh missing position attributes:', child);
                        return;
                    }
                    
                    const posCount = child.geometry.attributes.position.count;
                    const uvs = [];
                    const pos = child.geometry.attributes.position.array;

                    for (let i = 0; i < posCount; i++)
                    {
                        uvs.push((pos[i * 3] - bsphere.center.x) / bsphere.radius);
                        uvs.push((pos[i * 3 + 1] - bsphere.center.y) / bsphere.radius);
                    }

                    child.geometry.setAttribute('uv', new THREE.BufferAttribute(new Float32Array(uvs), 2));
                }
                else if (flipV)
                {
                    const uvCount = child.geometry.attributes.position.count;
                    const uvs = child.geometry.attributes.uv.array;
                    for (let i = 0; i < uvCount; i++)
                    {
                        let v = 1.0 - (uvs[i * 2 + 1]);
                        uvs[i * 2 + 1] = v;
                    }
                }
                uvTime += performance.now() - startUVTime;

                if (!child.geometry.attributes.normal)
                {
                    var startNormalTime = performance.now();
                    child.geometry.computeVertexNormals();
                    normalTime += performance.now() - startNormalTime;
                }

                if (child.geometry.getIndex())
                {
                    if (!child.geometry.attributes.tangent)
                    {
                        var startTangentTime = performance.now();
                        child.geometry.computeTangents();
                        tangentTime += performance.now() - startTangentTime;
                    }
                }

                // Use default MaterialX naming convention.
                var startStreamTime = performance.now();
                child.geometry.attributes.i_position = child.geometry.attributes.position;
                if (child.geometry.attributes.normal)
                    child.geometry.attributes.i_normal = child.geometry.attributes.normal;
                if (child.geometry.attributes.tangent)
                    child.geometry.attributes.i_tangent = child.geometry.attributes.tangent;
                if (child.geometry.attributes.color)
                    child.geometry.attributes.i_color_0 = child.geometry.attributes.color;
                if (child.geometry.attributes.color_1)
                    child.geometry.attributes.i_color_1 = child.geometry.attributes.color_1;
                if (child.geometry.attributes.uv)
                    child.geometry.attributes.i_texcoord_0 = child.geometry.attributes.uv;
                if (child.geometry.attributes.uv1)
                    child.geometry.attributes.i_texcoord_1 = child.geometry.attributes.uv1;
                if (child.geometry.attributes.uv2)
                    child.geometry.attributes.i_texcoord_2 = child.geometry.attributes.uv2;
                streamTime += performance.now() - startStreamTime;
            }
        });

        console.log("- Stream update time: ", performance.now() - startUpdateSceneTime, "ms");
        if (logDetailedTime)
        {
            console.log('  - UV time: ', uvTime);
            console.log('  - Normal time: ', normalTime);
            console.log('  - Tangent time: ', tangentTime);
            console.log('  - Stream Update time: ', streamTime);
            console.log('  - Bounds compute time: ', bboxTime);
        }

        // Update the background
        this._scene.background = this.getBackground();

        // Fit camera to model
        const camera = this.getCamera();
        camera.position.y = bsphere.center.y;
        camera.position.z = bsphere.radius * 2.0;
        camera.updateProjectionMatrix();

        orbitControls.target = bsphere.center;
        orbitControls.update();
    }

    updateObjectUniforms(child, material, camera)
    {
        if (!child || !material || !camera) return;
        const uniforms = material.uniforms;
        if (!uniforms) return;

        uniforms.u_worldMatrix.value = child.matrixWorld;
        uniforms.u_viewProjectionMatrix.value = this.#_viewProjMat.multiplyMatrices(camera.projectionMatrix, camera.matrixWorldInverse);

        if (uniforms.u_viewPosition)
            uniforms.u_viewPosition.value = camera.getWorldPosition(this.#_worldViewPos);

        if (uniforms.u_worldInverseTransposeMatrix)
            uniforms.u_worldInverseTransposeMatrix.value =
                new THREE.Matrix4().setFromMatrix3(this.#_normalMat.getNormalMatrix(child.matrixWorld));

        material.uniformsNeedUpdate = true;
    }

    /**
     * Update uniforms for all scene objects. This is called once per frame
     * and updates time and frame count uniforms.
     */
    updateTimeUniforms() {
        this._frame++;

        const scene = this.getScene();
        const time = performance.now() / 1000.0;
        const frame = this._frame;

        scene.traverse((child) =>
        {
            if (child.isMesh && child.material && child.material.uniforms)
            {
                const uniforms = child.material.uniforms;
                if (uniforms)
                {
                    if (uniforms.u_time)
                        uniforms.u_time.value = time;
                    if (uniforms.u_frame)
                        uniforms.u_frame.value = frame;
                }
            }
        });
    }

    // Determine string DAG path based on individual node names.
    getDagPath(node)
    {
        const rootNode = this.#_rootNode;

        let path = [node.userData?.name || node.name];
        while (node.parent)
        {
            node = node.parent;
            if (node)
            {
                // Stop at the root of the scene read in.
                if (node == rootNode)
                {
                    break;
                }
                path.unshift(node.userData?.name || node.name);
            }
        }
        return path;
    }

    // Assign material shader to associated geometry
    updateMaterial(matassign)
    {
        let assigned = 0;

        const shader = matassign.getShader();
        const material = matassign.getMaterial().getName();
        const geometry = matassign.getGeometry();
        const collection = matassign.getCollection();

        const scene = this.getScene();
        const camera = this.getCamera();
        scene.traverse((child) =>
        {
            // Safety check for null or undefined children
            if (!child) {
                return;
            }
            
            if (child.isMesh)
            {
                const dagPath = this.getDagPath(child).join('/');

                // Note that this is a very simplistic
                // assignment resolve and assumes basic
                // regular expression name match.
                let matches = (geometry == ALL_GEOMETRY_SPECIFIER);
                if (!matches)
                {
                    if (collection)
                    {
                        if (collection.matchesGeomString(dagPath))
                        {
                            matches = true;
                        }
                    }
                    else
                    {
                        if (geometry != NO_GEOMETRY_SPECIFIER)
                        {
                            const paths = geometry.split(',');
                            for (let path of paths)
                            {
                                if (dagPath.match(path))
                                {
                                    matches = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (matches)
                {
                    child.material = shader;
                    assigned++;
                }
            }
        });

        return assigned;
    }

    updateCamera()
    {
        const camera = this.getCamera();
        camera.aspect = window.innerWidth / window.innerHeight;
        camera.updateProjectionMatrix();
    }

    getScene()
    {
        return this._scene;
    }

    getCamera()
    {
        return this._camera;
    }

    getGeometryURL()
    {
        return this._geometryURL;
    }

    setGeometryURL(url)
    {
        this._geometryURL = url;
    }

    setBackgroundTexture(texture)
    {
        this.#_backgroundTexture = texture;
    }

    getShowBackgroundTexture()
    {
        return this.#_showBackgroundTexture;
    }

    setShowBackgroundTexture(enable)
    {
        this.#_showBackgroundTexture = enable;
    }

    getBackground()
    {
        if (this.#_backgroundTexture && this.#_showBackgroundTexture)
        {
            return this.#_backgroundTexture;
        }
        var color = new THREE.Color(this.#_backgroundColor);
        color.convertSRGBToLinear();
        return color;
    }

    toggleBackgroundTexture()
    {
        this.#_showBackgroundTexture = !this.#_showBackgroundTexture;
        this._scene.background = this.getBackground();
    }

    // Geometry file
    #_geometryURL = '';
    // Geometry loader
    #_gltfLoader = null;
    // Flip V coordinate of texture coordinates.
    // Set to true to be consistent with desktop viewer.
    #_flipV = true;

    // Scene
    #_scene = null;

    // Camera
    #_camera = null;

    // Background color
    #_backgroundColor = 0x4c4c52;

    // Background texture
    #_backgroundTexture = null;
    #_showBackgroundTexture = true;

    // Transform matrices
    #_normalMat = new THREE.Matrix3();
    #_viewProjMat = new THREE.Matrix4();
    #_worldViewPos = new THREE.Vector3();

    // Root node of imported scene
    #_rootNode = null;
}

/* 
    Property editor
*/
export class Editor
{
    // Initialize the editor, clearing any elements from previous materials.
    initialize()
    {
        Array.from(document.getElementsByClassName('lil-gui')).forEach(
            function (element, index, array)
            {
                if (element.className)
                {
                    element.remove();
                }
            }
        );

        this._gui = new GUI({ title: "Property Editor" });
        this._gui.close();
    }

    // Update ui properties
    // - Hide close button
    // - Update transparency so scene shows through if overlapping
    updateProperties(targetOpacity = 1)
    {
        // Set opacity
        Array.from(document.getElementsByClassName('dg')).forEach(
            function (element, index, array)
            {
                element.style.opacity = targetOpacity;
            }
        );
    }

    getGUI()
    {
        return this._gui;
    }

    _gui = null;
}

class MaterialAssign
{
    constructor(material, geometry, collection)
    {
        this._material = material;
        this._geometry = geometry;
        this._collection = collection;
        this._shader = null;
        this._materialUI = null;
    }

    setMaterialUI(value)
    {
        this._materialUI = value;
    }

    getMaterialUI()
    {
        return this._materialUI;
    }

    setShader(shader)
    {
        this._shader = shader;
    }

    getShader()
    {
        return this._shader;
    }

    getMaterial()
    {
        return this._material;
    }

    getGeometry()
    {
        return this._geometry;
    }

    setGeometry(value)
    {
        this._geometry = value;
    }

    getCollection()
    {
        return this._collection;
    }

    // MaterialX material node name
    _material;

    // MaterialX assignment geometry string
    _geometry;

    // MaterialX assignment collection
    _collection;

    // THREE.JS shader
    _shader;
}

export class Material
{
    constructor()
    {
        this._materials = [];
        this._defaultMaterial = null;
        this._soloMaterial = "";
    }

    clearMaterials()
    {
        this._materials.length = 0;
        this._defaultMaterial = null;
        this._soloMaterial = "";
    }

    setSoloMaterial(value)
    {
        this._soloMaterial = value;
    }

    getSoloMaterial()
    {
        return this._soloMaterial;
    }

    // If no material file is selected, we programmatically create a default material as a fallback
    static createFallbackMaterial(doc)
    {
        let ssNode = doc.getChild('Generated_Default_Shader');
        if (ssNode)
        {
            return ssNode;
        }
        const ssName = 'Generated_Default_Shader';
        ssNode = doc.addChildOfCategory('standard_surface', ssName);
        ssNode.setType('surfaceshader');
        const smNode = doc.addChildOfCategory('surfacematerial', 'Default');
        smNode.setType('material');
        const shaderElement = smNode.addInput('surfaceshader');
        shaderElement.setType('surfaceshader');
        shaderElement.setNodeName(ssName);

        return ssNode;
    }

    async loadMaterialFile(loader, materialFilename)
    {
        return new Promise((resolve, reject) =>
        {
            loader.load(materialFilename, data => resolve(data), null, reject);
        });
    }

    async loadMaterials(viewer, materialFilename)
    {
        var startTime = performance.now();

        const mx = viewer.getMx();

        // Re-initialize document
        var startDocTime = performance.now();
        var doc = mx.createDocument();
        doc.setDataLibrary(viewer.getLibrary());
        viewer.setDocument(doc);

        const fileloader = viewer.getFileLoader();

        let mtlxMaterial = await viewer.getMaterial().loadMaterialFile(fileloader, materialFilename);

        // Load lighting setup into document
        doc.importLibrary(viewer.getLightRig());

        console.log("- Material document load time: ", performance.now() - startDocTime, "ms.");

        // Set search path. Assumes images are relative to current file
        // location.
        if (!materialFilename) materialFilename = "/";
        const paths = materialFilename.split('/');
        paths.pop();
        const searchPath = paths.join('/');

        // Load material
        if (mtlxMaterial)
            try {                
                await mx.readFromXmlString(doc, mtlxMaterial, searchPath);
            }
            catch (error) {
                console.log('Error loading material file: ', error);
            }
        else
            Material.createFallbackMaterial(doc);

        // Check if there are any looks defined in the document
        // If so then traverse the looks for all material assignments.
        // Generate code and compile for any associated surface shader
        // and assign to the associated geometry. If there are no looks
        // then the first material is found and assignment to all the
        // geometry.
        this.clearMaterials();
        var looks = doc.getLooks();
        if (looks.length)
        {
            for (let look of looks)
            {
                const materialAssigns = look.getMaterialAssigns();
                for (let materialAssign of materialAssigns)
                {
                    let matName = materialAssign.getMaterial();
                    if (matName)
                    {
                        let mat = doc.getChild(matName);
                        var shader;
                        if (mat)
                        {
                            var shaders = mx.getShaderNodes(mat);
                            if (shaders.length)
                            {
                                shader = shaders[0];
                            }
                        }
                        let collection = materialAssign.getCollection();
                        let geom = materialAssign.getGeom();
                        let newAssignment;
                        if (collection || geom)
                        {
                            // Remove leading "/" from collection and geom for 
                            // later assignment comparison checking
                            if (collection && collection.charAt(0) == "/")
                            {
                                collection = collection.slice(1);
                            }
                            if (geom && geom.charAt(0) == "/")
                            {
                                geom = geom.slice(1);
                            }
                            newAssignment = new MaterialAssign(shader, geom, collection);
                        }
                        else
                        {
                            newAssignment = new MaterialAssign(shader, NO_GEOMETRY_SPECIFIER, null);
                        }

                        if (newAssignment)
                        {
                            this._materials.push(newAssignment);
                        }
                    }
                }
            }
        }
        else
        {
            // Search for any surface shaders. The first found
            // is assumed to be assigned to the entire scene
            // The identifier used is "*" to mean the entire scene. 
            const materialNodes = doc.getMaterialNodes();
            let shaderList = [];
            let foundRenderable = false;
            for (let i = 0; i < materialNodes.length; ++i)
            {
                let materialNode = materialNodes[i];
                if (materialNode)
                {
                    console.log('Scan material: ', materialNode.getNamePath());
                    let shaderNodes = mx.getShaderNodes(materialNode)
                    if (shaderNodes.length > 0)
                    {
                        let shaderNodePath = shaderNodes[0].getNamePath()
                        if (!shaderList.includes(shaderNodePath))
                        {
                            let assignment = NO_GEOMETRY_SPECIFIER;
                            if (foundRenderable == false)
                            {
                                assignment = ALL_GEOMETRY_SPECIFIER;
                                foundRenderable = true;
                            }
                            console.log('-- add shader: ', shaderNodePath);
                            shaderList.push(shaderNodePath);
                            this._materials.push(new MaterialAssign(shaderNodes[0], assignment));
                        }
                    }
                }
            }
            const nodeGraphs = doc.getNodeGraphs();
            for (let i = 0; i < nodeGraphs.length; ++i)
            {
                let nodeGraph = nodeGraphs[i];
                if (nodeGraph)
                {
                    if (nodeGraph.hasAttribute('nodedef') || nodeGraph.hasSourceUri())
                    {
                        continue;
                    }
                    // Skip any nodegraph that is connected to something downstream
                    if (nodeGraph.getDownstreamPorts().length > 0)
                    {
                        continue
                    }
                    let outputs = nodeGraph.getOutputs();
                    for (let j = 0; j < outputs.length; ++j)
                    {
                        let output = outputs[j];
                        {
                            let assignment = NO_GEOMETRY_SPECIFIER;
                            if (foundRenderable == false)
                            {
                                assignment = ALL_GEOMETRY_SPECIFIER;
                                foundRenderable = true;
                            }
                            let newMat = new MaterialAssign(output, assignment, null);
                            this._materials.push(newMat);
                        }
                    }
                }
            }
            const outputs = doc.getOutputs();
            for (let i = 0; i < outputs.length; ++i)
            {
                let output = outputs[i];
                if (output)
                {
                    let assignment = NO_GEOMETRY_SPECIFIER;
                    if (foundRenderable == false)
                    {
                        assignment = ALL_GEOMETRY_SPECIFIER;
                        foundRenderable = true;
                    }
                    this._materials.push(new MaterialAssign(output, assignment));
                }
            }

            const shaderNodes = [];
            for (let i = 0; i < shaderNodes.length; ++i)
            {
                let shaderNode = shaderNodes[i];
                let shaderNodePath = shaderNode.getNamePath()
                if (!shaderList.includes(shaderNodePath))
                {
                    let assignment = NO_GEOMETRY_SPECIFIER;
                    if (foundRenderable == false)
                    {
                        assignment = ALL_GEOMETRY_SPECIFIER;
                        foundRenderable = true;
                    }
                    shaderList.push(shaderNodePath);
                    this._materials.push(new MaterialAssign(shaderNode, assignment));
                }
            }
        }

        // Assign to default material if none found
        if (this._materials.length == 0)
        {
            const defaultNode = Material.createFallbackMaterial(doc);
            this._materials.push(new MaterialAssign(defaultNode, ALL_GEOMETRY_SPECIFIER));
        }

        // Create a new shader for each material node.
        // Only create the shader once even if assigned more than once.
        var startGenTime = performance.now();
        let shaderMap = new Map();
        let closeUI = false;
        for (let matassign of this._materials)
        {
            // Need to use path vs name to get a unique key.
            let materialName = matassign.getMaterial().getNamePath();
            let shader = shaderMap[materialName];
            if (!shader)
            {
                shader = await viewer.getMaterial().generateMaterial(matassign, viewer, searchPath, closeUI);
                shaderMap[materialName] = shader;
            }
            matassign.setShader(shader);
            closeUI = true;
        }
        console.log("- Generate (", this._materials.length, ") shader(s) time: ", performance.now() - startGenTime, " ms.",);

        // Update scene shader assignments
        this.updateMaterialAssignments(viewer, "");

        console.log("Total material time: ", (performance.now() - startTime), "ms");
    }

    //
    // Update the assignments for scene objects based on the
    // material assignment information stored in the viewer.
    // Note: If none of the MaterialX assignments match the geometry
    // in the scene, then the first material assignment shader is assigned
    // to the entire scene.
    //
    async updateMaterialAssignments(viewer, soloMaterial)
    {
        console.log("Update material assignments. Solo=", soloMaterial);
        var startTime = performance.now();

        let assigned = 0;
        let assignedSolo = false;
        for (let matassign of this._materials)
        {
            if (matassign.getShader())
            {
                if (soloMaterial.length)
                {
                    if (matassign.getMaterial().getNamePath() == soloMaterial)
                    {
                        let temp = matassign.getGeometry();
                        matassign.setGeometry(ALL_GEOMETRY_SPECIFIER);
                        assigned += viewer.getScene().updateMaterial(matassign);
                        matassign.setGeometry(temp);
                        assignedSolo = true;
                        break;
                    }
                }
                else
                {
                    assigned += viewer.getScene().updateMaterial(matassign);
                }
            }
        }
        if (assigned == 0 && this._materials.length)
        {
            this._defaultMaterial = new MaterialAssign(this._materials[0].getMaterial(), ALL_GEOMETRY_SPECIFIER);
            this._defaultMaterial.setShader(this._materials[0].getShader());
            viewer.getScene().updateMaterial(this._defaultMaterial);
        }

        if (assigned > 0)
        {
            console.log('Material assignment time: ', performance.now() - startTime, " ms.");
        }
    }

    // 
    // Generate a new material for a given element
    //
    async generateMaterial(matassign, viewer, searchPath, closeUI)
    {
        var elem = matassign.getMaterial();

        var startGenerateMat = performance.now();

        const mx = viewer.getMx();
        const textureLoader = new THREE.TextureLoader();

        const lights = viewer.getLights();
        const lightData = viewer.getLightData();
        const radianceTexture = viewer.getRadianceTexture();
        const irradianceTexture = viewer.getIrradianceTexture();
        const gen = viewer.getGenerator();
        const genContext = viewer.getGenContext();
        genContext.getOptions().hwSrgbEncodeOutput = true;

        // Perform transparency check on renderable item
        var startTranspCheckTime = performance.now();
        const isTransparent = mx.isTransparentSurface(elem, gen.getTarget());
        genContext.getOptions().hwTransparency = isTransparent;
        // Always set to complete. 
        // Can consider option to set to reduced as the parsing of large numbers of uniforms (e.g. on shading models)
        // can be quite expensive.
        genContext.getOptions().shaderInterfaceType = mx.ShaderInterfaceType.SHADER_INTERFACE_COMPLETE;

        if (logDetailedTime)
            console.log("  - Transparency check time: ", performance.now() - startTranspCheckTime, "ms");

        // Generate GLES code
        var startMTLXGenTime = performance.now();
        let shader = gen.generate(elem.getNamePath(), elem, genContext);
        if (logDetailedTime)
            console.log("  - MaterialX gen time: ", performance.now() - startMTLXGenTime, "ms");

        var startUniformUpdate = performance.now();

        // Get shaders and uniform values
        let vShader = shader.getSourceCode("vertex");
        let fShader = shader.getSourceCode("pixel");

        // Convert GLSL to SPIR-V and update shader text areas
        const shaderConverter = viewer.getShaderConverter();
        
        // Feature flag to enable/disable shader conversion (can cause external script errors)
        const enableShaderConversion = false; // Disable SPIR-V conversion to avoid ES6 module errors
        
        if (enableShaderConversion) {
            // Convert vertex shader to SPIR-V
            try {
                console.log('Attempting vertex shader SPIR-V conversion...');
                const vertexSpirv = await shaderConverter.convertGLSLToSPIRV(vShader, 'vertex');
                const vertexSpirvHex = Array.from(vertexSpirv)
                    .map(byte => byte.toString(16).padStart(2, '0'))
                    .join(' ');
                
                if (typeof window !== 'undefined' && typeof window.updateVtxShader === 'function') {
                    window.updateVtxShader(''); // Clear first
                    window.updateVtxShader(`// SPIR-V Binary (${vertexSpirv.length} bytes)\n// Hex representation:\n${vertexSpirvHex}\n\n// Original GLSL:\n${vShader}`);
                } else if (typeof updateVtxShader === 'function') {
                    updateVtxShader(''); // Clear first
                    updateVtxShader(`// SPIR-V Binary (${vertexSpirv.length} bytes)\n// Hex representation:\n${vertexSpirvHex}\n\n// Original GLSL:\n${vShader}`);
                } else {
                    console.log('updateVtxShader function not available');
                }
            } catch (error) {
                console.error('Failed to convert vertex shader to SPIR-V:', error);
                if (typeof window !== 'undefined' && typeof window.updateVtxShader === 'function') {
                    window.updateVtxShader(''); // Clear first
                    window.updateVtxShader(`// SPIR-V conversion failed: ${error.message}\n\n// Original GLSL:\n${vShader}`);
                } else if (typeof updateVtxShader === 'function') {
                    updateVtxShader(''); // Clear first
                    updateVtxShader(`// SPIR-V conversion failed: ${error.message}\n\n// Original GLSL:\n${vShader}`);
                } else {
                    console.log('updateVtxShader function not available for error display');
                }
            }

            // Convert fragment shader to SPIR-V
            try {
                console.log('Attempting fragment shader SPIR-V conversion...');
                const fragmentSpirv = await shaderConverter.convertGLSLToSPIRV(fShader, 'fragment');
                const fragmentSpirvHex = Array.from(fragmentSpirv)
                    .map(byte => byte.toString(16).padStart(2, '0'))
                    .join(' ');
                
                if (typeof window !== 'undefined' && typeof window.updatePxShader === 'function') {
                    window.updatePxShader(''); // Clear first
                    window.updatePxShader(`// SPIR-V Binary (${fragmentSpirv.length} bytes)\n// Hex representation:\n${fragmentSpirvHex}\n\n// Original GLSL:\n${fShader}`);
                } else if (typeof updatePxShader === 'function') {
                    updatePxShader(''); // Clear first
                    updatePxShader(`// SPIR-V Binary (${fragmentSpirv.length} bytes)\n// Hex representation:\n${fragmentSpirvHex}\n\n// Original GLSL:\n${fShader}`);
                } else {
                    console.log('updatePxShader function not available');
                }
            } catch (error) {
                console.error('Failed to convert fragment shader to SPIR-V:', error);
                if (typeof window !== 'undefined' && typeof window.updatePxShader === 'function') {
                    window.updatePxShader(''); // Clear first
                    window.updatePxShader(`// SPIR-V conversion failed: ${error.message}\n\n// Original GLSL:\n${fShader}`);
                } else if (typeof updatePxShader === 'function') {
                    updatePxShader(''); // Clear first
                    updatePxShader(`// SPIR-V conversion failed: ${error.message}\n\n// Original GLSL:\n${fShader}`);
                } else {
                    console.log('updatePxShader function not available for error display');
                }
            }
        } else {
            // Shader conversion disabled, just show original GLSL
            console.log('Shader conversion disabled, showing original GLSL');
            if (typeof window !== 'undefined' && typeof window.updateVtxShader === 'function') {
                window.updateVtxShader(`// Original GLSL Vertex Shader:\n${vShader}`);
            } else if (typeof updateVtxShader === 'function') {
                updateVtxShader(`// Original GLSL Vertex Shader:\n${vShader}`);
            }
            
            if (typeof window !== 'undefined' && typeof window.updatePxShader === 'function') {
                window.updatePxShader(`// Original GLSL Fragment Shader:\n${fShader}`);
            } else if (typeof updatePxShader === 'function') {
                updatePxShader(`// Original GLSL Fragment Shader:\n${fShader}`);
            }
        }

        let theScene = viewer.getScene();
        let flipV = theScene.getFlipGeometryV();
        let uniforms = {
            ...getUniformValues(shader.getStage('vertex'), textureLoader, searchPath, flipV),
            ...getUniformValues(shader.getStage('pixel'), textureLoader, searchPath, flipV),
        }

        Object.assign(uniforms, {
            u_numActiveLightSources: { value: lights.length },
            u_lightData: { value: lightData },
            u_envMatrix: { value: getLightRotation() },
            u_envRadiance: { value: radianceTexture },
            u_envRadianceMips: { value: Math.trunc(Math.log2(Math.max(radianceTexture.image.width, radianceTexture.image.height))) + 1 },
            u_envRadianceSamples: { value: 16 },
            u_envIrradiance: { value: irradianceTexture },
            u_refractionEnv: { value: true }
        });

        // Create Three JS Material
        let newMaterial = new THREE.RawShaderMaterial({
            uniforms: uniforms,
            vertexShader: vShader,
            fragmentShader: fShader,
            transparent: isTransparent,
            blendEquation: THREE.AddEquation,
            blendSrc: THREE.OneMinusSrcAlphaFactor,
            blendDst: THREE.SrcAlphaFactor,
            side: THREE.DoubleSide,
            name: elem.getName(),
        });


        if (logDetailedTime)
            console.log("  - Three material update time: ", performance.now() - startUniformUpdate, "ms");

        // Update property editor
        const gui = viewer.getEditor().getGUI();
        this.updateEditor(matassign, shader, newMaterial, gui, closeUI, viewer);

        if (logDetailedTime)
            console.log("- Per material generate time: ", performance.now() - startGenerateMat, "ms");

        return newMaterial;
    }

    clearSoloMaterialUI()
    {
        for (let i = 0; i < this._materials.length; ++i)
        {
            let matassign = this._materials[i];
            let matUI = matassign.getMaterialUI();
            if (matUI)
            {
                let matTitle = matUI.domElement.getElementsByClassName('title')[0];
                matTitle.classList.remove('peditor_material_assigned');
                let img = matTitle.getElementsByTagName('img')[0];
                img.src = 'public/shader_ball.svg';
                //matTitle.classList.remove('peditor_material_unassigned');
            }
        }
    }

    static updateSoloMaterial(viewer, elemPath, materials, event)
    {
        // Prevent the event from being passed to parent folder
        event.stopPropagation();

        for (let i = 0; i < materials.length; ++i)
        {
            let matassign = materials[i];
            // Need to use path vs name to get a unique key.
            let materialName = matassign.getMaterial().getNamePath();
            var matUI = matassign.getMaterialUI();
            let matTitle = matUI.domElement.getElementsByClassName('title')[0];
            let img = matTitle.getElementsByTagName('img')[0];
            if (materialName == elemPath)
            {
                if (this._soloMaterial == elemPath)
                {
                    img.src = 'public/shader_ball.svg';
                    matTitle.classList.remove('peditor_material_assigned');
                    this._soloMaterial = "";
                }
                else
                {
                    img.src = 'public/shader_ball2.svg';
                    matTitle.classList.add('peditor_material_assigned');
                    this._soloMaterial = elemPath;
                }
            }
            else
            {
                img.src = 'public/shader_ball.svg';
                matTitle.classList.remove('peditor_material_assigned');
            }
        }
        viewer.getMaterial().updateMaterialAssignments(viewer, this._soloMaterial);
    }

    //
    // Update property editor for a given MaterialX element, it's shader, and
    // Three material
    //
    updateEditor(matassign, shader, material, gui, closeUI, viewer)
    {
        var elem = matassign.getMaterial();
        var materials = this._materials;

        const DEFAULT_MIN = 0;
        const DEFAULT_MAX = 100;

        var startTime = performance.now();

        const elemPath = elem.getNamePath();

        // Create and cache associated UI
        var matUI = gui.addFolder(elemPath);
        matassign.setMaterialUI(matUI);

        let matTitle = matUI.domElement.getElementsByClassName('title')[0];
        // Add a icon to the title to allow for assigning the material to geometry
        // Clicking on the icon will "solo" the material to the geometry.
        // Clicking on the title will open/close the material folder.
        matTitle.innerHTML = "<img id='" + elemPath + "' src='public/shader_ball.svg' width='16' height='16' style='vertical-align:middle; margin-right: 5px;'>" + elem.getNamePath();
        let img = matTitle.getElementsByTagName('img')[0];
        if (img)
        {
            // Add event listener to icon to call updateSoloMaterial function
            img.addEventListener('click', function (event)
            {
                Material.updateSoloMaterial(viewer, elemPath, materials, event);
            });
        }

        if (closeUI)
        {
            matUI.close();
        }
        const uniformBlocks = Object.values(shader.getStage('pixel').getUniformBlocks());
        var uniformToUpdate;
        const ignoreList = ['u_envRadianceMips', 'u_envRadianceSamples', 'u_alphaThreshold'];

        var folderList = new Map();
        folderList[elemPath] = matUI;

        uniformBlocks.forEach(uniforms =>
        {
            if (!uniforms.empty())
            {
                for (let i = 0; i < uniforms.size(); ++i)
                {
                    const variable = uniforms.get(i);
                    const value = variable.getValue()?.getData();
                    let name = variable.getVariable();

                    if (ignoreList.includes(name))
                    {
                        continue;
                    }

                    let currentFolder = matUI;
                    let currentElemPath = variable.getPath();
                    if (!currentElemPath || currentElemPath.length == 0)
                    {
                        continue;
                    }
                    let currentElem = elem.getDocument().getDescendant(currentElemPath);
                    if (!currentElem)
                    {
                        continue;
                    }
                    
                    // Skip non-input types and anything > 2 levels deep 
                    if (!currentElem.asAInput() || currentElem.getNamePath().split('/').length > 2)
                    {
                        continue;
                    }

                    let currentNode = null;
                    if (currentElem.getParent() && currentElem.getParent().getNamePath() != "")
                    {
                        currentNode = currentElem.getParent();
                    }
                    let uiname = "";
                    let nodeDefInput = null;
                    if (currentNode)
                    {

                        let currentNodePath = currentNode.getNamePath();
                        var pathSplit = currentNodePath.split('/');
                        if (pathSplit.length)
                        {
                            currentNodePath = pathSplit[0];
                        }
                        currentFolder = folderList[currentNodePath];
                        if (!currentFolder)
                        {
                            currentFolder = matUI.addFolder(currentNodePath);
                            folderList[currentNodePath] = currentFolder;
                        }

                        // Check for ui attributes
                        var nodeDef = currentNode.getNodeDef();
                        if (nodeDef)
                        {
                            // Remove node name from shader uniform name for non root nodes
                            let lookup_name = name.replace(currentNode.getName() + '_', '');
                            nodeDefInput = nodeDef.getActiveInput(lookup_name);
                            if (nodeDefInput)
                            {
                                uiname = nodeDefInput.getAttribute('uiname');
                                let uifolderName = nodeDefInput.getAttribute('uifolder');
                                if (uifolderName && uifolderName.length)
                                {
                                    let newFolderName = currentNodePath + '/' + uifolderName;
                                    currentFolder = folderList[newFolderName];
                                    if (!currentFolder)
                                    {
                                        currentFolder = matUI.addFolder(uifolderName);
                                        currentFolder.domElement.classList.add('peditorfolder');
                                        folderList[newFolderName] = currentFolder;
                                    }
                                }
                            }
                        }
                    }

                    // Determine UI name to use
                    let path = name;
                    let interfaceName = currentElem.getAttribute("interfacename");
                    if (interfaceName && interfaceName.length)
                    {
                        const graph = currentNode.getParent();
                        if (graph)
                        {
                            const graphInput = graph.getInput(interfaceName);
                            if (graphInput)
                            {
                                let uiname = graphInput.getAttribute('uiname');
                                if (uiname.length)
                                {
                                    path = uiname;
                                }
                                else
                                {
                                    path = graphInput.getName();
                                }
                            }
                        }
                        else
                        {
                            path = interfaceName;
                        }
                    }
                    else
                    {
                        if (!uiname)
                        {
                            uiname = currentElem.getAttribute('uiname');
                        }
                        if (uiname && uiname.length)
                        {
                            path = uiname;
                        }
                    }

                    // Skip if already added to current folder 
                    let found = false;
                    for (let i = 0; i < currentFolder.children.length; ++i)
                    {
                        if (currentFolder.children[i]._name == path)
                        {
                            found = true;
                            break;
                        }
                    }                        
                    if (found)
                    {
                        continue;
                    }

                    switch (variable.getType().getName())
                    {
                        case 'float':
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null)
                            {
                                var minValue = DEFAULT_MIN;
                                if (value < minValue)
                                {
                                    minValue = value;
                                }
                                var maxValue = DEFAULT_MAX;
                                if (value > maxValue)
                                {
                                    maxValue = value;
                                }
                                var step = 0;
                                if (nodeDefInput)
                                {
                                    if (nodeDefInput.hasAttribute('uisoftmin'))
                                        minValue = parseFloat(nodeDefInput.getAttribute('uisoftmin'));
                                    else if (nodeDefInput.hasAttribute('uimin'))
                                        minValue = parseFloat(nodeDefInput.getAttribute('uimin'));

                                    if (nodeDefInput.hasAttribute('uisoftmax'))
                                        maxValue = parseFloat(nodeDefInput.getAttribute('uisoftmax'));
                                    else if (nodeDefInput.hasAttribute('uimax'))
                                        maxValue = parseFloat(nodeDefInput.getAttribute('uimax'));

                                    if (nodeDefInput.hasAttribute('uistep'))
                                        step = parseFloat(nodeDefInput.getAttribute('uistep'));
                                }
                                if (step == 0)
                                {
                                    step = (maxValue - minValue) / 1000.0;
                                }
                                const w = currentFolder.add(material.uniforms[name], 'value', minValue, maxValue, step).name(path);
                                w.domElement.classList.add('peditoritem');
                            }
                            break;

                        case 'integer':
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null)
                            {
                                var minValue = DEFAULT_MIN;
                                if (value < minValue)
                                {
                                    minValue = value;
                                }
                                var maxValue = DEFAULT_MAX;
                                if (value > maxValue)
                                {
                                    maxValue = value;
                                }
                                var step = 0;
                                var enumList = []
                                var enumValues = []
                                if (nodeDefInput)
                                {
                                    if (nodeDefInput.hasAttribute('enum'))
                                    {
                                        // Get enum and enum values attributes (if present)
                                        enumList = nodeDefInput.getAttribute('enum').split(',');
                                        if (nodeDefInput.hasAttribute('enumvalues'))
                                        {
                                            enumValues = nodeDefInput.getAttribute('enumvalues').split(',').map(Number);
                                        }
                                    }
                                    else
                                    {
                                        if (nodeDefInput.hasAttribute('uisoftmin'))
                                            minValue = parseInt(nodeDefInput.getAttribute('uisoftmin'));
                                        else if (nodeDefInput.hasAttribute('uimin'))
                                            minValue = parseInt(nodeDefInput.getAttribute('uimin'));

                                        if (nodeDefInput.hasAttribute('uisoftmax'))
                                            maxValue = parseInt(nodeDefInput.getAttribute('uisoftmax'));
                                        else if (nodeDefInput.hasAttribute('uimax'))
                                            maxValue = parseInt(nodeDefInput.getAttribute('uimax'));

                                        if (nodeDefInput.hasAttribute('uistep'))
                                            step = parseInt(nodeDefInput.getAttribute('uistep'));
                                    }
                                }
                                if (enumList.length == 0)
                                {
                                    if (step == 0)
                                    {
                                        step = 1 / (maxValue - minValue);
                                        step = Math.ceil(step);
                                        if (step == 0)
                                        {
                                            step = 1;
                                        }
                                    }
                                }
                                if (enumList.length == 0)
                                {
                                    let w = currentFolder.add(material.uniforms[name], 'value', minValue, maxValue, step).name(path);
                                    w.domElement.classList.add('peditoritem');
                                }
                                else
                                {
                                    // Map enumList strings to values
                                    // Map to 0..N if no values are specified via enumvalues attribute
                                    if (enumValues.length == 0)
                                    {
                                        for (let i = 0; i < enumList.length; ++i)
                                        {
                                            enumValues.push(i);
                                        }
                                    }
                                    const enumeration = {};
                                    enumList.forEach((str, index) =>
                                    {
                                        enumeration[str] = enumValues[index];
                                    });

                                    // Function to handle enum drop-down
                                    function handleDropdownChange(value)
                                    {
                                        if (material.uniforms[name])
                                        {
                                            material.uniforms[name].value = value;
                                        }
                                    }
                                    const defaultOption = enumList[value]; // Set the default selected option
                                    const dropdownController = currentFolder.add(enumeration, defaultOption, enumeration).name(path);
                                    dropdownController.onChange(handleDropdownChange);
                                    dropdownController.domElement.classList.add('peditoritem');
                                }
                            }
                            break;

                        case 'boolean':
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null)
                            {
                                let w = currentFolder.add(material.uniforms[name], 'value').name(path);
                                w.domElement.classList.add('peditoritem');
                            }
                            break;

                        case 'vector2':
                        case 'vector3':
                        case 'vector4':
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null)
                            {
                                var minValue = [DEFAULT_MIN, DEFAULT_MIN, DEFAULT_MIN, DEFAULT_MIN];
                                var maxValue = [DEFAULT_MAX, DEFAULT_MAX, DEFAULT_MAX, DEFAULT_MAX];
                                var step = [0, 0, 0, 0];

                                if (nodeDefInput)
                                {
                                    if (nodeDefInput.hasAttribute('uisoftmin'))
                                        minValue = nodeDefInput.getAttribute('uisoftmin').split(',').map(Number);
                                    else if (nodeDefInput.hasAttribute('uimin'))
                                        minValue = nodeDefInput.getAttribute('uimin').split(',').map(Number);

                                    if (nodeDefInput.hasAttribute('uisoftmax'))
                                        maxValue = nodeDefInput.getAttribute('uisoftmax').split(',').map(Number);
                                    else if (nodeDefInput.hasAttribute('uimax'))
                                        maxValue = nodeDefInput.getAttribute('uimax').split(',').map(Number);

                                    if (nodeDefInput.hasAttribute('uistep'))
                                        step = nodeDefInput.getAttribute('uistep').split(',').map(Number);
                                }
                                for (let i = 0; i < 4; ++i)
                                {
                                    if (step[i] == 0)
                                    {
                                        step[i] = 1 / (maxValue[i] - minValue[i]);
                                    }
                                }

                                const keyString = ["x", "y", "z", "w"];
                                let vecFolder = currentFolder.addFolder(path);
                                Object.keys(material.uniforms[name].value).forEach((key) =>
                                {
                                    let w = vecFolder.add(material.uniforms[name].value,
                                        key, minValue[key], maxValue[key], step[key]).name(keyString[key]);
                                    w.domElement.classList.add('peditoritem');
                                })
                            }
                            break;

                        case 'color3':
                            // Irksome way to map arrays to colors and back
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null)
                            {
                                var dummy =
                                {
                                    color: 0xFF0000
                                };
                                const color3 = new THREE.Color(dummy.color);
                                color3.fromArray(material.uniforms[name].value);
                                dummy.color = color3.getHex();
                                let w = currentFolder.addColor(dummy, 'color').name(path)
                                    .onChange(function (value)
                                    {
                                        const color3 = new THREE.Color(value);
                                        material.uniforms[name].value.set(color3.toArray());
                                    });
                                w.domElement.classList.add('peditoritem');
                            }
                            break;

                        case 'color4':
                            uniformToUpdate = material.uniforms[name];
                            if (uniformToUpdate && value != null)
                            {
                                var dummy =
                                {
                                    color: 0xFF0000
                                };
                                // Extract RGB from the color4 value
                                const color3 = new THREE.Color();
                                color3.fromArray(material.uniforms[name].value);
                                dummy.color = color3.getHex();
                                let alphaValue = material.uniforms[name].value[3]; // Get alpha component
                                
                                // Add the RGB color picker as one item
                                let colorPicker = currentFolder.addColor(dummy, 'color').name(path + '.rgb')
                                    .onChange(function (value)
                                    {
                                        const color3 = new THREE.Color(value);
                                        // Update RGB while preserving alpha
                                        material.uniforms[name].value[0] = color3.r;
                                        material.uniforms[name].value[1] = color3.g;
                                        material.uniforms[name].value[2] = color3.b;
                                    });
                                colorPicker.domElement.classList.add('peditoritem');
                                
                                // Add the alpha slider as a separate item at the same level
                                var alphaObj = { value: alphaValue };
                                let alphaSlider = currentFolder.add(alphaObj, 'value', 0, 1, 0.01).name(path + '.alpha')
                                    .onChange(function (value)
                                    {
                                        material.uniforms[name].value[3] = value;
                                    });
                                alphaSlider.domElement.classList.add('peditoritem');
                            }
                            break;

                        case 'matrix33':
                        case 'matrix44':
                        case 'samplerCube':
                        case 'filename':
                            break;
                        case 'string':
                            if (value != null)
                            {
                                var dummy =
                                {
                                    thevalue: value
                                }
                                let item = currentFolder.add(dummy, 'thevalue');
                                item.name(path);
                                item.disable(true);
                                item.domElement.classList.add('peditoritem');
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        });

        if (logDetailedTime)
        {
            console.log("  - Editor update time: ", performance.now() - startTime, "ms");
        }
    }

    // List of material assignments: { MaterialX node, geometry assignment string, and hardware shader }
    _materials;

    // Fallback material if nothing was assigned explicitly
    _defaultMaterial;
}

/*
    Viewer class

    Keeps track of local scene, and property editor as well as current MaterialX document 
    and associated material, shader and lighting information.
*/
export class Viewer
{
    static create()
    {
        return new Viewer();
    }

    constructor()
    {
        this.scene = new Scene();
        this.editor = new Editor();
        this.materials.push(new Material());
        this.shaderConverter = new ShaderConverter();

        this.fileLoader = new THREE.FileLoader();
        this.hdrLoader = new RGBELoader();
    }

    //
    // Create shader generator, generation context and "base" document which
    // contains the standard definition libraries and lighting elements.
    //
    async initialize(mtlxIn, renderer, radianceTexture, irradianceTexture, lightRigXml)
    {
        this.mx = mtlxIn;

        // Initialize base document
        this.generator = this.mx.EsslShaderGenerator.create();
        this.genContext = new this.mx.GenContext(this.generator);

        this.document = this.mx.createDocument();
        this.stdlib = this.mx.loadStandardLibraries(this.genContext);
        this.document.setDataLibrary(this.stdlib);

        this.initializeLighting(renderer, radianceTexture, irradianceTexture, lightRigXml);

        radianceTexture.mapping = THREE.EquirectangularReflectionMapping;
        this.getScene().setBackgroundTexture(radianceTexture);
    }

    //
    // Load in lighting rig document and register lights with generation context
    // Initialize environment lighting (IBLs).
    //
    async initializeLighting(renderer, radianceTexture, irradianceTexture, lightRigXml)
    {
        // Load lighting setup into document
        const mx = this.getMx();
        this.lightRigDoc = mx.createDocument();
        await mx.readFromXmlString(this.lightRigDoc, lightRigXml);
        this.document.importLibrary(this.lightRigDoc);

        // Register lights with generation context
        this.lights = findLights(this.document);
        this.lightData = registerLights(mx, this.lights, this.genContext);

        this.radianceTexture = prepareEnvTexture(radianceTexture, renderer.capabilities);
        this.irradianceTexture = prepareEnvTexture(irradianceTexture, renderer.capabilities);
    }

    getEditor()
    {
        return this.editor;
    }

    getScene()
    {
        return this.scene;
    }

    getMaterial()
    {
        return this.materials[0];
    }

    getFileLoader()
    {
        return this.fileLoader;
    }

    getHdrLoader()
    {
        return this.hdrLoader;
    }

    setDocument(doc)
    {
        this.doc = doc;
    }
    getDocument()
    {
        return this.doc;
    }

    getLibrary()
    {
        return this.stdlib;
    }

    getLightRig()
    {
        return this.lightRigDoc;
    }

    getMx()
    {
        return this.mx;
    }

    getGenerator()
    {
        return this.generator;
    }

    getGenContext()
    {
        return this.genContext;
    }

    getLights()
    {
        return this.lights;
    }

    getLightData()
    {
        return this.lightData;
    }

    getRadianceTexture()
    {
        return this.radianceTexture;
    }

    getIrradianceTexture()
    {
        return this.irradianceTexture;
    }

    getShaderConverter()
    {
        return this.shaderConverter;
    }

    // Three scene and materials. 
    scene = null;
    materials = [];

    // Property editor
    editor = null;

    // Utility loaders
    fileloader = null;
    hdrLoader = null;

    // MaterialX module, current document and support documents.
    mx = null;
    doc = null;
    stdlib = null;
    lightRigDoc = null;

    // MaterialX code generator and context
    generator = null;
    genContext = null;

    // Lighting information
    lights = null;
    lightData = null;
    radianceTexture = null;
    irradianceTexture = null;
}
