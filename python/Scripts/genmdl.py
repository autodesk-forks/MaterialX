#!/usr/bin/env python
'''
   TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
   All rights reserved.  See LICENSE.txt for license.

   Generate MDL implementation directory based on MaterialX nodedefs
'''

import sys
import os
import string; os.environ['PYTHONIOENCODING'] = 'utf-8'
import MaterialX as mx

def usage():
    print 'genmdl.py: Generate implementation directory for mdl based on existing MaterialX nodedefs in stdlib'
    print 'Usage:  genmdl.py <library search path> [<module name> <version>]'
    print '- A new directory called "library/stdlib/genmdl" will be created with two files added:'
    print '   - <module_name>.mdl: Module with signature stubs for each MaterialX nodedef'
    print '   - stdlib_genmdl_impl.mtlx: MaterialX nodedef implementation mapping file'
    print '- By default <module_name>="mymodule" and <version>="1.6"'

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

def _writeHeader(file, version):
    file.write('mdl ' + version + ';\n')
    file.write('using core import *;\n')
    IMPORT_LIST = { '::anno::*', '::base::*', '.::swizzle::*', '::math::*', '::state::*', '::tex::*' }
    # To verify what are the minimal imports required
    for i in IMPORT_LIST:
        file.write('import' + i + ';\n')
    file.write('\n\n')
    file.write('// Helper function mapping texture node addressmodes to MDL wrap modes\n')
    file.write('::tex::wrap_mode map_addressmode( mx_addressmode_type value ) {\n')
    file.write('    switch (value) {\n')
    file.write('        case mx_addressmode_type_clamp:\n')
    file.write('        return ::tex::wrap_clamp;\n')
    file.write('    case mx_addressmode_type_mirror:\n')
    file.write('        return ::tex::wrap_mirrored_repeat;\n')
    file.write('    default:\n')
    file.write( '   return ::tex::wrap_repeat;\n')
    file.write('    }\n')
    file.write('}\n\n')
    file.write('color4 mk_color4( float4 f ) {\n')
    file.write('    return color4( color(f.x,f.y,f.z), f.w );\n')
    file.write('}\n\n')
    file.write('float4 mk_float4( float f ) {\n')
    file.write('    return float4(f, f, f, f);\n')
    file.write('}\n\n')

def _mapGeomProp(geomprop):
    outputValue = ''
    if len(geomprop):
        if geomprop.find('UV') >= 0:
            outputValue = 'swizzle::xy(::state::texture_coordinate(0))'
        elif geomprop.find('Pobject') >= 0:
            outputValue = '::state::transform_point(::state::coordinate_internal,::state::coordinate_object,::state::position())'
        elif geomprop.find('PWorld') >= 0:
            outputValue = '::state::transform_point(::state::coordinate_internal,::state::coordinate_world,::state::position())'
        elif geomprop.find('Nobject') >= 0:
            outputValue = '::state::transform_normal(::state::coordinate_internal,::state::coordinate_object,::state::normal())'
        elif geomprop.find('Nworld') >= 0:
            outputValue = '::state::transform_normal(::state::coordinate_internal,::state::coordinate_world,::state::normal())'
        elif geomprop.find('Tobject') >= 0:
            outputValue = '::state::transform_vector(::state::coordinate_internal,::state::coordinate_object,::state::texture_tangent_u(0))'
        elif geomprop.find('Tworld') >= 0:
            outputValue = 'state::transform_vector(::state::coordinate_internal,::state::coordinate_world,::state::texture_tangent_u(0))'
        elif geomprop.find('Bobject') >= 0:
            outputValue = 'state::transform_vector(::state::coordinate_internal,::state::coordinate_object,::state::texture_tangent_v(0))'
        elif geomprop.find('Bworld') >= 0:
            outputValue = '::state::transform_vector(::state::coordinate_internal,::state::coordinate_world,::state::texture_tangent_v(0))'
    return outputValue



def _writeValueAssignment(file, outputValue, outputType, writeEmptyValues):
    # Mapping of types to initializers
    assignMap = dict()
    assignMap['float2[<N>]'] = 'float2[]'

    if outputType == 'color4':
        outputType = 'mk_color4'

    elif outputType in assignMap:
        outputType = assignMap[outputType]
        writeEmptyValues = True

    if len(outputValue) or writeEmptyValues:
        file.write(' = ')
        if outputType:
            file.write(outputType + '(')
        if outputType == 'string':
            file.write('"')
        file.write(outputValue)
        if outputType == 'string':
            file.write('"')
        if outputType:
            file.write(')')

