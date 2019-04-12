//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(MATERIALX_TEST_RENDER) 

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
#include <MaterialXRenderGlsl/GlslValidator.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>

#ifdef MATERIALX_BUILD_CONTRIB
#include <MaterialXContrib/Handlers/TinyEXRImageLoader.h>
#endif
#ifdef MATERIALX_BUILD_OIIO
#include <MaterialXRender/Handlers/OiioImageLoader.h>
#endif
#include <MaterialXRender/Handlers/StbImageLoader.h>

#include <MaterialXRender/Handlers/GeometryHandler.h>
#include <MaterialXRender/Handlers/TinyObjLoader.h>

#include <MaterialXTest/RenderUtil.h>

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <ctime>

namespace mx = MaterialX;

#define LOG_TO_FILE

//
// Create a validator with an image and geometry handler
// If a filename is supplied then a stock geometry of that name will be used if it can be loaded.
// By default if the file can be loaded it is assumed that rendering is done using a perspective
// view vs an orthographic view. This flag argument is updated and returned.
//
static mx::GlslValidatorPtr createGLSLValidator(const std::string& fileName, std::ostream& log)
{
    bool initialized = false;
    mx::GlslValidatorPtr validator = mx::GlslValidator::create();
    mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
    mx::GLTextureHandlerPtr imageHandler = mx::GLTextureHandler::create(stbLoader);
#ifdef MATERIALX_BUILD_CONTRIB
    mx::TinyEXRImageLoaderPtr exrLoader = mx::TinyEXRImageLoader::create();
    imageHandler->addLoader(exrLoader);
#endif
    try
    {
        validator->initialize();
        validator->setImageHandler(imageHandler);
        validator->setLightHandler(nullptr);
        mx::GeometryHandlerPtr geometryHandler = validator->getGeometryHandler();
        std::string geometryFile;
        if (fileName.length())
        {
            geometryFile = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/") / mx::FilePath(fileName);
            if (!geometryHandler->hasGeometry(geometryFile))
            {
                geometryHandler->clearGeometry();
                geometryHandler->loadGeometry(geometryFile);
            }
        }
        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    catch (mx::Exception& e)
    {
        log << e.what() << std::endl;
    }
    REQUIRE(initialized);

    return validator;
}

// Test by connecting it to a supplied element
// 1. Create the shader and checks for source generation
// 2. Writes doc to disk if valid
// 3. Writes vertex and pixel shaders to disk
// 4. Validates creation / compilation of shader program
// 5. Validates that inputs were created properly
// 6. Validates rendering
// 7. Saves rendered image to disk
//
// Outputs error log if validation fails
//
static void runGLSLValidation(const std::string& shaderName, mx::TypedElementPtr element, mx::GlslValidator& validator,
                              mx::GenContext& context, const mx::LightHandlerPtr lightHandler,
                              mx::DocumentPtr doc, std::ostream& log, const RenderUtil::ShaderValidTestOptions& testOptions, 
                              RenderUtil::ShaderValidProfileTimes& profileTimes,
                              const mx::FileSearchPath& imageSearchPath, const std::string& outputPath=".")
{
    RenderUtil::AdditiveScopedTimer totalGLSLTime(profileTimes.languageTimes.totalTime, "GLSL total time");

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
        log << "------------ Run GLSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                RenderUtil::AdditiveScopedTimer ioDir(profileTimes.languageTimes.ioTime, "GLSL dir time");
                outputFilePath.createDirectory();
            }

            std::string shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);
            mx::ShaderPtr shader;
            try
            {
                RenderUtil::AdditiveScopedTimer transpTimer(profileTimes.languageTimes.transparencyTime, "GLSL transparency time");
                options.hwTransparency = mx::isTransparentSurface(element, shadergen);
                transpTimer.endTimer();

                RenderUtil::AdditiveScopedTimer generationTimer(profileTimes.languageTimes.generationTime, "GLSL generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                contextOptions.fileTextureVerticalFlip = true;
                shader = shadergen.generate(shaderName, element, context);
                generationTimer.endTimer();
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
            const std::string& vertexSourceCode = shader->getSourceCode(mx::Stage::VERTEX);
            const std::string& pixelSourceCode = shader->getSourceCode(mx::Stage::PIXEL);
            CHECK(vertexSourceCode.length() > 0);
            CHECK(pixelSourceCode.length() > 0);

            if (testOptions.dumpGeneratedCode)
            {
                RenderUtil::AdditiveScopedTimer dumpTimer(profileTimes.languageTimes.ioTime, "GLSL io time");
                std::ofstream file;
                file.open(shaderPath + "_vs.glsl");
                file << vertexSourceCode;
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << pixelSourceCode;
                file.close();
            }

            if (!testOptions.compileCode)
            {
                return;
            }

            // Validate
            MaterialX::GlslProgramPtr program = validator.program();
            bool validated = false;
            try
            {
                mx::GeometryHandlerPtr geomHandler = validator.getGeometryHandler();

                bool isShader = mx::elementRequiresShading(element);
                if (isShader)
                {
                    mx::FilePath geomPath;
                    if (!testOptions.glslShaderGeometry.isEmpty())
                    {
                        if (!testOptions.glslShaderGeometry.isAbsolute())
                        {
                            geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry") / testOptions.glslShaderGeometry;
                        }
                        else
                        {
                            geomPath = testOptions.glslShaderGeometry;
                        }
                    }
                    else
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/shaderball.obj");
                    }
                    if (!geomHandler->hasGeometry(geomPath))
                    {
                        geomHandler->clearGeometry();
                        geomHandler->loadGeometry(geomPath);
                    }
                    validator.setLightHandler(lightHandler);
                }
                else
                {
                    mx::FilePath geomPath;
                    if (!testOptions.glslNonShaderGeometry.isEmpty())
                    {
                        if (!testOptions.glslNonShaderGeometry.isAbsolute())
                        {
                            geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry") / testOptions.glslNonShaderGeometry;
                        }
                        else
                        {
                            geomPath = testOptions.glslNonShaderGeometry;
                        }
                    }
                    else
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/sphere.obj");
                    }
                    if (!geomHandler->hasGeometry(geomPath))
                    {
                        geomHandler->clearGeometry();
                        geomHandler->loadGeometry(geomPath);
                    }
                    validator.setLightHandler(nullptr);
                }

                {
                    RenderUtil::AdditiveScopedTimer compileTimer(profileTimes.languageTimes.compileTime, "GLSL compile time");
                    validator.validateCreation(shader);
                    validator.validateInputs();
                }

                if (testOptions.dumpGlslUniformsAndAttributes)
                {
                    RenderUtil::AdditiveScopedTimer printTimer(profileTimes.languageTimes.ioTime, "GLSL io time");
                    log << "* Uniform:" << std::endl;
                    program->printUniforms(log);
                    log << "* Attributes:" << std::endl;
                    program->printAttributes(log);

                    log << "* Uniform UI Properties:" << std::endl;
                    const std::string& target = shadergen.getTarget();
                    const MaterialX::GlslProgram::InputMap& uniforms = program->getUniformsList();
                    for (auto uniform : uniforms)
                    {
                        const std::string& path = uniform.second->path;
                        if (path.empty())
                        {
                            continue;
                        }

                        mx::UIProperties uiProperties;
                        if (getUIProperties(path, doc, target, uiProperties) > 0)
                        {
                            log << "Program Uniform: " << uniform.first << ". Path: " << path;
                            if (!uiProperties.uiName.empty())
                                log << ". UI Name: \"" << uiProperties.uiName << "\"";
                            if (!uiProperties.uiFolder.empty())
                                log << ". UI Folder: \"" << uiProperties.uiFolder << "\"";
                            if (!uiProperties.enumeration.empty())
                            {
                                log << ". Enumeration: {";
                                for (size_t i = 0; i<uiProperties.enumeration.size(); i++)
                                    log << uiProperties.enumeration[i] << " ";
                                log << "}";
                            }
                            if (!uiProperties.enumerationValues.empty())
                            {
                                log << ". Enum Values: {";
                                for (size_t i = 0; i < uiProperties.enumerationValues.size(); i++)
                                    log << uiProperties.enumerationValues[i]->getValueString() << "; ";
                                log << "}";
                            }
                            if (uiProperties.uiMin)
                                log << ". UI Min: " << uiProperties.uiMin->getValueString();
                            if (uiProperties.uiMax)
                                log << ". UI Max: " << uiProperties.uiMax->getValueString();
                            log << std::endl;
                        }
                    }
                }

                if (testOptions.renderImages)
                {
                    {
                        RenderUtil::AdditiveScopedTimer renderTimer(profileTimes.languageTimes.renderTime, "GLSL render time");
                        validator.getImageHandler()->setSearchPath(imageSearchPath);
                        validator.validateRender(!isShader);
                    }

                    if (testOptions.saveImages)
                    {
                        RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.languageTimes.imageSaveTime, "GLSL image save time");
                        std::string fileName = shaderPath + "_glsl.png";
                        validator.save(fileName, false);
                    }
                }

                validated = true;
            }
            catch (mx::ExceptionShaderValidationError& e)
            {
                // Always dump shader stages on error
                std::ofstream file;
                file.open(shaderPath + "_vs.glsl");
                file << shader->getSourceCode(mx::Stage::VERTEX);
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << shader->getSourceCode(mx::Stage::PIXEL);
                file.close();

                for (auto error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump files: " << shaderPath << "_ps.glsl and _vs.glsl files" << std::endl;
            }
            catch (mx::Exception& e)
            {
                log << e.what() << std::endl;
            }
            CHECK(validated);
        }
    }
}

