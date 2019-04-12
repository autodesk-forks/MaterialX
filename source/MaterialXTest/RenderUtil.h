//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <MaterialXRender/Util.h>
#include <MaterialXRender/Handlers/LightHandler.h>

#include <MaterialXTest/GenShaderUtil.h>

#include <chrono>
#include <ctime>

namespace mx = MaterialX;

namespace RenderUtil
{
void createLightRig(mx::DocumentPtr doc, mx::LightHandler& lightHandler, mx::GenContext& context,
    const mx::FilePath& envIrradiancePath, const mx::FilePath& envRadiancePath);

//
// Shader validation options structure
//
class ShaderValidTestOptions
{
public:
    void print(std::ostream& output) const
    {
        output << "Shader Validation Test Options:" << std::endl;
        output << "\tOverride Files: ";
        for (auto overrideFile : overrideFiles)
        {
            output << overrideFile << " ";
        }
        output << std::endl;
        output << "\tLight Setup Files: ";
        for (auto lightFile : lightFiles)
        {
            output << lightFile << " ";
        }
        output << std::endl;
        output << "\tRun GLSL Tests: " << runGLSLTests << std::endl;
        output << "\tRun OGSFX Tests: " << runOGSFXTests << std::endl;
        output << "\tRun OSL Tests: " << runOSLTests << std::endl;
        output << "\tCheck Implementation Usage Count: " << checkImplCount << std::endl;
        output << "\tDump Generated Code: " << dumpGeneratedCode << std::endl;
        output << "\tShader Interfaces: " << shaderInterfaces << std::endl;
        output << "\tValidate Element To Render: " << validateElementToRender << std::endl;
        output << "\tCompile code: " << compileCode << std::endl;
        output << "\tRender Images: " << renderImages << std::endl;
        output << "\tSave Images: " << saveImages << std::endl;
        output << "\tDump GLSL Uniforms and Attributes  " << dumpGlslUniformsAndAttributes << std::endl;
        output << "\tGLSL Non-Shader Geometry: " << glslNonShaderGeometry.asString() << std::endl;
        output << "\tGLSL Shader Geometry: " << glslShaderGeometry.asString() << std::endl;
        output << "\tRadiance IBL File Path " << radianceIBLPath.asString() << std::endl;
        output << "\tIrradiance IBL File Path: " << irradianceIBLPath.asString() << std::endl;
    }

    // Filter list of files to only run validation on.
    MaterialX::StringVec overrideFiles;

    // Comma separated list of light setup files
    mx::StringVec lightFiles;

    // Set to true to always dump generated code to disk
    bool dumpGeneratedCode = false;

    // Execute GLSL tests
    bool runGLSLTests = true;

    // Execute OGSFX tests
    bool runOGSFXTests = false;

    // Execute OSL tests
    bool runOSLTests = true;

    // Check the count of number of implementations used
    bool checkImplCount = true;

    // Run using a set of interfaces:
    // - 3 = run complete + reduced.
    // - 2 = run complete only (default)
    // - 1 = run reduced only.
    int shaderInterfaces = 2;

    // Validate element before attempting to generate code. Default is false.
    bool validateElementToRender = false;

    // Perform source code compilation validation test
    bool compileCode = true;

    // Perform rendering validation test
    bool renderImages = true;

    // Perform saving of image. Can only be disabled for GLSL tests.
    bool saveImages = true;

    // Set this to be true if it is desired to dump out GLSL uniform and attribut information to the logging file.
    bool dumpGlslUniformsAndAttributes = true;

    // Non-shader GLSL geometry file
    MaterialX::FilePath glslNonShaderGeometry;

    // Shader GLSL geometry file
    MaterialX::FilePath glslShaderGeometry;

    // Radiance IBL file
    MaterialX::FilePath radianceIBLPath;

    // IradianceIBL file
    MaterialX::FilePath irradianceIBLPath;
};

// Per language profile times
class LanguageProfileTimes
{
public:
    void print(const std::string& label, std::ostream& output) const
    {
        output << label << std::endl;
        output << "\tTotal: " << totalTime << " seconds" << std::endl;;
        output << "\tSetup: " << setupTime << " seconds" << std::endl;;
        output << "\tTransparency: " << transparencyTime << " seconds" << std::endl;;
        output << "\tGeneration: " << generationTime << " seconds" << std::endl;;
        output << "\tCompile: " << compileTime << " seconds" << std::endl;
        output << "\tRender: " << renderTime << " seconds" << std::endl;
        output << "\tI/O: " << ioTime << " seconds" << std::endl;
        output << "\tImage save: " << imageSaveTime << " seconds" << std::endl;
    }
    double totalTime = 0.0;
    double setupTime = 0.0;
    double transparencyTime = 0.0;
    double generationTime = 0.0;
    double compileTime = 0.0;
    double renderTime = 0.0;
    double ioTime = 0.0;
    double imageSaveTime = 0.0;
};

//
// Shader validation profiling structure
//
class ShaderValidProfileTimes
{
public:
    void print(std::ostream& output) const
    {
        output << "Overall time: " << languageTimes.totalTime << " seconds" << std::endl;
        output << "\tI/O time: " << ioTime << " seconds" << std::endl;
        output << "\tValidation time: " << validateTime << " seconds" << std::endl;
        output << "\tRenderable search time: " << renderableSearchTime << " seconds" << std::endl;

        languageTimes.print("GLSL Profile Times:", output);

        output << "Elements tested: " << elementsTested << std::endl;
    }

    LanguageProfileTimes languageTimes;
    double totalTime = 0;
    double ioTime = 0.0;
    double validateTime = 0.0;
    double renderableSearchTime = 0.0;
    unsigned int elementsTested = 0;
};

// Scoped timer which adds a duration to a given externally reference timing duration
class AdditiveScopedTimer
{
public:
    AdditiveScopedTimer(double& durationRefence, const std::string& label)
        : _duration(durationRefence)
        , _label(label)
    {
        startTimer();
    }

    ~AdditiveScopedTimer()
    {
        endTimer();
    }

    void startTimer()
    {
        _startTime = std::chrono::system_clock::now();

        if (_debugUpdate)
        {
            std::cout << "Start time for timer (" << _label << ") is: " << _duration << std::endl;
        }
    }

    void endTimer()
    {
        std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
        std::chrono::duration<double> timeDuration = endTime - _startTime;
        double currentDuration = timeDuration.count();
        _duration += currentDuration;
        _startTime = endTime;

        if (_debugUpdate)
        {
            std::cout << "Current duration for timer (" << _label << ") is: " << currentDuration << ". Total duration: " << _duration << std::endl;
        }
    }

protected:
    double& _duration;
    bool _debugUpdate = false;
    std::string _label;
    std::chrono::time_point<std::chrono::system_clock> _startTime;
};


// Create a list of generation options based on unit test options
// These options will override the original generation context options.
void getGenerationOptions(const ShaderValidTestOptions& testOptions,
    const mx::GenOptions& originalOptions,
    std::vector<mx::GenOptions>& optionsList);

bool getTestOptions(const std::string& optionFile, ShaderValidTestOptions& options);

void printRunLog(const ShaderValidProfileTimes &profileTimes,
    const ShaderValidTestOptions& options,
    mx::StringSet& usedImpls,
    std::ostream& profilingLog,
    mx::DocumentPtr dependLib,
    mx::GenContext& context,
    const std::string& language);

} // namespace RenderUtil

#endif