def _mapType(typeName, typeMap, functionName):
    if 'mx_constant_filename' == functionName:
        return 'string'
    if typeName in typeMap:
        return typeMap[typeName]
    return typeName

INDENT = '\t'
SPACE = ' '
QUOTE = '"'
FUNCTION_PREFIX = 'mx_'
FUNCTION_PARAMETER_PREFIX = 'mxp_'

# Basic template for writing out logic for "image" node definition
def _writeImageImplementation(file, outputType):
    file.write(INDENT + 'if ( mxp_uaddressmode == mx_addressmode_type_constant\n')
    file.write(INDENT + '     && ( mxp_texcoord.x < 0.0 || mxp_texcoord.x > 1.0))\n')
    file.write(INDENT + INDENT + 'return mxp_default;\n')
    file.write(INDENT + 'if ( mxp_vaddressmode == mx_addressmode_type_constant\n')
    file.write(INDENT + '     && ( mxp_texcoord.y < 0.0 || mxp_texcoord.y > 1.0))\n')
    file.write(INDENT + INDENT + 'return mxp_default;\n\n')

    file.write(INDENT + outputType + ' returnValue')
    isColor4 = (outputType == 'color4')
    if isColor4:
        outputType = 'float4'
        outputType = 'mk_color4( ::tex::lookup_' + outputType
    else:
        outputType = '::tex::lookup_' + outputType
    outputValue = 'tex: mxp_file, \n' \
        + INDENT*6 + 'coord: mxp_texcoord,\n' \
        + INDENT*6 + 'wrap_u: map_addressmode(mxp_uaddressmode),\n' \
        + INDENT*6 + 'wrap_v: map_addressmode(mxp_vaddressmode)'
    _writeValueAssignment(file, outputValue, outputType, True)
    if isColor4:
        file.write(')')
    file.write(';\n')
    file.write(INDENT + 'return returnValue;\n')

def _writeOneArgumentMath(file, outputType, functionName):
        if outputType == 'color4':
            file.write(INDENT + 'return mk_color4(::math::' + functionName + '(mk_float4(mxp_in)));\n')
        elif outputType == 'color':
            file.write(INDENT + 'return color(::math::' + functionName + '(float3(mxp_in)));\n')
        else:
            file.write(INDENT + 'return ::math::' + functionName + '(mxp_in);\n')

def _writeOperatorMath(file, outputType, arg1, functionName, arg2):
        if outputType == 'color4':
            file.write(INDENT + 'return mk_color4(mk_float4(' + arg1 +') ' + functionName + ' mk_float4(' + arg2 + '));\n')
        elif outputType == 'float3x3' or outputType == 'float4x4':
            file.write(INDENT + 'return ' + outputType + '(' + arg1 + ') ' + functionName + ' ' + outputType + '(' + arg2 + ');\n')
        else:
            file.write(INDENT + 'return ' + arg1 + ' ' + functionName + ' ' + arg2 + ';\n')

def _writTwoArgumentMath(file, outputType, functionName):
        if outputType == 'color4':
            file.write(INDENT + 'return mk_color4(::math::' + functionName + '(mk_float4(mxp_in1), mk_float4(mxp_in2)));\n')
        elif outputType == 'color':
            file.write(INDENT + 'return color(::math::' + functionName + '(float3(mxp_in1), float3(mxp_in2)));\n')
        else:
            file.write(INDENT + 'return ::math::' + functionName + '(mxp_in1, mxp_in2);\n')

