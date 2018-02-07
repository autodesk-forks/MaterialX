
#include "GLew/glew.h"
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslValidator.h>

namespace MaterialX
{

GlslValidator::GlslValidator() :
    _programId(0),
    _colorTarget(0),
    _depthTarget(0),
    _frameBuffer(0),
    _frameBufferWidth(256),
    _frameBufferHeight(256),
    _initialized(false)
{
}

GlslValidator::~GlslValidator()
{
    deleteProgram();
    deleteTarget();
}

void GlslValidator::setStages(const HwShader& shader)
{
    // Clear out any old data
    clearStages();

    // Extract out the shader code per stage
    for (size_t i = 0; i < HwShader::NUM_STAGES; i++)
    {
        _stages[i] = shader.getSourceCode(i);
    }
}

void GlslValidator::initialize(ErrorList& errors)
{
    if (!_initialized)
    {
        // Initialize windowing code -- to add GLUT or similar

        // Initialize glew
#ifndef __APPLE__
        glewInit();
        if (!glewIsSupported("GL_VERSION_2_0"))
        {
            errors.push_back("OpenGL 2.0 not supported");
        }
#endif
        glClearColor(0, 0, 0, 0);
        glClearStencil(0);

        _initialized = true;
    }
}

void GlslValidator::clearStages()
{
    for (size_t i = 0; i < HwShader::NUM_STAGES; i++)
    {
        _stages[i].clear();
    }
}

bool GlslValidator::haveValidStages() const
{
    // Need at least a pixel shader stage and a vertex shader stage
    const std::string& vertexShaderSource = _stages[HwShader::VERTEX_STAGE];
    const std::string& fragmentShaderSource = _stages[HwShader::PIXEL_STAGE];

    return (vertexShaderSource.length() > 0 && fragmentShaderSource.length() > 0);
}

void GlslValidator::deleteTarget()
{
    if (_frameBuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &_frameBuffer);
    }
}

bool GlslValidator::createTarget(ErrorList& errors)
{
    // Only frame buffer only once
    if (_frameBuffer > 0)
    {
        return true;
    }

    // Set up frame buffer
    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

    // Create an offscreen floating point color target and attach to the framebuffer
    _colorTarget = 0;
    glGenTextures(1, &_colorTarget);
    glBindTexture(GL_TEXTURE_2D, _colorTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _frameBufferWidth, _frameBufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _colorTarget, 0);

    // Create floating point offscreen depth target
    _depthTarget = 0;
    glGenTextures(1, &_depthTarget);
    glBindTexture(GL_TEXTURE_2D, _depthTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _frameBufferWidth, _frameBufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTarget, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDrawBuffer(GL_NONE);

    // Validate the framebuffer
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &_frameBuffer);
        _frameBuffer = 0;
        errors.push_back("Frame buffer setup failed.");
        return false;
    }

    // Unbind on cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

bool GlslValidator::bindTarget(bool bind, ErrorList& errors)
{
    // Make sure we have a target to bind first
    createTarget(errors);
    if (!_frameBuffer)
    {
        return false;
    }

    // Bind the frame buffer and route to color texture target
    if (bind)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
        GLenum colorList[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, colorList);
    }
    // Unbind frame buffer and route nowhere.
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_NONE);
    }
    return true;
}

void GlslValidator::deleteProgram()
{
    if (_programId > 0)
    {
        glDeleteObjectARB(_programId);
        _programId = 0;
    }
}

unsigned int GlslValidator::createProgram(ErrorList& errors)
{
    errors.clear();

    deleteProgram();

    if (!haveValidStages())
    {
        errors.push_back("An invalid set of stages has been provided.");
        return 0;
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
    GLuint vertexShaderId = 0;
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
    GLuint fragmentShaderId = 0;
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

    return _programId;
}

bool GlslValidator::render(ErrorList& errors)
{
    errors.clear();

    if (_programId <= 0)
    {
        errors.push_back("No valid program to render with exists.");
        return false;
    }

    // Set up target
    bindTarget(true, errors);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up viewing / projection matrices for an orthographic rendering
    glViewport(0, 0, _frameBufferWidth, _frameBufferHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, _frameBufferWidth, 0.0, _frameBufferHeight, -100.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Bind program
    glUseProgram(_programId);

    // Draw simple geometry 
    {
        glPushMatrix();

        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 1.0f);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f(0.0f, (float)_frameBufferHeight);

        glTexCoord2f(0.0f, 0.0f);
        glNormal3f(1.0f, 0.0, 0.0);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glNormal3f(1.0f, 0.0, 0.0);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex2f((float)_frameBufferWidth, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glNormal3f(1.0f, 0.0, 0.0);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex2f((float)_frameBufferWidth, (float)_frameBufferHeight);

        glEnd();
        glPopMatrix();
    }

    // Unbind program
    glUseProgram(0);

    // Unset target
    bindTarget(false, errors);

    return true;
}

bool GlslValidator::save(std::string& /*fileName*/)
{
    return false;
}

}