import * as THREE from 'three';
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';

let camera, scene, renderer, controls, uniforms;

init();


function init() {
    let container = document.getElementById('container');

    camera = new THREE.PerspectiveCamera(50, window.innerWidth / window.innerHeight, 1, 100);
    camera.position.set(0, 5, 5);

    // Set up scene
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x4c4c52);

    scene.add(new THREE.AmbientLight( 0x222222));
    const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
    directionalLight.position.set(1, 1, 1).normalize();
    scene.add(directionalLight);

    renderer = new THREE.WebGLRenderer();
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);
    container.appendChild(renderer.domElement);

    window.addEventListener('resize', onWindowResize);

    // controls
    controls = new OrbitControls(camera, renderer.domElement);

    // Load model and shaders
    var fileloader = new THREE.FileLoader();
    const objLoader = new OBJLoader();

    Promise.all([
        new Promise(resolve => objLoader.load('shaderball.obj', resolve)),
        new Promise(resolve => fileloader.load('shader-frag.glsl', resolve)),
        new Promise(resolve => fileloader.load('shader-vert.glsl', resolve))
    ]).then(([model, fShader, vShader]) => {
        
        const material = new THREE.RawShaderMaterial({
            uniforms: { time: { value: 0.0 } },
            vertexShader: vShader,
            fragmentShader: fShader,
        });
        uniforms = material.uniforms;

        model.traverse((child) => {
            if (child.isMesh) child.material = material;
        } );
        scene.add(model);
    }).then(() => {
        animate();
    })

}

function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate() {
    if(uniforms)
        uniforms.time.value = performance.now() / 1000;
    requestAnimationFrame(animate);
    renderer.render(scene, camera);
}