def main():

    if len(sys.argv) < 2:
        usage()
        sys.exit(0)

    _startPath = os.path.abspath(sys.argv[1])
    if os.path.exists(_startPath) == False:
        print('Start path does not exist: ' + _startPath + '. Using current directory.\n')
        _startPath = os.path.abspath(os.getcwd())

    moduleName = 'mymodule'
    if len(sys.argv) > 2:
        moduleName = sys.argv[2]

    version = '1.6'
    if len(sys.argv) > 3:
        version = sys.argv[3]

    LIBRARY = 'stdlib'

    doc = mx.createDocument()
    searchPath = os.path.join(_startPath, 'libraries')
    libraryPath = os.path.join(searchPath, 'stdlib')
    _loadLibraries(doc, searchPath, libraryPath)

    DEFINITION_PREFIX = 'ND_'
    IMPLEMENTATION_PREFIX = 'IM_'
    IMPLEMENTATION_STRING = 'impl'
    GENMDL = 'genmdl'

    # Create target directory if don't exist
    outputPath = os.path.join(libraryPath, GENMDL)
    if not os.path.exists(outputPath):
        os.mkdir(outputPath)

    file = None

    # Write to single file if module name specified
    if len(moduleName):
        file = open(outputPath + '/' + moduleName + '.mdl', 'w+')
        _writeHeader(file, version)

    # Dictionary to map from MaterialX type declarations
    # to MDL type declarations
    typeMap = dict()
    typeMap['boolean'] = 'bool'
    typeMap['integer'] = 'int'
    typeMap['color2'] = 'float2'
    typeMap['color3'] = 'color'
    typeMap['color4'] = 'color4'
    typeMap['vector2'] = 'float2'
    typeMap['vector3'] = 'float3'
    typeMap['vector4'] = 'float4'
    typeMap['matrix33'] = 'float3x3'
    typeMap['matrix44'] = 'float4x4'
    typeMap['filename'] = 'texture_2d' # Assume all file textures are 2d for now
    typeMap['geomname'] = 'string'
    typeMap['floatarray'] = 'float[<N>]'
    typeMap['integerarray'] = 'int[<N>]'
    typeMap['color2array'] = 'float2[<N>]'
    typeMap['color3array'] = 'color[<N>]'
    typeMap['color4array'] = 'float4[<N>]'
    typeMap['vector2array'] = 'float2[<N>]'
    typeMap['vector3array'] = 'float3[<N>]'
    typeMap['vector4array'] = 'float4[<N>]'
    typeMap['stringarray'] = 'string[<N>]'
    typeMap['geomnamearray'] = 'string[<N>]'
    typeMap['surfaceshader'] = 'material'
    typeMap['volumeshader'] = 'material'
    typeMap['displacementshader'] = 'material'
    typeMap['lightshader'] = 'material'

    functionTypeMap = dict()
    functionTypeMap['mx_separate2_color2'] = 'mx_separate2_color2_type'
    functionTypeMap['mx_separate3_color3'] = 'mx_separate3_color3_type'
    functionTypeMap['mx_separate4_color4'] = 'mx_separate4_color4_type'
    functionTypeMap['mx_separate2_vector2'] = 'mx_separate2_vector2_type'
    functionTypeMap['mx_separate3_vector3'] = 'mx_separate3_vector3_type'
    functionTypeMap['mx_separate4_vector4'] = 'mx_separate4_vector4_type'

    # Create an implementation per nodedef
    #
    implDoc = mx.createDocument()
    nodedefs = doc.getNodeDefs()
    nodeGraphs = doc.getNodeGraphs()
    implementedCont = 0;
    totalCount = 0;
    for nodedef in nodedefs:

        # Skip any node definitions which are implemented as node graphs
        nodeDefName = nodedef.getName()
        #print(nodeDef)
        implementationIsGraph = False
        for nodeGraph in nodeGraphs:
            graphName = nodeGraph.getName()
            #print('Scane nodegraph: ' + nodeGraph.getName() + '\n')
            if nodeGraph.getAttribute('nodedef') == nodeDefName:
                file.write('// Nodedef: ' + nodeDefName + ' is represented by a nodegraph: ' + graphName + '\n')
                implementationIsGraph = True
                break
        if implementationIsGraph:
            continue

        # These definitions are for organization only
        nodeGroup = nodedef.getAttribute('nodegroup')
        if nodeGroup == 'organization':
            continue

        # TODO: Skip array definitions for now
        nodeCategory = nodedef.getAttribute('node')
        if  nodeCategory == 'arrayappend':
            continue

        if len(nodedef.getActiveOutputs()) == 0:
           continue

        totalCount += 1

        outputValue = ''
        outputType = ''

        # String out definition prefix
        nodeName = nodedef.getName()
        if len(nodeName) > 3:
            if (nodeName[0:3] == DEFINITION_PREFIX):
                nodeName = nodeName[3:]

        filename = nodeName + '.mdl'

        implname = IMPLEMENTATION_PREFIX + nodeName + '_' + GENMDL
        impl = implDoc.addImplementation(implname)
        impl.setNodeDef(nodedef)
        if len(moduleName):
            impl.setFile('stdlib/genmdl/' + moduleName + '.mdl')
        else:
            impl.setFile('stdlib/genmdl/' + filename)

        functionName = FUNCTION_PREFIX + nodeName
        functionCallName = functionName
        if len(moduleName):
            functionCallName = moduleName + '::' + functionName
        impl.setFunction(functionCallName)
        impl.setLanguage(GENMDL)

        # If no module name, create a new mdl file per nodedef
        if len(moduleName) == 0:
            file = open(outputPath + '/' + filename, 'w+')
            _writeHeader(file, version)

        outType = nodedef.getType()
        routeInputToOutput = False

        # TODO: Skip multioutput nodes for now
        #if outType == 'multioutput':
        #    continue

        # Create a signature for the nodedef
        file.write('export ')
        # Add output argument
        if functionName in functionTypeMap:
            outType = functionTypeMap[functionName]
        else:
            outType = _mapType(outType, typeMap, functionName)

        file.write(outType + SPACE)
        file.write(functionName + '(\n')

        # Add input arguments
        #
        elems = nodedef.getActiveValueElements()
        lastComma = len(elems) - len(nodedef.getActiveOutputs())
        i = 0
        for elem in elems:

            dataType = ''
            defaultgeomprop = ''

            # Skip output elements
            if isinstance(elem, mx.Output):
                outputValue = elem.getAttribute('default')
                if outputValue == '[]':
                    outputvalue = ''
                if not outputValue:
                    outputValue = elem.getAttribute('defaultinput')
                    if outputValue:
                        outputValue = FUNCTION_PARAMETER_PREFIX + outputValue
                    routeInputToOutput = True
                outputType = elem.getType()
                outputType = _mapType(outputType, typeMap, functionName)
                continue

            # Parameters map to uniforms
            elif isinstance(elem, mx.Parameter):
                dataType = 'uniform '
            # Inputs map to varyings
            elif isinstance(elem, mx.Input):
                dataType = ''
                defaultgeomprop = elem.getAttribute('defaultgeomprop')

            # Determine type
            typeString = elem.getType()
            isFileTexture = (typeString == 'filename')
            typeString  = _mapType(typeString , typeMap, functionName)
            isString = (typeString == 'string')

            # Determine value
            isGeometryInput = len(defaultgeomprop) > 0
            if isGeometryInput:
                valueString = _mapGeomProp(defaultgeomprop)
            else:
                valueString = elem.getValueString()

            parameterName = FUNCTION_PARAMETER_PREFIX + elem.getName();
            isEnumeration = len(elem.getAttribute('enum')) > 0
            # Remap enumerations.
            # Note: This is hard-coded since there are no type enumerations in MaterialX to map from
            if isEnumeration and not isGeometryInput:
                ADDRESS_MODE = { 'constant', 'clamp', 'periodic', 'mirror'}
                FILTER_LOOKUP = { 'closest', 'linear', 'cubic' }
                COORDINATE_SPACES = { 'model', 'object' , 'world' }
                FILTER_TYPE = { 'box', 'gaussian' }
                if valueString in ADDRESS_MODE:
                    typeString = 'mx_addressmode_type'
                    valueString = typeString + '_' + valueString
                elif valueString in FILTER_LOOKUP:
                    typeString = 'mx_filterlookup_type'
                    valueString = typeString + '_' + valueString
                elif valueString in COORDINATE_SPACES:
                    typeString = 'mx_coordinatespace_type'
                    valueString = typeString + '_' + valueString
                elif valueString in FILTER_TYPE:
                    typeString = 'mx_filter_type'
                    valueString = typeString + '_' + valueString

            file.write(INDENT + dataType + typeString + SPACE + parameterName)
            _writeValueAssignment(file, valueString, typeString, isFileTexture or isString)

            # Add annotations if any
            description = elem.getAttribute('doc')
            if len(elem.getAttribute('enum')):
                description = description + 'Enumeration {' + elem.getAttribute('enum') + '}.'
            if len(elem.getAttribute('unittype')):
                description = description + 'Unit Type:' + elem.getAttribute('unittype') + '.'
            if len(elem.getAttribute('unit')):
                description = description + ' Unit:' + elem.getAttribute('unit') + "."
            uiname = elem.getAttribute('uiname')
            uigroup = elem.getAttribute('uifolder')
            if len(description) or len(uiname) or len(uigroup):
                file.write(INDENT + '\n' + INDENT + '[[')
                count = 0
                if len(description):
                    file.write("\n" + INDENT + INDENT + 'anno::description("' + description + '")')
                    count = count + 1
                if len(uiname):
                    if count > 0:
                        file.write(',')
                    file.write("\n" + INDENT + INDENT + 'anno::display_name("' + uiname + '")')
                    count = count + 1
                if len(uigroup):
                    if count > 0:
                        file.write(',')
                    file.write("\n" + INDENT + INDENT + 'anno::in_group("' + uigroup + '")')
                file.write('\n' + INDENT + ']]')

            i = i + 1
            if i < lastComma:
                file.write(',')
            file.write('\n')

        file.write(')\n')
        nodegroup = nodedef.getAttribute('nodegroup')
        if len(nodegroup):
            file.write(INDENT + '[[\n')
            file.write(INDENT + INDENT + 'anno::description("Node Group: ' + nodegroup + '")\n')
            file.write(INDENT + ']]\n')
        if outputType == 'material':
            if outputValue:
                file.write('= ' + outputValue + '; // TODO \n\n')
            else:
                file.write('= material(); // TODO \n\n')
        else:
            file.write('{\n')
            if functionName in functionTypeMap:
                file.write(INDENT + '// No-op. Return default value for now\n')
                file.write(INDENT + 'return ' + functionTypeMap[functionName] + '();\n')
            else:
                wroteImplementation = False
                if nodeCategory == 'constant':
                    file.write(INDENT + 'return mxp_value;\n')
                    wroteImplementation = True;
                elif nodeCategory == 'absval':
                    _writeOneArgumentMath(file, outputType, 'abs')
                    wroteImplementation = True;
                elif nodeCategory == 'ceil':
                    _writeOneArgumentMath(file, outputType, nodeCategory)
                    wroteImplementation = True;
                elif nodeCategory == 'floor':
                    _writeOneArgumentMath(file, outputType, nodeCategory)
                    wroteImplementation = True;
                elif nodeCategory == 'sin':
                    _writeOneArgumentMath(file, outputType, nodeCategory)
                    wroteImplementation = True;
                elif nodeCategory == 'cos':
                    _writeOneArgumentMath(file, outputType, nodeCategory)
                    wroteImplementation = True;
                elif nodeCategory == 'tan':
                    _writeOneArgumentMath(file, outputType, nodeCategory)
                    wroteImplementation = True;
                elif nodeCategory == 'max':
                    _writTwoArgumentMath(file, outputType, 'max')
                    wroteImplementation = True;
                elif nodeCategory == 'min':
                    _writTwoArgumentMath(file, outputType, 'min')
                    wroteImplementation = True;
                elif nodeCategory == 'add':
                    _writeOperatorMath(file, outputType, 'mxp_in1', '+', 'mxp_in2')
                    wroteImplementation = True;
                elif nodeCategory == 'subtract':
                    _writeOperatorMath(file, outputType, 'mxp_in1', '-', 'mxp_in2')
                    wroteImplementation = True;
                elif nodeCategory == 'invert':
                    _writeOperatorMath(file, outputType, 'mxp_amount', '-', 'mxp_in')
                    wroteImplementation = True;
                elif nodeCategory == 'multiply':
                    _writeOperatorMath(file, outputType, 'mxp_in1', '*', 'mxp_in2')
                    wroteImplementation = True;
                elif nodeCategory == 'divide':
                    if outputType == 'color4':
                        file.write(INDENT + 'return mk_color4(mk_float4(mxp_in1) / mk_float4(mxp_in2));\n')
                        wroteImplementation = True;
                    elif outputType == 'float3x3' or outputType == 'float4x4':
                        print('Skip division implementation for ' + outputType + '. Not supported in MDL\n')
                        #file.write(INDENT + 'return ' + outputType + '(mxp_in1) / ' + outputType + '(mxp_in2);\n')
                    else:
                        file.write(INDENT + 'return mxp_in1 / mxp_in2;\n')
                        wroteImplementation = True;
                elif nodeCategory == 'modulo':
                    _writTwoArgumentMath(file, outputType, 'fmod')
                    wroteImplementation = True;
                elif nodeCategory == 'power':
                    _writTwoArgumentMath(file, outputType, 'pow')
                    wroteImplementation = True;
                elif nodeCategory == 'image':
                    _writeImageImplementation(file, outputType)
                    wroteImplementation = True;

                if wroteImplementation:
                    implementedCont += 1
                else:
                    file.write(INDENT + '// No-op. Return default value for now\n')
                    file.write(INDENT + outputType + ' defaultValue')
                    if routeInputToOutput:
                        outputType = ''
                    _writeValueAssignment(file, outputValue, outputType, outputType == 'texture_2d')
                    file.write(';\n')
                    file.write(INDENT + 'return defaultValue;\n')
            file.write('}\n\n')

        if len(moduleName) == 0:
            file.close()

    if len(moduleName):
        file.close()

    # Save implementation reference file to disk
    implFileName = LIBRARY + '_' + GENMDL + '_' + IMPLEMENTATION_STRING + '.mtlx'
    implPath = os.path.join(outputPath, implFileName)
    print('Wrote implementation file: ' + implPath + '. ' + str(implementedCont) + '/' + str(totalCount) + '\n')
    mx.writeToXmlFile(implDoc, implPath)

if __name__ == '__main__':
    main()
