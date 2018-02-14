
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GLew/glew.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GLUtil/SimpleWindow.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GLUtil/GLBaseContext.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslValidator.h>

#include <iostream>
#include <algorithm>

namespace MaterialX
{

GlslValidator::GlslValidator() :
    _programId(0),
    _colorTarget(0),
    _depthTarget(0),
    _frameBuffer(0),
    _frameBufferWidth(256),
    _frameBufferHeight(256),
    _uvBuffer(0),
    _indexBuffer(0),
    _indexBufferSize(0),
    _vertexArray(0),
    _initialized(false)
{
    _attributeLocations.resize(ATTRIBUTE_COUNT);
    std::fill(_attributeLocations.begin(), _attributeLocations.end(), -1);
    _attributeBuffers.resize(ATTRIBUTE_COUNT);
    std::fill(_attributeBuffers.begin(), _attributeBuffers.end(), 0);
}

GlslValidator::~GlslValidator()
{
    deleteProgram();
    deleteTarget();

    GLBaseContext* context = GLBaseContext::get();
    if (context)
        GLBaseContext::destroy();
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
        // Creeate window
        SimpleWindow window;
        const char* windowName = "Validator Window";
        bool created = window.create(const_cast<char *>(windowName), 
                                    _frameBufferWidth, _frameBufferHeight, 
                                    nullptr);
        if (!created)
        {
            errors.push_back("Failed to create window for testing.");
        }
        else
        {
            // Create offscreen context
            GLBaseContext* context = GLBaseContext::create(window.windowWrapper(), nullptr);
            if (!context)
            {
                errors.push_back("Failed to create OpenGL context for testing.");
            }
            else
            {
                if (context->makeCurrent())
                {
                    // Initialize glew
                    bool initializedFunctions = true;
#ifndef OSMac_
                    glewInit();
                    if (!glewIsSupported("GL_VERSION_4_0"))
                    {
                        initializedFunctions = false;
                        errors.push_back("OpenGL version 4.0 not supported");
                    }
#endif
                    if (initializedFunctions)
                    {
                        glClearColor(0, 0, 0, 1);
                        glClearStencil(0);

                        _initialized = true;
                    }
                }
            }
        }
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
        GLBaseContext* context = GLBaseContext::get();
        if (context && context->makeCurrent())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &_frameBuffer);
        }
    }
}

bool GlslValidator::createTarget(ErrorList& errors)
{
    GLBaseContext* context = GLBaseContext::get();
    if (!context)
    {
        errors.push_back("No valid OpenGL context to create target with.");
    }
    if (!context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create target with.");
    }

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
        GLBaseContext* context = GLBaseContext::get();
        if (context && context->makeCurrent())
        {
            glDeleteObjectARB(_programId);
        }
        _programId = 0;
    }
}

unsigned int GlslValidator::createProgram(ErrorList& errors)
{
    errors.clear();

    GLBaseContext* context = GLBaseContext::get();
    if (!context)
    {
        errors.push_back("No valid OpenGL context to create program with.");
    }
    if (!context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create program.");
    }

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

const GlslValidator::ProgramInputList& GlslValidator::createUniformsList()
{
    _uniformList.clear();

    if (_programId <= 0)
    {
        //errors.push_back("Cannot bind matrices without a valid program");
        return _uniformList;
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
            ProgramInputPtr inputPtr = std::make_shared<ProgramInput>(uniformLocation, uniformType);
            _uniformList[std::string(uniformName)] = inputPtr;
            //std::cout << "Scanned uniform : " << uniformName << ". Location: " << uniformLocation << ". Type: " << uniformType << std::endl;
        }
    }
    delete uniformName;

    return _uniformList;
}

const GlslValidator::ProgramInputList& GlslValidator::createAttributesList()
{
    _attributeList.clear();

    if (_programId <= 0)
    {
        //errors.push_back("Cannot bind matrices without a valid program");
        return _attributeList;
    }

    GLint numAttributes = 0;
    GLint maxNameLength = 0;
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &numAttributes);
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);
    char* attributeName = new char[maxNameLength];

    for (int i = 0; i < numAttributes; i++)
    {
        GLint attribueSize = 0;
        GLenum attributeType = 0;
        glGetActiveAttrib(_programId, GLuint(i), maxNameLength, nullptr, &attribueSize, &attributeType, attributeName);
        GLint attributeLocation = glGetAttribLocation(_programId, attributeName);
        if (attributeLocation >= 0)
        {
            ProgramInputPtr inputPtr = std::make_shared<ProgramInput>(attributeLocation, attributeType);
            _attributeList[std::string(attributeName)] = inputPtr;
            //std::cout << "Scanned attribute : " << attributeName << ". Location: " << attributeLocation << ". Type: " << attributeType << std::endl;
        }
    }
    delete attributeName;

    return _attributeList;
}


