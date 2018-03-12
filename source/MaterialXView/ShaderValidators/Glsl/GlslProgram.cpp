
#include <MaterialXView/External/GLew/glew.h>
#include <MaterialXView/ShaderValidators/Glsl/GlslProgram.h>

#include <iostream>
#include <algorithm>
#include <cmath>

namespace MaterialX
{
// OpenGL Constants
unsigned int GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID = 0;
int GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION = -1;
int GlslProgram::ProgramInput::INVALID_OPENGL_TYPE = -1;

GlslProgram::GlslProgram() :
    _programId(0),
    _hwShader(nullptr)
{
}

GlslProgram::~GlslProgram()
{
    // Clean up the program and offscreen target
    deleteProgram();
}

void GlslProgram::setStage(const std::string& code, size_t stage)
{
    if (stage < HwShader::NUM_STAGES)
    {
        _stages[stage] = code;
    }

    // A stage change invalidates any cached parsed inputs
    clearInputLists();
    // For now a individual stage setting means to not use a HwShader.
    // If / when injection of stages is allowed then this should not longer be true
    _hwShader = nullptr;
}


void GlslProgram::setStages(const HwShaderPtr shader)
{
    if (!shader)
    {
        ShaderValidationErrorList errors;
        throw ExceptionShaderValidationError("Cannot set stages using null hardware shader.", errors);
    }

    // Clear out any old data
    clearStages();

    // Extract out the shader code per stage
    _hwShader = shader;
    for (size_t i = 0; i < HwShader::NUM_STAGES; i++)
    {
        _stages[i] = _hwShader->getSourceCode(i);
    }

    // A stage change invalidates any cached parsed inputs
    clearInputLists();
}

const std::string GlslProgram::getStage(size_t stage) const
{
    if (stage < HwShader::NUM_STAGES)
    {
        return _stages[stage];
    }
    return std::string();
}

void GlslProgram::clearStages()
{
    for (size_t i = 0; i < HwShader::NUM_STAGES; i++)
    {
        _stages[i].clear();
    }

    // Clearing stages invalidates any cached inputs
    clearInputLists();
}

bool GlslProgram::haveValidStages() const
{
    // Need at least a pixel shader stage and a vertex shader stage
    const std::string& vertexShaderSource = _stages[HwShader::VERTEX_STAGE];
    const std::string& fragmentShaderSource = _stages[HwShader::PIXEL_STAGE];

    return (vertexShaderSource.length() > 0 && fragmentShaderSource.length() > 0);
}

void GlslProgram::deleteProgram()
{
    if (_programId > 0)
    {
        glDeleteObjectARB(_programId);
        _programId = UNDEFINED_OPENGL_RESOURCE_ID;
    }

    // Program deleted, so also clear cached input lists
    clearInputLists();
}

unsigned int GlslProgram::createProgram()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL program creation error.");

    deleteProgram();

    if (!haveValidStages())
    {
        errors.push_back("An invalid set of stages has been provided.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint GLStatus = GL_FALSE;
    int GLInfoLogLength = 0;

    unsigned int stagesBuilt = 0;
    unsigned int desiredStages = 0;
    for (unsigned int i = 0; i < HwShader::NUM_STAGES; i++)
    {
        if (_stages[i].length())
            desiredStages++;
    }

    // Create vertex shader
    GLuint vertexShaderId = UNDEFINED_OPENGL_RESOURCE_ID;
    std::string &vertexShaderSource = _stages[HwShader::VERTEX_STAGE];
    if (vertexShaderSource.length())
    {
        vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

        // Compile vertex shader
        const char* vertexChar = vertexShaderSource.c_str();
        glShaderSource(vertexShaderId, 1, &vertexChar, NULL);
        glCompileShader(vertexShaderId);

        // Check Vertex Shader
        glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in compiling vertex shader:");
            glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> vsErrorMessage(GLInfoLogLength + 1);
                glGetShaderInfoLog(vertexShaderId, GLInfoLogLength, NULL,
                    &vsErrorMessage[0]);
                errors.push_back(&vsErrorMessage[0]);
            }
        }
        else
        {
            stagesBuilt++;
        }
    }

