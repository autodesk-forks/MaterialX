#include <MaterialXView/ShaderValidators/Osl/OslValidator.h>
#include <MaterialXView/Handlers/ObjGeometryHandler.h>

#include <iostream>
#include <algorithm>

namespace MaterialX
{
//
// Creator
//
OslValidatorPtr OslValidator::create()
{
    return std::shared_ptr<OslValidator>(new OslValidator());
}

OslValidator::OslValidator() :
    ShaderValidator()
{
}

OslValidator::~OslValidator()
{
}

void OslValidator::initialize()
{
}

void OslValidator::validateCreation(const ShaderPtr shader)
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL compilation error.");

    bool haveCompiler = false;
    if (!haveCompiler)
    {
        errors.push_back("No valid OSL compiler found.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    bool shaderCompiled = false;
    if (!shaderCompiled)
    {
        errors.push_back("Shader failed to compile");
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void OslValidator::validateCreation(const std::vector<std::string>& /*stages*/)
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL compilation error.");

    std::string oslcCommand("oslc -q"); // Should be user defined. Want in quite mode to catch errors
    std::string oslIncludePath("d:/Work/arnold/Arnold-SDK/osl/include"); // Should be user defined
    // Get source for stage and write to file temporarily
    std::string oslFileName("conditionals.osl");
    std::string errorFile("conditionals_error.txt");
    std::string redirectString(" 2> & 1");

    // Run the command and get back the result. If non-empty string throw exception with error
    std::string command = oslcCommand + " -I" + oslIncludePath + " " + oslFileName + " > " +
        errorFile + redirectString;
    int result = std::system(command.c_str());
    if (result != 0)
    {
        errors.push_back("No valid OSL compiler found.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    bool shaderCompiled = false;
    if (!shaderCompiled)
    {
        errors.push_back("Shader failed to compile");
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void OslValidator::validateInputs()
{
    // No input validation at this time.
}

void OslValidator::validateRender(bool /*orthographicView*/)
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL rendering error.");

    errors.push_back("OSL rendering not supported at this time.");
    throw ExceptionShaderValidationError(errorType, errors);
}

void OslValidator::save(const std::string& /*fileName*/)
{
    ShaderValidationErrorList errors;
    const std::string errorType("OSL image save error.");

    if (!_imageHandler)
    {
        errors.push_back("No image handler specified.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

}
