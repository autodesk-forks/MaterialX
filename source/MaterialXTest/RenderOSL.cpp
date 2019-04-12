//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(MATERIALX_TEST_RENDER) 

#include <MaterialXRender/HardwarePlatform.h>

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXRender/Handlers/LightHandler.h>
#include <MaterialXRender/Util.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXRenderOsl/OslValidator.h>

#ifdef MATERIALX_BUILD_CONTRIB
#include <MaterialXContrib/Handlers/TinyEXRImageLoader.h>
#endif
#ifdef MATERIALX_BUILD_OIIO
#include <MaterialXRender/Handlers/OiioImageLoader.h>
#endif
#include <MaterialXRender/Handlers/StbImageLoader.h>

#include <MaterialXRender/Handlers/GeometryHandler.h>
#include <MaterialXRender/Handlers/TinyObjLoader.h>

#include <fstream>
#include <iostream>
#include <unordered_set>

#include <MaterialXTest/RenderUtil.h>

namespace mx = MaterialX;

#define LOG_TO_FILE

// Validator setup
static mx::OslValidatorPtr createOSLValidator(std::ostream& log)
{
    bool initialized = false;

    mx::OslValidatorPtr validator = mx::OslValidator::create();
    const std::string oslcExecutable(MATERIALX_OSLC_EXECUTABLE);
    validator->setOslCompilerExecutable(oslcExecutable);
    const std::string testRenderExecutable(MATERIALX_TESTRENDER_EXECUTABLE);
    validator->setOslTestRenderExecutable(testRenderExecutable);
    validator->setOslIncludePath(mx::FilePath(MATERIALX_OSL_INCLUDE_PATH));
    try
    {
        validator->initialize();
        validator->setImageHandler(nullptr);
        validator->setLightHandler(nullptr);
        initialized = true;

        // Pre-compile some required shaders for testrender
        if (!oslcExecutable.empty() && !testRenderExecutable.empty())
        {
            mx::FilePath shaderPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/");
            validator->setOslOutputFilePath(shaderPath);

            const std::string OSL_EXTENSION("osl");
            mx::FilePathVec files = shaderPath.getFilesInDirectory(OSL_EXTENSION);
            for (auto file : files)
            {
                mx::FilePath filePath = shaderPath / file;
                validator->compileOSL(filePath.asString());
            }

            // Set the search path for these compiled shaders.
            validator->setOslUtilityOSOPath(shaderPath);
        }
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(initialized);

    return validator;
}

// Validator execution
static void runOSLValidation(const std::string& shaderName, mx::TypedElementPtr element,
                             mx::OslValidator& validator, mx::GenContext& context, mx::DocumentPtr doc,
                             std::ostream& log, const RenderUtil::ShaderValidTestOptions& testOptions, RenderUtil::ShaderValidProfileTimes& profileTimes,
                             const mx::FileSearchPath& imageSearchPath, const std::string& outputPath=".")
{
    RenderUtil::AdditiveScopedTimer totalOSLTime(profileTimes.oslTimes.totalTime, "OSL total time");

    const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    // Perform validation if requested
    if (testOptions.validateElementToRender)
    {
        std::string message;
        if (!element->validate(&message))
        {
            log << "Element is invalid: " << message << std::endl;
            return;
        }
    }

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run OSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                RenderUtil::AdditiveScopedTimer genTimer(profileTimes.oslTimes.generationTime, "OSL generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                shader = shadergen.generate(shaderName, element, context);
            }
            catch (mx::Exception& e)
            {
                log << ">> " << e.what() << "\n";
                shader = nullptr;
            }
            CHECK(shader != nullptr);
            if (shader == nullptr)
            {
                log << ">> Failed to generate shader\n";
                return;
            }
            CHECK(shader->getSourceCode().length() > 0);

            std::string shaderPath;
            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                RenderUtil::AdditiveScopedTimer ioDir(profileTimes.oslTimes.ioTime, "OSL dir time");
                outputFilePath.createDirectory();
            }

            shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);

            // Write out osl file
            if (testOptions.dumpGeneratedCode)
            {
                RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.oslTimes.ioTime, "OSL io time");
                std::ofstream file;
                file.open(shaderPath + ".osl");
                file << shader->getSourceCode();
                file.close();
            }

            if (!testOptions.compileCode)
            {
                return;
            }

            // Validate
            bool validated = false;
            try
            {
                // Set output path and shader name
                validator.setOslOutputFilePath(outputFilePath);
                validator.setOslShaderName(shaderName);

                // Validate compilation
                {
                    RenderUtil::AdditiveScopedTimer compileTimer(profileTimes.oslTimes.compileTime, "OSL compile time");
                    validator.validateCreation(shader);
                }

                if (testOptions.renderImages)
                {
                    const mx::ShaderStage& stage = shader->getStage(mx::Stage::PIXEL);

                    // Look for textures and build parameter override string for each image
                    // files if a relative path maps to an absolute path
                    const mx::VariableBlock& uniforms = stage.getUniformBlock(mx::OSL::UNIFORMS);

                    mx::StringVec overrides;
                    mx::StringMap separatorMapper;
                    separatorMapper["\\\\"] = "/";
                    separatorMapper["\\"] = "/";
                    for (size_t i = 0; i<uniforms.size(); ++i)
                    {
                        const mx::ShaderPort* uniform = uniforms[i];
                        if (uniform->getType() != MaterialX::Type::FILENAME)
                        {
                            continue;
                        }
                        if (uniform->getValue())
                        {
                            const std::string& uniformName = uniform->getName();
                            mx::FilePath filename;
                            mx::FilePath origFilename(uniform->getValue()->getValueString());
                            if (!origFilename.isAbsolute())
                            {
                                filename = imageSearchPath.find(origFilename);
                                if (filename != origFilename)
                                {
                                    std::string overrideString("string " + uniformName + " \"" + filename.asString() + "\";\n");
                                    overrideString = mx::replaceSubstrings(overrideString, separatorMapper);
                                    overrides.push_back(overrideString);
                                }
                            }
                        }
                    }
                    validator.setShaderParameterOverrides(overrides);

                    const mx::VariableBlock& outputs = stage.getOutputBlock(mx::OSL::OUTPUTS);
                    if (outputs.size() > 0)
                    {
                        const mx::ShaderPort* output = outputs[0];
                        const mx::TypeSyntax& typeSyntax = shadergen.getSyntax().getTypeSyntax(output->getType());

                        const std::string& outputName = output->getName();
                        const std::string& outputType = typeSyntax.getTypeAlias().empty() ? typeSyntax.getName() : typeSyntax.getTypeAlias();

                        static const std::string SHADING_SCENE_FILE = "closure_color_scene.xml";
                        static const std::string NON_SHADING_SCENE_FILE = "constant_color_scene.xml";
                        const std::string& sceneTemplateFile = mx::elementRequiresShading(element) ? SHADING_SCENE_FILE : NON_SHADING_SCENE_FILE;

                        // Set shader output name and type to use
                        validator.setOslShaderOutput(outputName, outputType);

                        // Set scene template file. For now we only have the constant color scene file
                        mx::FilePath sceneTemplatePath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/");
                        sceneTemplatePath = sceneTemplatePath / sceneTemplateFile;
                        validator.setOslTestRenderSceneTemplateFile(sceneTemplatePath.asString());

                        // Validate rendering
                        {
                            RenderUtil::AdditiveScopedTimer renderTimer(profileTimes.oslTimes.renderTime, "OSL render time");
                            validator.validateRender();
                        }
                    }
                    else
                    {
                        CHECK(false);
                        log << ">> Shader has no output to render from\n";
                    }
                }

                validated = true;
            }
            catch (mx::ExceptionShaderValidationError& e)
            {
                // Always dump shader on error
                std::ofstream file;
                file.open(shaderPath + ".osl");
                file << shader->getSourceCode();
                file.close();

                for (auto error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump file: " << shaderPath << ".osl file" << std::endl;
            }
            catch (mx::Exception& e)
            {
                std::cout << e.what();
            }
            CHECK(validated);
        }
    }
}

