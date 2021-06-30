import * as THREE from 'three';
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader.js';
import { RGBELoader } from 'three/examples/jsm/loaders/RGBELoader.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'three/examples/jsm/postprocessing/ShaderPass.js';
import { GammaCorrectionShader } from 'three/examples/jsm/shaders/GammaCorrectionShader.js';

import { generateTangents, prepareEnvTexture, toThreeUniforms } from './helper.js'

let camera, scene, model, renderer, composer, controls, mx;

let normalMat = new THREE.Matrix3();
let viewProjMat = new THREE.Matrix4();
let worldViewPos = new THREE.Vector3();

const materialFilename = new URLSearchParams(document.location.search).get("file");

init();


// If no material file is selected, we programmatically create a jade material as a fallback
function fallbackMaterial(mx, doc) {
    const ssName = 'SR_jade';
    const ssNode = doc.addChildOfCategory('standard_surface', ssName);
    ssNode.setType('surfaceshader');
    ssNode.addInput('base').setValueFloat(0.5);
    ssNode.addInput('base_color').setValueColor3(new mx.Color3(0.0603, 0.4398, 0.1916));
    ssNode.addInput('specular_roughness').setValueFloat(0.25);
    ssNode.addInput('specular_IOR').setValueFloat(2.418);
    ssNode.addInput('specular_anisotropy').setValueFloat(0.5);
    ssNode.addInput('subsurface').setValueFloat(0.4);
    ssNode.addInput('subsurface_color').setValueColor3(new mx.Color3(0.0603, 0.4398, 0.1916));
    const smNode = doc.addChildOfCategory('surfacematerial', 'Jade');
    smNode.setType('material');
    const shaderElement = smNode.addInput('surfaceshader');
    shaderElement.setType('surfaceshader');
    shaderElement.setNodeName(ssName);
}

function init() {
    let canvas = document.getElementById('webglcanvas');
    let context = canvas.getContext('webgl2');

    camera = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 1, 100);

    // Set up scene
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x4c4c52);
    scene.background.convertSRGBToLinear();

    scene.add(new THREE.AmbientLight( 0x222222));
    const directionalLight = new THREE.DirectionalLight(new THREE.Color(1, 0.894474, 0.567234), 2.52776);
    directionalLight.position.set(-1, 1, 1).normalize();
    scene.add(directionalLight);
    const lightData = {
      type: 1,
      direction: directionalLight.position.negate(), 
      color: new THREE.Vector3(...directionalLight.color.toArray()), 
      intensity: directionalLight.intensity
    };

    renderer = new THREE.WebGLRenderer({canvas, context});
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);

    composer = new EffectComposer( renderer );
    const renderPass = new RenderPass( scene, camera );
    composer.addPass( renderPass );
    const gammaCorrectionPass = new ShaderPass( GammaCorrectionShader );
    composer.addPass( gammaCorrectionPass );

    window.addEventListener('resize', onWindowResize);

    // controls
    controls = new OrbitControls(camera, renderer.domElement);

    // Load model and shaders
    var fileloader = new THREE.FileLoader();
    const objLoader = new OBJLoader();
    const hdrloader = new RGBELoader();
    const textureLoader = new THREE.TextureLoader();

    Promise.all([
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('Lights/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => hdrloader.setDataType(THREE.FloatType).load('Lights/irradiance/san_giuseppe_bridge_split.hdr', resolve)),
        new Promise(resolve => objLoader.load('Geometry/shaderball.obj', resolve)),
        new Promise(function (resolve) { MaterialX().then((module) => { resolve(module); }); }),
        new Promise(resolve => materialFilename ? fileloader.load(materialFilename, resolve) : resolve())
    ]).then(([loadedRadianceTexture, loadedIrradianceTexture, obj, mxIn, mtlxMaterial]) => {
        // Initialize MaterialX and the shader generation context
        mx = mxIn;
        let doc = mx.createDocument();
        let gen = new mx.EsslShaderGenerator();
        let genContext = new mx.GenContext(gen);
        let stdlib = mx.loadStandardLibraries(genContext);
        doc.importLibrary(stdlib);        

        // Load material
        if (mtlxMaterial)
            mx.readFromXmlString(doc, mtlxMaterial);
        else
            fallbackMaterial(mx, doc);

        let elem = mx.findRenderableElement(doc);
        let shader = gen.generate(elem.getNamePath(), elem, genContext);

        // Get GL ES shaders and uniform values
        let fShader = shader.getSourceCode("pixel");       
        let vShader = shader.getSourceCode("vertex");
        let uniforms = {
          ...toThreeUniforms(JSON.parse(shader.getUniformValues("vertex")), textureLoader),
          ...toThreeUniforms(JSON.parse(shader.getUniformValues("pixel")), textureLoader)
        }

        const radianceTexture = prepareEnvTexture(loadedRadianceTexture, renderer.capabilities);
        const irradianceTexture = prepareEnvTexture(loadedIrradianceTexture, renderer.capabilities);

        Object.assign(uniforms, {
          time: { value: 0.0 },
          u_numActiveLightSources: {value: 1},
          u_lightData: {value: [ lightData ]},

          u_envMatrix: {value: new THREE.Matrix4().makeRotationY(Math.PI)},
          u_envRadiance: {value: radianceTexture},
          u_envRadianceMips: {value: Math.trunc(Math.log2(Math.max(radianceTexture.image.width, radianceTexture.image.height))) + 1},
          u_envRadianceSamples: {value: 16},
          u_envIrradiance: {value: irradianceTexture}
        });

        // Create Three JS Material
        const threeMaterial = new THREE.RawShaderMaterial({
          uniforms: uniforms,
          vertexShader: vShader,
          fragmentShader: fShader,
        });
        obj.traverse((child) => {
            if (child.isMesh) {
              generateTangents(child.geometry);
              child.geometry.attributes.uv_0 = child.geometry.attributes.uv
              child.material = threeMaterial;
            }
        });
        model = obj;
        scene.add(model);

        // Fit camera to model
        const bbox = new THREE.Box3().setFromObject(model);
        const bsphere = new THREE.Sphere();
        bbox.getBoundingSphere(bsphere);

        controls.target = bsphere.center;
        camera.position.y = camera.position.z = bsphere.radius * 2.5;
        controls.update();

        camera.far = bsphere.radius * 10;
        camera.updateProjectionMatrix();

    }).then(() => {
        animate();
    }).catch(err => {
      console.error(mx.getExceptionMessage(err));
    }) 

}

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate() {  
    requestAnimationFrame(animate);
    composer.render();

    model.traverse((child) => {
      if (child.isMesh) {
        const uniforms = child.material.uniforms;
        if(uniforms) {
          uniforms.time.value = performance.now() / 1000;
          uniforms.u_viewPosition.value = camera.getWorldPosition(worldViewPos);
          uniforms.u_worldMatrix.value = child.matrixWorld;
          uniforms.u_viewProjectionMatrix.value = viewProjMat.multiplyMatrices(camera.projectionMatrix, camera.matrixWorldInverse);
          uniforms.u_worldInverseTransposeMatrix.value = new THREE.Matrix4().setFromMatrix3(normalMat.getNormalMatrix(child.matrixWorld)).elements;
        }
      }
    });
}
