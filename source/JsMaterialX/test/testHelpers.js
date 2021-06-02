import Module from './_build/JsMaterialX.js';
// import Module from './_build/JsMaterialX-1.37.1.js'; // uncomment to load the versioned JsMaterialX file
import 'regenerator-runtime'; // This is required for the async/awaits
var fs = require('fs');
var path = require('path');

/**
 * Helper function used to traverse a tree or a graph
 * @param {MaterialX.TreeIterator|MaterialX.GraphIterator} elements - iterator
 * @param {function} elemCb - callback called on each element. The element is passed to the callback.
 */
export function traverse(elements, elemCb) {
    for(const elem of elements) {
        elemCb && elemCb(elem);
    }
}

export function getMtlxStrings(fileNames, subPath) {
    const mtlxStrs = [];
    for (let i = 0; i < fileNames.length; i++) {
        const p = path.resolve(subPath, fileNames[parseInt(i, 10)]);
        const t = fs.readFileSync(p, 'utf8');
        mtlxStrs.push(t);
    }
    return mtlxStrs;
}
