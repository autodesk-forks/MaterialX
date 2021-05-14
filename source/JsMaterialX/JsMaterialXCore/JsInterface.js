// jsInterface
addWrapper(function(Module, api) {

    /** Setup the PortElement class */
    api.PortElement = wrapperFactory(Module.PortElement);

    /** Setup the Input class */
    api.Input = wrapperFactory(Module.Input);

    /** Setup the Output class */
    api.Output = wrapperFactory(Module.Output);

    /** Setup the InterfaceElement class */
    api.InterfaceElement = wrapperFactory(Module.InterfaceElement);
});
