#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXShaderGen/HwShader.h>

#include <fstream>

namespace mx = MaterialX;

// Compile if module was compiled
#if defined(MATERIALX_BUILD_VIEW)
// Run only on windows for now
#if defined(_WIN32)
#include <iostream>
#include <MaterialXView/ShaderValidators/Glsl/GlslValidator.h>

TEST_CASE("GLSL Validation", "[shadervalid]")
{
    // Initialize a GLSL validator. Will initialize 
    // window and context as well for usage
    mx::GlslValidator validator;
    bool initialized = false;
    try
    {
        validator.initialize();
        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError e)
    {
        for (auto error : e._errorLog)
        {
            std::cout << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(initialized);

    // Read in some sample fragments
    unsigned int stagesSet = 0;
    //std::string shaderName("conditional_test1");
    //std::string shaderName("simple_test1_graphoutput");
    //std::string shaderName("simple_test1_node");
    //std::string shaderName("geometric_nodes");
    std::string shaderName("subgraph_ex1");
    //std::string shaderName("subgraph_ex2");
    std::string vertexShaderPath = shaderName + ".vert";
    std::string pixelShaderPath = shaderName + ".frag";
    std::stringstream vertexShaderStream;
    std::stringstream pixelShaderStream;
    std::ifstream shaderFile;
    shaderFile.open(vertexShaderPath);
    if (shaderFile.is_open())
    {
        vertexShaderStream << shaderFile.rdbuf();
        validator.setStage(vertexShaderStream.str(), mx::HwShader::VERTEX_STAGE);
        shaderFile.close();
        stagesSet++;
    }
    shaderFile.open(pixelShaderPath);
    if (shaderFile.is_open())
    {
        pixelShaderStream << shaderFile.rdbuf();
        validator.setStage(pixelShaderStream.str(), mx::HwShader::PIXEL_STAGE);
        shaderFile.close();
        stagesSet++;
    }
    REQUIRE(stagesSet == 2);
    if (stagesSet == 2)
    {
        bool programCompiled = false;
        try 
        {
            validator.createProgram();
            programCompiled = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e._errorLog)
            {
                std::cout << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(programCompiled);
    }

    bool uniformsParsed = false;
    try {
        const mx::GlslValidator::ProgramInputList& uniforms = validator.getUniformsList();
        for (auto input : uniforms)
        {
            unsigned int type = input.second->_type;
            int location = input.second->_location;
            std::cout << "Program Uniform: \"" << input.first << "\". Location=" << location << ". Type=" << type << "." << std::endl;
        }
        uniformsParsed = true;
    }
    catch (mx::ExceptionShaderValidationError e)
    {
        for (auto error : e._errorLog)
        {
            std::cout << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(uniformsParsed);        

    bool attributesParsed = false;
    try
    {
        const mx::GlslValidator::ProgramInputList& attributes = validator.getAttributesList();
        for (auto input : attributes)
        {
            unsigned int type = input.second->_type;
            int location = input.second->_location;
            std::cout << "Program Attribute: \"" << input.first << "\". Location=" << location << ". Type=" << type << "." << std::endl;
        }
        attributesParsed = true;
    }
    catch (mx::ExceptionShaderValidationError e)
    {
        for (auto error : e._errorLog)
        {
            std::cout << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(attributesParsed);

    // To add: Hook in set up of program for validator. 
    bool renderSucceeded = false;
    try
    {
        validator.render();
        renderSucceeded = true;
    }
    catch (mx::ExceptionShaderValidationError e)
    {
        for (auto error : e._errorLog)
        {
            std::cout << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(renderSucceeded);

    try
    {
        std::string fileName = shaderName + ".exr";
        validator.save(fileName);
    }
    catch (mx::ExceptionShaderValidationError e)
    {
        for (auto error : e._errorLog)
        {
            std::cout << e.what() << " " << error << std::endl;
        }
    }
}
#endif
#endif
