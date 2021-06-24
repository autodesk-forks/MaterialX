/**
 * Returns a promise that resolves the MaterialX namespace
 */
export function initMaterialX() {
    return new Promise(function (resolve) {
        // Note: Module is not a promise.
        // The then function is defined by emscripten.
        MaterialX().then((module) => {
            resolve(module.getMaterialX());
        });
    });
}
