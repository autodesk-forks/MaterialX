import { expect } from 'chai';;
import Module from './_build/JsMaterialXCore.js';

describe('Value', () => {
    let mx;
    before(async () => {
        mx = await Module();
    });

    it('Create values of different types', () => {
        const testValues = {
            integer: '1',
            boolean: 'true',
            float: '1.1',
            color3: '0.1, 0.2, 0.3',
            color4: '0.1, 0.2, 0.3, 0.4',
            vector2: '1.1, 2.1',
            vector3: '1.1, 2.1, 3.1',
            vector4: '1.1, 2.1, 3.1, 4.1',
            matrix33: '0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1',
            matrix44: '1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1, 1.1',
            string: 'value',
            integerarray: '1, 2, 3',
            booleanarray: 'false, true, false',
            floatarray: '1.1, 2.1, 3.1',
            stringarray: "'one', 'two', 'three'",
        };

        for (let type in testValues) {
            const value = testValues[String(type)];
            const newValue = mx.Value.createValueFromStrings(value, type);
            const valuePtr = mx.Value._createValueFromStrings(value, type);
            const typeString = valuePtr.getTypeString();
            const valueString = valuePtr.getValueString();
            expect(typeString).to.equal(type);
            expect(valueString).to.equal(value);
            expect(newValue).to.deep.equal(valuePtr.getData());
        }
    });
});