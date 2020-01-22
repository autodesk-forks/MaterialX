#!/usr/bin/env python
import sys
import os
import string
os.environ['PYTHONIOENCODING'] = 'utf-8'
import MaterialX as mx

def usage():
    print 'genmdl.py: Generate implementation directory for mdl'
    print 'Usage:  genmdl.py <library search path> <package name>'
    
def _getSubDirectories(libraryPath):
    return [name for name in os.listdir(libraryPath)
            if os.path.isdir(os.path.join(libraryPath, name))]

def _getMTLXFilesInDirectory(path):
    for file in os.listdir(path):
        if file.endswith('.mtlx'):
            yield file

def _loadLibrary(file, doc):
    libDoc = mx.createDocument()
    mx.readFromXmlFile(libDoc, file)
    libDoc.setSourceUri(file)
    doc.importLibrary(libDoc)

def _loadLibraries(doc, searchPath, libraryPath):
    librarySubPaths = _getSubDirectories(libraryPath)
    librarySubPaths.append(libraryPath)
    for path in librarySubPaths:
        filenames = _getMTLXFilesInDirectory(os.path.join(libraryPath, path))
        for filename in filenames:
            filePath = os.path.join(libraryPath, os.path.join(path, filename))
            _loadLibrary(filePath, doc)

def _writeImports(file):
    file.write('mdl 1.6;\n')
    # To verify what are the minimal imports required
    file.write('import::anno::*;\n')
    file.write('import::base::*;\n')
    file.write('import::df::*;\n')
    file.write('import::math::*;\n')
    file.write('import::state::*;\n')
    file.write('import::tex::*; \n')                        
    file.write('\n')


def main():

    if len(sys.argv) < 2:
        usage()
        sys.exit(0)

    _startPath = os.path.abspath(sys.argv[1])
    if os.path.exists(_startPath) == False:
        print('Start path does not exist: ' + _startPath + '. Using current directory.\n')
        _startPath = os.path.abspath(os.getcwd())

    moduleName = ''
    if len(sys.argv) > 2:
        moduleName = sys.argv[2]

    LIBRARY = 'stdlib'

    doc = mx.createDocument()
    searchPath = os.path.join(_startPath, 'libraries')
    libraryPath = os.path.join(searchPath, 'stdlib')
    _loadLibraries(doc, searchPath, libraryPath)

    DEFINITION_PREFIX = 'ND_'
    IMPLEMENTATION_PREFIX = 'IM_'
    IMPLEMENTATION_STRING = 'impl'
    GENMDL = 'genmdl'

    # Create target Directory if don't exist
    outputPath = os.path.join(libraryPath, GENMDL)
    if not os.path.exists(outputPath):
        os.mkdir(outputPath)

    file = None
    wroteIncludes = True
    if len(moduleName):
        file = open(outputPath + '/' + moduleName + '.mdl', 'w+')
        _writeImports(file)
        
    # Create an implementation per nodedef
    #
    implDoc = mx.createDocument()

    nodedefs = doc.getNodeDefs()
    for nodedef in nodedefs:

        if len(nodedef.getActiveOutputs()) == 0:
           continue

        # String out definition prefix
        nodeName = nodedef.getName()
        if len(nodeName) > 3:
            if (nodeName[0:3] == DEFINITION_PREFIX):
                nodeName = nodeName[3:]

        filename = nodeName + '.mdl'
        INDENT = '  '

        typeMap = dict()
        typeMap['boolean'] = 'bool'
        typeMap['integer'] = 'int'
        typeMap['color2'] = 'float2'
        typeMap['color4'] = 'float4'
        typeMap['vector2'] = 'float2'
        typeMap['vector3'] = 'float3'
        typeMap['vector4'] = 'float4'
        typeMap['matrix33'] = 'float3x3'
        typeMap['matrix44'] = 'float4x4'
        typeMap['filename'] = 'string'
        typeMap['geomname'] = 'string'
        typeMap['floatarray'] = 'float[<count>]'
        typeMap['integerarray'] = 'int[<count>]'
        typeMap['color2array'] = 'float2[<count>]'
        typeMap['color3array'] = 'color[<count>]'
        typeMap['color4array'] = 'float4[<count>]'
        typeMap['vector2array'] = 'float2[<count>]'
        typeMap['vector3array'] = 'float3[<count>]'
        typeMap['vector4array'] = 'float4[<count>]'
        typeMap['stringarray'] = 'string[<count>]'
        typeMap['geomnamearray'] = 'string[<count>]'

        implname = IMPLEMENTATION_PREFIX + nodeName + '_' + GENMDL
        impl = implDoc.addImplementation(implname)                    
        impl.setNodeDef(nodedef)
        if len(moduleName):
            impl.setFile('stdlib/genmdl/' + moduleName + '.mdl')
        else:
            impl.setFile('stdlib/genmdl/' + filename)

        functionName = 'mx_' + nodeName
        functionCallName = functionName
        if len(moduleName):
            functionCallName = moduleName + '::' + functionName
        impl.setFunction(functionCallName)
        impl.setLanguage(GENMDL)

        if len(moduleName) == 0:
            file = open(outputPath + '/' + filename, 'w+')
            _writeImports(file)

        # Create a signature for the nodedef
        file.write('export ')
        # Add output argument
        outputType = nodedef.getType()
        if outputType in typeMap:
            outputType = typeMap[outputType]

        file.write(outputType + ' ')
        file.write(functionName + '\n')
        # Add input arguments

        elems = nodedef.getActiveValueElements()
        lastComma = len(elems) - 2
        i = 0
        for elem in elems:

            # Skip output elements
            if isinstance(elem, mx.Output):
                continue

            typeString = elem.getType()
            if typeString in typeMap:
                typeString = typeMap[typeString]
            
            isString = (typeString == 'string')
            file.write(INDENT + typeString)
            file.write(INDENT + elem.getName())
            valueString = elem.getValueString()

            if len(valueString):
                file.write(' = ' + typeString + '(')
                if isString:
                    file.write('\'')
                file.write(valueString)
                if isString:
                    file.write('\'')
                file.write(')')

            if i < lastComma:
                file.write(',')
            file.write('\n')
            i = i + 1

        file.write(')\n')
        file.write('{\n')
        file.write(INDENT + '// TODO: Add implementation here\n')
        file.write(INDENT + outputType + ' outVal;\n')
        file.write(INDENT + 'return outVal;\n')
        file.write('}\n\n')

        if len(moduleName) == 0:
            file.close()

    if len(moduleName):
        file.close()

    # Save implementations to disk
    implFileName = LIBRARY + '_' + GENMDL + '_' + IMPLEMENTATION_STRING + '.mtlx'
    implPath = os.path.join(outputPath, implFileName)
    print('Wrote implementation file: ' + implPath + '\n')
    mx.writeToXmlFile(implDoc, implPath)

if __name__ == '__main__':
    main()
