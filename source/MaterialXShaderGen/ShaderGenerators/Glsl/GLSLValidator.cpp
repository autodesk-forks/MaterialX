
#include "GL/glew.h"
#include <MaterialXShaderGen/ShaderGenerators/GLSL/GLSLValidator.h>

namespace MaterialX
{

GLSLValidator::GLSLValidator() :
    _programId(0)
{
}

GLSLValidator::~GLSLValidator()
{
    cleanup();
}

void GLSLValidator::setup(const HwShader& /*shader*/)
{
    // generate the shader here.. where does binding occur?
    // need to store a generator or have one passed in
}

void GLSLValidator::cleanup()
{
    if (_programId > 0)
    {
        glDeleteObjectARB(_programId);
        _programId = 0;
    }
    for (unsigned int i = 0; i < (unsigned int)Stage::STAGE_COUNT; i++)
    {
        _stages[i].clear();
    }
}

bool GLSLValidator::haveValidStages() const
{
    // Need at least a pixel shader stage and a vertex shader stage
    const std::string& vertexShaderSource = _stages[static_cast<unsigned int>(Stage::VERTEX)];
    const std::string& fragmentShaderSource = _stages[static_cast<unsigned int>(Stage::PIXEL)];
    
    return (vertexShaderSource.length() > 0 && fragmentShaderSource.length() > 0);
}


unsigned int GLSLValidator::createProgram(std::vector<std::string>& errors)
{
    errors.clear();

    cleanup();

    if (!haveValidStages())
    {
        errors.push_back("An invalid set of stages has been provided.");
        return 0;
    }

    GLint GLStatus = GL_FALSE;
    int GLInfoLogLength = 0;

    unsigned int stagesBuilt = 0;
    unsigned int desiredStages = 0;
    for (unsigned int i = 0; i < (unsigned int)Stage::STAGE_COUNT; i++)
    {
        if (_stages[i].length())
            desiredStages++;
    }

    // Create vertex shader
    GLuint vertexShaderId = 0;
    std::string &vertexShaderSource = _stages[static_cast<unsigned int>(Stage::VERTEX)];
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
    GLuint fragmentShaderId = 0;
    std::string& fragmentShaderSource = _stages[static_cast<unsigned int>(Stage::PIXEL)];
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

    return _programId;
}

bool GLSLValidator::render(std::vector<std::string>& errors)
{
    errors.clear();
    return false;
}

bool GLSLValidator::save(std::string& /*fileName*/)
{
    return false;
}

}