    // Create fragment shader
    GLuint fragmentShaderId = UNDEFINED_OPENGL_RESOURCE_ID;
    std::string& fragmentShaderSource = _stages[HwShader::PIXEL_STAGE];
    if (fragmentShaderSource.length())
    {
        fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile fragment shader
        const char *fragmentChar = fragmentShaderSource.c_str();
        glShaderSource(fragmentShaderId, 1, &fragmentChar, NULL);
        glCompileShader(fragmentShaderId);

        // Check fragment shader
        glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in compiling fragment shader:");
            glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> fsErrorMessage(GLInfoLogLength + 1);
                glGetShaderInfoLog(fragmentShaderId, GLInfoLogLength, NULL,
                    &fsErrorMessage[0]);
                errors.push_back(&fsErrorMessage[0]);
            }
        }
        else
        {
            stagesBuilt++;
        }
    }

    // Link stages to a programs
    if (stagesBuilt == desiredStages)
    {
        _programId = glCreateProgram();
        glAttachShader(_programId, vertexShaderId);
        glAttachShader(_programId, fragmentShaderId);
        glLinkProgram(_programId);

        // Check the program
        glGetProgramiv(_programId, GL_LINK_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in linking program:");
            glGetProgramiv(_programId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> ProgramErrorMessage(GLInfoLogLength + 1);
                glGetProgramInfoLog(_programId, GLInfoLogLength, NULL,
                    &ProgramErrorMessage[0]);
                errors.push_back(&ProgramErrorMessage[0]);
            }
        }
    }

    // Cleanup
    if (vertexShaderId > 0)
    {
        if (_programId > 0)
        {
            glDetachShader(_programId, vertexShaderId);
        }
        glDeleteShader(vertexShaderId);
    }
    if (fragmentShaderId > 0)
    {
        if (_programId > 0)
        {
            glDetachShader(_programId, fragmentShaderId);
        }
        glDeleteShader(fragmentShaderId);
    }

    // If we encountered any errors while trying to create return list
    // of all errors. That is we collect all errors per stage plus any
    // errors during linking and throw one exception for them all so that
    // if there is a failure a complete set of issues is returned. We do
    // this after cleanup so keep GL state clean.
    if (errors.size())
    {
        throw ExceptionShaderValidationError(errorType, errors);
    }

    return _programId;
}

bool GlslProgram::bind() const
{
    if (_programId >= 0)
    {
        glUseProgram(_programId);
        return true;
    }
    return false;
}

bool GlslProgram::haveActiveAttributes() const
{
    GLint activeAttributeCount = 0;
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &activeAttributeCount);
    return (activeAttributeCount > 0);
}

void GlslProgram::unbind() const
{
    glUseProgram(0);
}


void GlslProgram::clearInputLists()
{
    _uniformList.clear();
    _attributeList.clear();
}

const GlslProgram::ProgramInputMap& GlslProgram::getUniformsList()
{
    return updateUniformsList();
}

const GlslProgram::ProgramInputMap& GlslProgram::getAttributesList()
{
    return updateAttributesList();
}

