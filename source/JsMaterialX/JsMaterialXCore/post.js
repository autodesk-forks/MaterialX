//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved. See LICENSE.txt for license.
//

// Wrapping code in an anonymous function to prevent clashes with the main module and other pre / post JS.
(function () {
    onModuleReady(function() {
        var valueGetter = {
            AttributeDef: [
                "getValue"
            ],
            Document: [
                "getGeomPropValue"
            ],
            ValueElement: [
                "getValue",
                "getResolvedValue",
                "getDefaultValue"
            ],
            InterfaceElement: [
                "getInputValue"
            ],
            PropertySet: [
                "getPropertyValue"
            ]
        };

        function makeWrapper(c, f) {
            return function () {
                var funcName = "_" + f;
                var value = Module[c].prototype[funcName].apply(this, arguments);
                if (value) {
                    return value.getData();
                }
                return value;
            }
        }

        for (var c of Object.keys(valueGetter)) {
            for (var f of valueGetter[c]) {
                Module[c].prototype[f] = makeWrapper(c, f);
            }
        }
    });
})();
