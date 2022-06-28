#!/usr/bin/env python
'''
Generate and write *.mtlx nodegraph for a target format from an input document.
'''

import sys, os, argparse
import MaterialX as mx
from MaterialX import PyMaterialXGenShader as mx_gen_shader

def main():
    parser = argparse.ArgumentParser(description="Generate and write *.mtlx nodegraph for a target format from an input document.")
    parser.add_argument("--path", dest="paths", action='append', nargs='+', help="An additional absolute search path location (e.g. '/projects/MaterialX')")
    parser.add_argument("--library", dest="libraries", action='append', nargs='+', help="An additional relative path to a custom data library folder (e.g. 'libraries/custom')")
    parser.add_argument(dest="inputFilename", help="Filename of the input document.")
    parser.add_argument(dest="outputFilename", help="Filename of the output document.")
    parser.add_argument(dest="destShader", help="Destination shader for translation")
    opts = parser.parse_args()

    doc = mx.createDocument()
    try:
        mx.readFromXmlFile(doc, opts.inputFilename)
    except mx.ExceptionFileMissing as err:
        print(err)
        sys.exit(0)

    stdlib = mx.createDocument()
    filePath = os.path.dirname(os.path.abspath(__file__))
    searchPath = mx.FileSearchPath(os.path.join(filePath, '..', '..'))
    searchPath.append(os.path.dirname(opts.inputFilename))
    libraryFolders = []
    if opts.paths:
        for pathList in opts.paths:
            for path in pathList:
                searchPath.append(path)
    if opts.libraries:
        for libraryList in opts.libraries:
            for library in libraryList:
                libraryFolders.append(library)
    libraryFolders.append("libraries")
    mx.loadLibraries(libraryFolders, searchPath, stdlib)
    doc.importLibrary(stdlib)

    valid, msg = doc.validate()
    if not valid:
        print("Validation warnings for input document:")
        print(msg)

    # Translate materials between shading models
    translator = mx_gen_shader.ShaderTranslator.create()
    try:
        translator.translateAllMaterials(doc, opts.destShader)
    except mx.Exception as err:
        print(err)
        sys.exit(0)
    
    mx.writeToXmlFile(doc, opts.outputFilename)
    print(f"Wrote {opts.outputFilename}")
        
if __name__ == '__main__':
    main()
