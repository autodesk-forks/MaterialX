// jsElement
addWrapper(function (Module, api) {

    /** Setup the Element class */
    api.Element = wrapperFactory(Module.Element);

    /** Setup the TypedElement class */
    api.TypedElement = wrapperFactory(Module.TypedElement);

    /** Setup the ValueElement class */
    api.ValueElement = wrapperFactory(Module.ValueElement);

    /** Setup the Token class */
    api.Token = wrapperFactory(Module.Token);

    /** Setup the StringResolver class */
    api.StringResolver = wrapperFactory(Module.StringResolver);

    api.prettyPrint = Module.prettyPrint;
});