TEST_CASE("Render: TestSuite", "[renderosl]")
{
    // Test has been turned off so just do nothing.
    // Check for an option file
    mx::FilePath path = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath optionsPath = path / mx::FilePath("_options.mtlx");
    RenderUtil::ShaderValidTestOptions options;
    if (!getTestOptions(optionsPath, options))
    {
        return;
    }
    if (!options.runOSLTests)
    {
        return;
    }

    // Profiling times
    RenderUtil::ShaderValidProfileTimes profileTimes;
    // Global setup timer
    RenderUtil::AdditiveScopedTimer totalTime(profileTimes.totalTime, "Global total time");

#ifdef LOG_TO_FILE
    std::ofstream oslLogfile("genosl_vanilla_render_test.txt");
    std::ostream& oslLog(oslLogfile);
    std::string docValidLogFilename = "genosl_vanilla_render_validate_doc.txt";
    std::ofstream docValidLogFile(docValidLogFilename);
    std::ostream& docValidLog(docValidLogFile);
    std::ofstream profilingLogfile("genosl_vanilla__render_profiling_log.txt");
    std::ostream& profilingLog(profilingLogfile);
#else
    std::ostream& oslLog(std::cout);
    std::string docValidLogFilename(std::out);
    std::ostream& docValidLog(std::cout);
    std::ostream& profilingLog(std::cout);
#endif

    // For debugging, add files to this set to override
    // which files in the test suite are being tested.
    // Add only the test suite filename not the full path.
    mx::StringSet testfileOverride;
    for (auto filterFile : options.overrideFiles)
    {
        testfileOverride.insert(filterFile);
    }

    RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.ioTime, "Global I/O time");
    mx::FilePathVec dirs;
    mx::FilePath baseDirectory = path;
    dirs = baseDirectory.getSubDirectories();

    ioTimer.endTimer();

    // Library search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");

    // Load in the library dependencies once
    // This will be imported in each test document below
    ioTimer.startTimer();
    mx::DocumentPtr dependLib = mx::createDocument();
    mx::StringSet excludeFiles;
    excludeFiles.insert("stdlib_" + mx::GlslShaderGenerator::LANGUAGE + "_impl.mtlx");
    excludeFiles.insert("stdlib_" + mx::GlslShaderGenerator::LANGUAGE + "_ogsfx_impl.mtlx");

    const mx::StringVec libraries = { "stdlib", "pbrlib" };
    GenShaderUtil::loadLibraries(libraries, searchPath, dependLib, &excludeFiles);
    GenShaderUtil::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/standard_surface.mtlx"), dependLib);
    ioTimer.endTimer();

    // Create validators and generators
    RenderUtil::AdditiveScopedTimer oslSetupTime(profileTimes.oslTimes.setupTime, "OSL setup time");

    mx::OslValidatorPtr oslValidator = createOSLValidator(oslLog);
    mx::ShaderGeneratorPtr oslShaderGenerator = mx::OslShaderGenerator::create();

    mx::GenContext oslContext(oslShaderGenerator);
    oslContext.registerSourceCodeSearchPath(searchPath);
    oslContext.registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

    mx::ColorManagementSystemPtr oslColorManagementSystem = mx::DefaultColorManagementSystem::create(oslShaderGenerator->getLanguage());
    oslColorManagementSystem->loadLibrary(dependLib);
    oslShaderGenerator->setColorManagementSystem(oslColorManagementSystem);

    oslSetupTime.endTimer();

    mx::CopyOptions importOptions;
    importOptions.skipDuplicateElements = true;

    // Map to replace "/" in Element path names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";

    RenderUtil::AdditiveScopedTimer validateTimer(profileTimes.validateTime, "Global validation time");
    RenderUtil::AdditiveScopedTimer renderableSearchTimer(profileTimes.renderableSearchTime, "Global renderable search time");

    mx::StringSet usedImpls;

    const std::string MTLX_EXTENSION("mtlx");
    const std::string OPTIONS_FILENAME("_options.mtlx");
    for (auto dir : dirs)
    {
        ioTimer.startTimer();
        mx::FilePathVec files;
        files = dir.getFilesInDirectory(MTLX_EXTENSION);
        ioTimer.endTimer();

        for (const std::string& file : files)
        {

            if (file == OPTIONS_FILENAME)
            {
                continue;
            }

            ioTimer.startTimer();
            // Check if a file override set is used and ignore all files
            // not part of the override set
            if (testfileOverride.size() && testfileOverride.count(file) == 0)
            {
                ioTimer.endTimer();
                continue;
            }

            const mx::FilePath filePath = mx::FilePath(dir) / mx::FilePath(file);
            const std::string filename = filePath;

            mx::DocumentPtr doc = mx::createDocument();
            mx::readFromXmlFile(doc, filename, dir);

            doc->importLibrary(dependLib, &importOptions);
            ioTimer.endTimer();

            validateTimer.startTimer();
            std::cout << "Validating MTLX file: " << filename << std::endl;
            if (options.runOSLTests)
                oslLog << "MTLX Filename: " << filename << std::endl;

            // Validate the test document
            std::string validationErrors;
            bool validDoc = doc->validate(&validationErrors);
            if (!validDoc)
            {
                docValidLog << filename << std::endl;
                docValidLog << validationErrors << std::endl;
            }
            validateTimer.endTimer();
            CHECK(validDoc);

            renderableSearchTimer.startTimer();
            std::vector<mx::TypedElementPtr> elements;
            try
            {
                mx::findRenderableElements(doc, elements);
            }
            catch (mx::Exception& e)
            {
                docValidLog << e.what() << std::endl;
                WARN("Find renderable elements failed, see: " + docValidLogFilename + " for details.");
            }
            renderableSearchTimer.endTimer();

            std::string outputPath = mx::FilePath(dir) / mx::FilePath(mx::removeExtension(file));
            mx::FileSearchPath imageSearchPath(dir);
            for (auto element : elements)
            {
                mx::OutputPtr output = element->asA<mx::Output>();
                mx::ShaderRefPtr shaderRef = element->asA<mx::ShaderRef>();
                mx::NodeDefPtr nodeDef = nullptr;
                if (output)
                {
                    nodeDef = output->getConnectedNode()->getNodeDef();
                }
                else if (shaderRef)
                {
                    nodeDef = shaderRef->getNodeDef();
                }
                if (nodeDef)
                {
                    mx::string elementName = mx::replaceSubstrings(element->getNamePath(), pathMap);
                    elementName = mx::createValidName(elementName);
                    if (options.runOSLTests)
                    {
                        renderableSearchTimer.startTimer();
                        mx::InterfaceElementPtr impl2 = nodeDef->getImplementation(oslShaderGenerator->getTarget(), oslShaderGenerator->getLanguage());
                        renderableSearchTimer.endTimer();
                        if (impl2)
                        {
                            if (options.checkImplCount)
                            {
                                mx::NodeGraphPtr nodeGraph = impl2->asA<mx::NodeGraph>();
                                mx::InterfaceElementPtr nodeGraphImpl = nodeGraph ? nodeGraph->getImplementation() : nullptr;
                                usedImpls.insert(nodeGraphImpl ? nodeGraphImpl->getName() : impl2->getName());
                            }
                            runOSLValidation(elementName, element, *oslValidator, oslContext, doc, oslLog, options, profileTimes, imageSearchPath, outputPath);
                        }
                    }
                }
            }
        }
    }

    // Dump out profiling information
    totalTime.endTimer();

    printRunLog(profileTimes, options, usedImpls, profilingLog, dependLib, oslContext,
                mx::OslShaderGenerator::LANGUAGE);
}

#endif
