// jsTypes
addWrapper(function(Module, api = {}) {
    Object.keys(Module, (key) => {
        api[key] = Module[key];
    });
    api.Vector2 = wrapperFactory(Module.Vector2);
    api.Vector3 = wrapperFactory(Module.Vector3);
    api.Vector4 = wrapperFactory(Module.Vector4);
    api.Color3 = wrapperFactory(Module.Color3);
    api.Color4 = wrapperFactory(Module.Color4);
    api.Matrix33 = wrapperFactory(Module.Matrix33);
    api.Matrix44 = wrapperFactory(Module.Matrix44);
});