TEST_CASE("Render: GLSL TestSuite", "[renderglsl]")
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
    if (!options.runGLSLTests)
    {
        return;
    }

    // Profiling times
    RenderUtil::ShaderValidProfileTimes profileTimes;
    // Global setup timer
    RenderUtil::AdditiveScopedTimer totalTime(profileTimes.totalTime, "Global total time");

#ifdef LOG_TO_FILE
    const std::string PREFIX("genglsl");
   std::ofstream logfile(PREFIX + "_render_test.txt");
    std::ostream& log(logfile);
    std::string docValidLogFilename = PREFIX + "_render_validate_doc.txt";
    std::ofstream docValidLogFile(docValidLogFilename);
    std::ostream& docValidLog(docValidLogFile);
    std::ofstream profilingLogfile(PREFIX + "__render_profiling_log.txt");
    std::ostream& profilingLog(profilingLogfile);
#else
    std::ostream& log(std::cout);
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
    excludeFiles.insert("stdlib_" + mx::GlslShaderGenerator::LANGUAGE + "_ogsfx_impl.mtlx");
    excludeFiles.insert("stdlib_osl_impl.mtlx");
    excludeFiles.insert("stdlib_" + mx::OslShaderGenerator::LANGUAGE + "_impl.mtlx");

    const mx::StringVec libraries = { "stdlib", "pbrlib" };
    GenShaderUtil::loadLibraries(libraries, searchPath, dependLib, &excludeFiles);
    GenShaderUtil::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/standard_surface.mtlx"), dependLib);

    mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/Lights");
    if (options.lightFiles.size() == 0)
    {
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("lightcompoundtest.mtlx"), dependLib);
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("lightcompoundtest_ng.mtlx"), dependLib);
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("light_rig.mtlx"), dependLib);
    }
    else
    {
        for (auto lightFile : options.lightFiles)
        {
            GenShaderUtil::loadLibrary(lightDir / mx::FilePath(lightFile), dependLib);
        }
    }
    ioTimer.endTimer();

    // Create validators and generators
    RenderUtil::AdditiveScopedTimer setupTime(profileTimes.languageTimes.setupTime, "Setup time");

    mx::GlslValidatorPtr validator = createGLSLValidator("sphere.obj", log);
    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();

    mx::ColorManagementSystemPtr colorManagementSystem = mx::DefaultColorManagementSystem::create(shaderGenerator->getLanguage());
    colorManagementSystem->loadLibrary(dependLib);
    shaderGenerator->setColorManagementSystem(colorManagementSystem);

    mx::GenContext context(shaderGenerator);
    context.registerSourceCodeSearchPath(searchPath);

    setupTime.endTimer();

    mx::CopyOptions importOptions;
    importOptions.skipDuplicateElements = true;

    mx::LightHandlerPtr glslLightHandler = nullptr;
    {
        RenderUtil::AdditiveScopedTimer glslSetupLightingTimer(profileTimes.languageTimes.setupTime, "GLSL setup lighting time");
        // Add lights as a dependency
        glslLightHandler = mx::LightHandler::create();
        RenderUtil::createLightRig(dependLib, *glslLightHandler, context,
                           options.radianceIBLPath, options.irradianceIBLPath);
    }

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
            log << "MTLX Filename: " << filename << std::endl;

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
                    {
                        renderableSearchTimer.startTimer();
                        mx::InterfaceElementPtr impl = nodeDef->getImplementation(shaderGenerator->getTarget(), shaderGenerator->getLanguage());
                        renderableSearchTimer.endTimer();
                        if (impl)
                        {
                            if (options.checkImplCount)
                            {
                                mx::NodeGraphPtr nodeGraph = impl->asA<mx::NodeGraph>();
                                mx::InterfaceElementPtr nodeGraphImpl = nodeGraph ? nodeGraph->getImplementation() : nullptr;
                                usedImpls.insert(nodeGraphImpl ? nodeGraphImpl->getName() : impl->getName());
                            }
                            runGLSLValidation(elementName, element, *validator, context, glslLightHandler, doc, log, options, profileTimes, imageSearchPath, outputPath);
                        }
                    }
                }
            }
        }
    }

    // Dump out profiling information
    totalTime.endTimer();
    printRunLog(profileTimes, options, usedImpls, profilingLog, dependLib, context,
                mx::GlslShaderGenerator::LANGUAGE);
}

#endif