bool GlslValidator::bindMatrices(ErrorList& errors, const HwShader* hwShader)
{
    if (_programId <= 0)
    {
        errors.push_back("Cannot bind matrices without a valid program");
        return false;
    }

    // Pull information from HwShader
    if (hwShader)
    {
        return false;
    }

    // Set projection and modelview matrices
    GLfloat m[16];
    GLint location = -1;
    auto programInput = _uniformList.find("u_viewProjectionMatrix");
    if (programInput != _uniformList.end())
    {
        location = programInput->second->_location;
        if (location >= 0)
        {
            //std::cout << "Bound u_viewProjectionMatrix" << std::endl;
            glGetFloatv(GL_PROJECTION_MATRIX, m);
            glUniformMatrix4fv(location, 1, GL_FALSE, m);
        }
    }
    programInput = _uniformList.find("u_modelMatrix");
    if (programInput != _uniformList.end())
    {
        location = programInput->second->_location;
        if (location >= 0)
        {
            //std::cout << "Bound u_modelMatrix" << std::endl;
            glGetFloatv(GL_MODELVIEW_MATRIX, m);
            glUniformMatrix4fv(location, 1, GL_FALSE, m);
        }
    }
    programInput = _uniformList.find("u_worldMatrix");
    if (programInput != _uniformList.end())
    {
        location = programInput->second->_location;
        if (location >= 0)
        {
            //std::cout << "Bound u_worldMatrix" << std::endl;
            glGetFloatv(GL_MODELVIEW_MATRIX, m);
            glUniformMatrix4fv(location, 1, GL_FALSE, m);
        }
    } 

    // Set normal transform matrix (world-inverse-transpose) and other matrices
    // TO ADD...

    checkErrors(errors);

    return true;
}

bool GlslValidator::updateAttribute(const GLfloat* bufferData, size_t bufferSize,
    const std::string& attributeId,
    const GlslValidator::AttributeIndex attributeIndex,
    unsigned int floatCount)
{
    int location = -1;
    auto programInput = _attributeList.find(attributeId);
    if (programInput != _attributeList.end())
    {
        location = programInput->second->_location;
        if (location >= 0)
        {
            // Create a buffer
            unsigned int buffer = 0;
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData, GL_STATIC_DRAW);

            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location, floatCount, GL_FLOAT, GL_FALSE, 0, 0);

            _attributeLocations[attributeIndex] = location;
            _attributeBuffers[attributeIndex] = buffer;
            return true;
        }
    }
    return false;
}

bool GlslValidator::bindGeometry(ErrorList& errors, const HwShader* hwShader)
{
    if (_programId <= 0)
    {
        errors.push_back("Cannot bind geometry without a valid program");
        return false;
    }

    // Pull information from HwShader
    if (hwShader)
    {
        return false;
    }

    // Pull information from program directly
    {
        // Set up vertex arrays 
        glGenVertexArrays(1, &_vertexArray);
        glBindVertexArray(_vertexArray);

        unsigned int indexData[] = { 0, 1, 2, 0, 2, 3 };
        _indexBufferSize = 6;
        glGenBuffers(1, &_indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

        // Create positions
        const GLfloat positionData[] = { 0.0f, 0.0f, 0.0f,
            0.0f, (float)_frameBufferWidth, 0.0f,
            (float)_frameBufferWidth, (float)_frameBufferHeight, 0.0f,
            (float)_frameBufferWidth, 0.0f, 0.0f };
        updateAttribute(positionData, sizeof(positionData), "i_position", POSITION_ATTRIBUTE, 3);

        // Create normals
        float normalData[] = { 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f };
        updateAttribute(normalData, sizeof(normalData), "i_normal", NORMAL_ATTRIBUTE, 3);

        // Create tangent
        float tangentData[] = { 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f };
        updateAttribute(tangentData, sizeof(tangentData), "i_tangent", NORMAL_ATTRIBUTE, 3);

        // Create bitangent
        float bitangentData[] = { 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f };
        updateAttribute(bitangentData, sizeof(bitangentData), "i_bitangent", BITANGENT_ATTRIBUTE, 3);

        // Create colors
        float colorData[] = { 1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f };
        updateAttribute(colorData, sizeof(colorData), "i_color0", COLOR_ATTRIBUTE, 4);

        // Create texture coords
        GLfloat uvData[] = { 0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f };
        _uvLocations.clear();
        for (unsigned int i = 0; i < 10; i++)
        {
            int uvLocation = -1;
            std::string texcoordLabel = "i_texcoord" + std::to_string(i);
            auto programInput = _attributeList.find(texcoordLabel);
            if (programInput != _attributeList.end())
            {
                uvLocation = glGetAttribLocation(_programId, texcoordLabel.c_str());
                if (uvLocation >= 0)
                {
                    _uvLocations.push_back(uvLocation);
                    if (_uvBuffer == 0)
                    {
                        glGenBuffers(1, &_uvBuffer);
                        glBindBuffer(GL_ARRAY_BUFFER, _uvBuffer);
                        glBufferData(GL_ARRAY_BUFFER, sizeof(uvData), uvData, GL_STATIC_DRAW);
                    }
                    else
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, _uvBuffer);
                    }
                    glEnableVertexAttribArray(uvLocation);
                    glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
                }
            }
        }
    }

    checkErrors(errors);
    return true;
}