const GlslProgram::ProgramInputMap& GlslProgram::updateUniformsList()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL uniform parsing error.");

    if (_uniformList.size() > 0)
    {
        return _uniformList;
    }

    if (_programId <= 0)
    {
        errors.push_back("Cannot parse for uniforms without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Scan for textures
    int uniformCount = -1;
    int uniformSize = -1;
    GLenum uniformType = 0;
    int maxNameLength = 0;
    glGetProgramiv(_programId, GL_ACTIVE_UNIFORMS, &uniformCount);
    glGetProgramiv(_programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
    char* uniformName = new char[maxNameLength];
    for (int i = 0; i < uniformCount; i++)
    {
        glGetActiveUniform(_programId, GLuint(i), maxNameLength, nullptr, &uniformSize, &uniformType, uniformName);
        GLint uniformLocation = glGetUniformLocation(_programId, uniformName);
        if (uniformLocation >= 0)
        {
            ProgramInputPtr inputPtr = std::make_shared<ProgramInput>(uniformLocation, uniformType, uniformSize);
            _uniformList[std::string(uniformName)] = inputPtr;
        }
    }
    delete[] uniformName;

    if (_hwShader)
    {
        // Check for any type mismatches between the program and the h/w shader.
        // i.e the type indicated by the HwShader does not match what was generated.
        bool uniformTypeMismatchFound = false;

        /// Return all blocks of uniform variables for a stage.
        const MaterialX::Shader::VariableBlockMap& pixelShaderUniforms = _hwShader->getUniformBlocks(HwShader::PIXEL_STAGE);
        for (auto uniforms : pixelShaderUniforms)
        {
            MaterialX::Shader::VariableBlockPtr block = uniforms.second;

            for (const MaterialX::Shader::Variable* input : block->variableOrder)
            {
                // There is no way to match with an unnamed variable
                if (input->name.empty())
                {
                    continue;
                }

                auto programInput = _uniformList.find(input->name);
                if (programInput != _uniformList.end())
                {
                    if (programInput->second->gltype == mapTypeToOpenGLType(input->type))
                    {
                        programInput->second->typeString = input->type;
                        programInput->second->value = input->value;
                    }
                    else
                    {
                        errors.push_back(
                            "Pixel shader uniform block type mismatch[" + uniforms.first + "]. "
                            + "Name: \"" + input->name
                            + "\". Type: \"" + input->type
                            + "\". Semantic: \"" + input->semantic
                            + "\". Value: \"" + (input->value ? input->value->getValueString() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(input->type))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        const MaterialX::Shader::VariableBlockMap& vertexShaderUniforms = _hwShader->getUniformBlocks(HwShader::VERTEX_STAGE);
        for (auto uniforms : vertexShaderUniforms)
        {
            MaterialX::Shader::VariableBlockPtr block = uniforms.second;
            for (const MaterialX::Shader::Variable* input : block->variableOrder)
            {
                auto programInput = _uniformList.find(input->name);
                if (programInput != _uniformList.end())
                {
                    if (programInput->second->gltype == mapTypeToOpenGLType(input->type))
                    {
                        programInput->second->typeString = input->type;
                        programInput->second->value = input->value;
                        programInput->second->typeString = input->type;
                    }
                    else
                    {
                        errors.push_back(
                            "Vertex shader uniform block type mismatch[" + uniforms.first + "]. "
                            + "Name: \"" + input->name
                            + "\". Type: \"" + input->type
                            + "\". Semantic: \"" + input->semantic
                            + "\". Value: \"" + (input->value ? input->value->getValueString() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(input->type))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
                else
                {
                    programInput->second->typeString = input->type;
                }
            }
        }

        // Throw an error if any type mismatches were found
        if (uniformTypeMismatchFound)
        {
            ExceptionShaderValidationError(errorType, errors);
        }
    }

    return _uniformList;
}

int GlslProgram::mapTypeToOpenGLType(const std::string& type)
{
    if (type == MaterialX::getTypeString<int>())
        return GL_INT;
    else if (type == MaterialX::getTypeString<bool>())
        return GL_BOOL;
    else if (type == MaterialX::getTypeString<float>())
        return GL_FLOAT;
    else if (type == MaterialX::getTypeString<Vector2>() || type == MaterialX::getTypeString<Color2>())
        return GL_FLOAT_VEC2;
    else if (type == MaterialX::getTypeString<Vector3>() || type == MaterialX::getTypeString<Color3>())
        return GL_FLOAT_VEC3;
    else if (type == MaterialX::getTypeString<Vector4>() || type == MaterialX::getTypeString<Color4>())
        return GL_FLOAT_VEC4;
    else if (type == MaterialX::getTypeString<Matrix3x3>())
        return GL_FLOAT_MAT3;
    else if (type == MaterialX::getTypeString<Matrix4x4>())
        return GL_FLOAT_MAT4;
    else if (type == MaterialX::FILENAME_TYPE_STRING)
    {
        // A "filename" is not indicative of type, so just return a 2d sampler.
        return GL_SAMPLER_2D;
    }

    return GlslProgram::ProgramInput::INVALID_OPENGL_TYPE;
}

const GlslProgram::ProgramInputMap& GlslProgram::updateAttributesList()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL attribute parsing error.");

    if (_attributeList.size() > 0)
    {
        return _attributeList;
    }

    if (_programId <= 0)
    {
        errors.push_back("Cannot parse for attributes without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint numAttributes = 0;
    GLint maxNameLength = 0;
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &numAttributes);
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);
    char* attributeName = new char[maxNameLength];

    for (int i = 0; i < numAttributes; i++)
    {
        GLint attributeSize = 0;
        GLenum attributeType = 0;
        glGetActiveAttrib(_programId, GLuint(i), maxNameLength, nullptr, &attributeSize, &attributeType, attributeName);
        GLint attributeLocation = glGetAttribLocation(_programId, attributeName);
        if (attributeLocation >= 0)
        {
            ProgramInputPtr inputPtr = std::make_shared<ProgramInput>(attributeLocation, attributeType, attributeSize);
            _attributeList[std::string(attributeName)] = inputPtr;
        }
    }
    delete[] attributeName;

    if (_hwShader)
    {        
        const MaterialX::Shader::VariableBlock& appDataBlock = _hwShader->getAppDataBlock();
        
        bool uniformTypeMismatchFound = false;

        if (appDataBlock.variableOrder.size())
        { 
            for (const MaterialX::Shader::Variable* input : appDataBlock.variableOrder)
            {
                //bool foundMatch = false;
                auto programInput = _attributeList.find(input->name);
                if (programInput != _attributeList.end())
                {
                    if (programInput->second->gltype == mapTypeToOpenGLType(input->type))
                    {
                        //foundMatch = true;
                        programInput->second->typeString = input->type;
                        programInput->second->value = input->value;
                    }
                    else
                    {
                        errors.push_back(
                            "Application uniform type mismatch in block. Name: \"" + input->name
                            + "\". Type: \"" + input->type
                            + "\". Semantic: \"" + input->semantic
                            + "\". Value: \"" + (input->value ? input->value->getValueString() : "<none>")
                            + "\". GLType: " + std::to_string(mapTypeToOpenGLType(input->type))
                        );
                        uniformTypeMismatchFound = true;
                    }
                }
            }
        }

        // Throw an error if any type mismatches were found
        if (uniformTypeMismatchFound)
        {
            ExceptionShaderValidationError(errorType, errors);
        }
    }

    return _attributeList;
}

void GlslProgram::findProgramInputs(const std::string& variable, 
                                         const ProgramInputMap& variableList,
                                         ProgramInputMap& foundList,
                                         bool exactMatch)
{
    foundList.clear();

    // Scan all attributes which match the attribute identifier completely or as a prefix
    //
    int ilocation = UNDEFINED_OPENGL_PROGRAM_LOCATION;
    auto programInput = variableList.find(variable);
    if (programInput != variableList.end())
    {
        ilocation = programInput->second->location;
        if (ilocation >= 0)
        {
            foundList[variable] = programInput->second;
        }
    }
    else if (!exactMatch)
    {
        for (programInput = variableList.begin(); programInput != variableList.end(); programInput++)
        {
            const std::string& name = programInput->first;
            if (name.compare(0, variable.size(), variable) == 0)
            {
                ilocation = programInput->second->location;
                if (ilocation >= 0)
                {
                    foundList[programInput->first] = programInput->second;
                }
            }
        }
    }
}

void GlslProgram::checkErrors()
{
    ShaderValidationErrorList errors;

    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        errors.push_back("OpenGL error: " + std::to_string(error));
    }
    if (errors.size())
    {
        throw ExceptionShaderValidationError("OpenGL context error.", errors);
    }
}

}
