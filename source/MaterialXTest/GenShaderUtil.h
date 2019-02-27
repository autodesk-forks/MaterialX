#ifndef GENSHADER_UTIL_H
#define GENSHADER_UTIL_H


#include <MaterialXCore/Document.h>
#include <MaterialXCore/Observer.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShader.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

namespace mx = MaterialX;

namespace GenShaderUtil
{
    static const std::string LIGHT_SHADER_TYPE = "lightshader";

    //
    // Load in a library. Sets the URI before import
    //
    void loadLibrary(const mx::FilePath& file, mx::DocumentPtr doc);

    //
    // Loads all the MTLX files below a given library path
    //
    void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc,
        const std::set<std::string>* excludeFiles = nullptr);
    
    //
    // Get source content, source path and resolved paths for
    // an implementation
    //
    bool getShaderSource(mx::ShaderGeneratorPtr generator,
                                const mx::ImplementationPtr implementation,
                                mx::FilePath& sourcePath,
                                mx::FilePath& resolvedPath,
                                std::string& sourceContents);

    // Find all light shaders in a document and register them with a hardware shader generator
    void registerLightType(mx::DocumentPtr doc, mx::HwShaderGenerator& shadergen, const mx::GenOptions& options);

    // Check that implementations exist for all nodedefs supported per generator
    void checkImplementations(mx::ShaderGeneratorPtr generator, std::set<std::string> generatorSkipNodeTypes,
                                     std::set<std::string> generatorSkipNodeDefs);

    // Utility test to  check unique name generation on a shader generator
    void testUniqueNames(mx::ShaderGeneratorPtr shaderGenerator, const std::string& stage);

    // Test code generation for a given element
    bool generateCode(mx::ShaderGenerator& shaderGenerator, const std::string& shaderName, mx::TypedElementPtr element,
        const mx::GenOptions& options, std::ostream& log, std::vector<std::string>testStages);
}

#endif