bool GlslValidator::bindTextures(ErrorList& errors, const HwShader* hwShader)
{
    if (_programId <= 0)
    {
        errors.push_back("Cannot bind textures without a valid program");
        return false;
    }

    // Pull information from HwShader
    if (hwShader)
    {
        return false;
    }

    // Pull information from program directly
    else
    {
        int textureUnit = 0;
        GLint maxImageUnits = -1;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxImageUnits);
        for (auto uniform : _uniformList)
        {
            GLenum uniformType = uniform.second->_type;
            GLint uniformLocation = uniform.second->_location;
            if (uniformLocation >= 0 &&
                uniformType >= GL_SAMPLER_1D && uniformType <= GL_SAMPLER_CUBE)
            {
                if (textureUnit >= maxImageUnits)
                {
                    break;
                }

                // Map location to a texture unit incrementally
                //glUniform1i(uniformLocation, textureUnit);
                //glActiveTexture(GL_TEXTURE0 + textureUnit);
                //glBindTexture(uniformType, 0); // Currently there is no way to determine what to bind here yet.
                textureUnit++;
                //std::cout << "Bind sample:" << uniform.first << ". Location: " << uniformLocation << "Type: " << uniformType << std::endl;
            }
        }
    }

    checkErrors(errors);
    return true;
}


void GlslValidator::unbindGeometry(ErrorList& errors)
{
    // Cleanup attribute bindings
    //
    glBindVertexArray(0);
    int numberAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numberAttributes);
    for (int i = 0; i < numberAttributes; i++)
    {
        glDisableVertexAttribArray(i);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Clean up buffers
    //
    if (_indexBuffer > 0)
    {
        glDeleteBuffers(1, &_indexBuffer);
        _indexBuffer = 0;
    }
    for (unsigned int i=0; i<ATTRIBUTE_COUNT; i++)
    {
        unsigned int bufferId = _attributeBuffers[i];
        if (bufferId > 0)
        {
            glDeleteBuffers(1, &bufferId);
            _attributeBuffers[i] = 0;
        }
    }
    if (_uvBuffer > 0)
    {
        glDeleteBuffers(1, &_uvBuffer);
        _uvBuffer = 0;
    }

    // Reset program locations
    std::fill(_attributeLocations.begin(), _attributeLocations.end(), -1);
    _uvLocations.clear();

    checkErrors(errors);
}

bool GlslValidator::render(ErrorList& errors)
{
    errors.clear();

    GLBaseContext* context = GLBaseContext::get();
    if (!context)
    {
        errors.push_back("No valid OpenGL context to render to.");
        return false;
    }
    if (!context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to render to.");
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

    // Bind program and input parameters
    if (_programId > 0)
    {
        // Check if we have any attributes to bind. If not then
        // there is nothing to draw
        GLint activeAttributeCount = 0;
        glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &activeAttributeCount);
        
        if (activeAttributeCount <= 0)
        {
            errors.push_back("Program has no input vertex data.");
        }
        else
        {
            // Bind the program to use
            glUseProgram(_programId);
            checkErrors(errors);

            createUniformsList();
            createAttributesList();

            if (bindMatrices(errors, nullptr) &&
                bindGeometry(errors, nullptr) &&
                bindTextures(errors, nullptr))
            {
                // Draw
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
                glDrawElements(GL_TRIANGLES, (GLsizei)_indexBufferSize, GL_UNSIGNED_INT, (void*)0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                checkErrors(errors);
            }

            // Unbind the program and cleanup geometry
            glUseProgram(0);
            unbindGeometry(errors);
        }
    }

    // Draw simple geometry 
    else
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

    // Unset target
    bindTarget(false, errors);

    return true;
}

bool GlslValidator::save(std::string& /*fileName*/)
{
    return false;
}

void GlslValidator::checkErrors(ErrorList& errors)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        errors.push_back("OpenGL error: " + std::to_string(error));
    }
}


}