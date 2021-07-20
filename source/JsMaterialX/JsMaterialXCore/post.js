//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved. See LICENSE.txt for license.
//

// Wrapping code in an anonymous function to prevent clashes with the main module and other pre / post JS.
(function () {
    onModuleReady(function() {
        var types = [
            "Integer", "Boolean", "Float", "Color3", "Color4", "Vector2", "Vector3", "Vector4", "Matrix33", "Matrix44", "String", "IntegerArray", "BooleanArray", "FloatArray", "StringArray"
        ];

        // class member functions
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
            ],
        };

        // static functions
        var staticValueGetter = {
            Value: [
                "createValueFromStrings"
            ]
        }

        for (var type of types) {
            staticValueGetter.Value.push("createValue" + type);
            staticValueGetter["TypedValue" + type] = [
                "createFromString"
            ];
        }

        function makeWrapper(c, f, classFunc = false) {
            return function () {
                var funcName = "_" + f;
                var value;
                if (classFunc) {
                    value = Module[c][funcName].apply(this, arguments);
                } else {
                    value = Module[c].prototype[funcName].apply(this, arguments);
                }
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

        for (var c of Object.keys(staticValueGetter)) {
            for (var f of staticValueGetter[c]) {
                Module[c][f] = makeWrapper(c, f, true);
            }
        }
    });
})();
