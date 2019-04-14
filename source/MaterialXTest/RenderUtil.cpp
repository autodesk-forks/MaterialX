//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//


#include <MaterialXTest/RenderUtil.h>
#include <MaterialXTest/Catch/catch.hpp>

namespace mx = MaterialX;

namespace RenderUtil
{
void createLightRig(mx::DocumentPtr doc, mx::LightHandler& lightHandler, mx::GenContext& context,
                    const mx::FilePath& envIrradiancePath, const mx::FilePath& envRadiancePath)
{
    // Scan for lights
    std::vector<mx::NodePtr> lights;
    lightHandler.findLights(doc, lights);
    lightHandler.registerLights(doc, lights, context);

    // Set the list of lights on the with the generator
    lightHandler.setLightSources(lights);
    // Set up IBL inputs
    lightHandler.setLightEnvIrradiancePath(envIrradiancePath);
    lightHandler.setLightEnvRadiancePath(envRadiancePath);
}

// Create a list of generation options based on unit test options
// These options will override the original generation context options.
void getGenerationOptions(const ShaderValidTestOptions& testOptions,
    const mx::GenOptions& originalOptions,
    std::vector<mx::GenOptions>& optionsList)
{
    optionsList.clear();
    if (testOptions.shaderInterfaces & 1)
    {
        mx::GenOptions reducedOption = originalOptions;
        reducedOption.shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;
        optionsList.push_back(reducedOption);
    }
    // Alway fallback to complete if no options specified.
    if ((testOptions.shaderInterfaces & 2) || optionsList.empty())
    {
        mx::GenOptions completeOption = originalOptions;
        completeOption.shaderInterfaceType = mx::SHADER_INTERFACE_COMPLETE;
        optionsList.push_back(completeOption);
    }
}


bool getTestOptions(const std::string& optionFile, ShaderValidTestOptions& options)
{
    const std::string SHADER_VALID_TEST_OPTIONS_STRING("ShaderValidTestOptions");
    const std::string OVERRIDE_FILES_STRING("overrideFiles");
    const std::string LIGHT_FILES_STRING("lightFiles");
    const std::string SHADER_INTERFACES_STRING("shaderInterfaces");
    const std::string VALIDATE_ELEMENT_TO_RENDER_STRING("validateElementToRender");
    const std::string COMPILE_CODE_STRING("compileCode");
    const std::string RENDER_IMAGES_STRING("renderImages");
    const std::string SAVE_IMAGES_STRING("saveImages");
    const std::string DUMP_GLSL_UNIFORMS_AND_ATTRIBUTES_STRING("dumpGlslUniformsAndAttributes");
    const std::string RUN_OSL_TESTS_STRING("runOSLTests");
    const std::string RUN_GLSL_TESTS_STRING("runGLSLTests");
    const std::string RUN_OGSFX_TESTS_STRING("runOGSFXTests");
    const std::string CHECK_IMPL_COUNT_STRING("checkImplCount");
    const std::string DUMP_GENERATED_CODE_STRING("dumpGeneratedCode");
    const std::string GLSL_NONSHADER_GEOMETRY_STRING("glslNonShaderGeometry");
    const std::string GLSL_SHADER_GEOMETRY_STRING("glslShaderGeometry");
    const std::string RADIANCE_IBL_PATH_STRING("radianceIBLPath");
    const std::string IRRADIANCE_IBL_PATH_STRING("irradianceIBLPath");
    const std::string SPHERE_OBJ("sphere.obj");
    const std::string SHADERBALL_OBJ("shaderball.obj");

    options.overrideFiles.clear();
    options.dumpGeneratedCode = false;
    options.glslNonShaderGeometry = SPHERE_OBJ;
    options.glslShaderGeometry = SHADERBALL_OBJ;

    MaterialX::DocumentPtr doc = MaterialX::createDocument();
    try {
        MaterialX::readFromXmlFile(doc, optionFile);

        MaterialX::NodeDefPtr optionDefs = doc->getNodeDef(SHADER_VALID_TEST_OPTIONS_STRING);
        if (optionDefs)
        {
            for (MaterialX::ParameterPtr p : optionDefs->getParameters())
            {
                const std::string& name = p->getName();
                MaterialX::ValuePtr val = p->getValue();
                if (val)
                {
                    if (name == OVERRIDE_FILES_STRING)
                    {
                        options.overrideFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    if (name == LIGHT_FILES_STRING)
                    {
                        options.lightFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    else if (name == SHADER_INTERFACES_STRING)
                    {
                        options.shaderInterfaces = val->asA<int>();
                    }
                    else if (name == VALIDATE_ELEMENT_TO_RENDER_STRING)
                    {
                        options.validateElementToRender = val->asA<bool>();
                    }
                    else if (name == COMPILE_CODE_STRING)
                    {
                        options.compileCode = val->asA<bool>();
                    }
                    else if (name == RENDER_IMAGES_STRING)
                    {
                        options.renderImages = val->asA<bool>();
                    }
                    else if (name == SAVE_IMAGES_STRING)
                    {
                        options.saveImages = val->asA<bool>();
                    }
                    else if (name == DUMP_GLSL_UNIFORMS_AND_ATTRIBUTES_STRING)
                    {
                        options.dumpGlslUniformsAndAttributes = val->asA<bool>();
                    }
                    else if (name == RUN_OSL_TESTS_STRING)
                    {
                        options.runOSLTests = val->asA<bool>();
                    }
                    else if (name == RUN_GLSL_TESTS_STRING)
                    {
                        options.runGLSLTests = val->asA<bool>();
                    }
                    else if (name == RUN_OGSFX_TESTS_STRING)
                    {
                        options.runOGSFXTests = val->asA<bool>();
                    }
                    else if (name == CHECK_IMPL_COUNT_STRING)
                    {
                        options.checkImplCount = val->asA<bool>();
                    }
                    else if (name == DUMP_GENERATED_CODE_STRING)
                    {
                        options.dumpGeneratedCode = val->asA<bool>();
                    }
                    else if (name == GLSL_NONSHADER_GEOMETRY_STRING)
                    {
                        options.glslNonShaderGeometry = p->getValueString();
                    }
                    else if (name == GLSL_SHADER_GEOMETRY_STRING)
                    {
                        options.glslShaderGeometry = p->getValueString();
                    }
                    else if (name == RADIANCE_IBL_PATH_STRING)
                    {
                        options.radianceIBLPath = p->getValueString();
                    }
                    else if (name == IRRADIANCE_IBL_PATH_STRING)
                    {
                        options.irradianceIBLPath = p->getValueString();
                    }
                }
            }
        }

        // Disable render and save of images if not compiled code will be generated
        if (!options.compileCode)
        {
            options.renderImages = false;
            options.saveImages = false;
        }
        // Disable saving images, if no images are to be produced
        if (!options.renderImages)
        {
            options.saveImages = false;
        }

        // If there is a filter on the files to run turn off profile checking
        if (!options.overrideFiles.empty())
        {
            options.checkImplCount = false;
        }

        // If implementation count check is required, then OSL and GLSL/OGSFX
        // code generation must be executed to be able to check implementation usage.
        if (options.checkImplCount)
        {
            options.runGLSLTests = true;
            options.runOSLTests = true;
            options.runOGSFXTests = true;
        }
        return true;
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
    return false;
}

void printRunLog(const ShaderValidProfileTimes &profileTimes,
    const ShaderValidTestOptions& options,
    mx::StringSet& usedImpls,
    std::ostream& profilingLog,
    mx::DocumentPtr dependLib,
    mx::GenContext& context,
    const std::string& language)
{
    profileTimes.print(profilingLog);

    profilingLog << "---------------------------------------" << std::endl;
    options.print(profilingLog);

    if (options.checkImplCount)
    {
        profilingLog << "---------------------------------------" << std::endl;

        // Get implementation count from libraries. 
        std::set<mx::ImplementationPtr> libraryImpls;
        const std::vector<mx::ElementPtr>& children = dependLib->getChildren();
        for (auto child : children)
        {
            mx::ImplementationPtr impl = child->asA<mx::Implementation>();
            if (!impl)
            {
                continue;
            }

            // Only check implementations for languages we're interested in and
            // are testing.
            // 
            if (impl->getLanguage() == language)
            {
                libraryImpls.insert(impl);
            }
        }

        size_t skipCount = 0;
        profilingLog << "-- Possibly missed implementations ----" << std::endl;
        mx::StringVec whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue"
        };
        unsigned int implementationUseCount = 0;
        for (auto libraryImpl : libraryImpls)
        {
            const std::string& implName = libraryImpl->getName();

            // Skip white-list items
            bool inWhiteList = false;
            for (auto w : whiteList)
            {
                if (implName.find(w) != std::string::npos)
                {
                    skipCount++;
                    inWhiteList = true;
                    break;
                }
            }
            if (inWhiteList)
            {
                implementationUseCount++;
                continue;
            }

            if (usedImpls.count(implName))
            {
                implementationUseCount++;
                continue;
            }

            if (context.findNodeImplementation(implName))
            {
                implementationUseCount++;
                continue;
            }
            profilingLog << "\t" << implName << std::endl;
        }
        size_t libraryCount = libraryImpls.size();
        profilingLog << "Tested: " << implementationUseCount << " out of: " << libraryCount << " library implementations." << std::endl;
        // Enable when implementations and testing are complete
        // CHECK(implementationUseCount == libraryCount);
    }
}

bool ShaderRenderTester::testRender()
{
    // Test has been turned off so just do nothing.
    // Check for an option file
    mx::FilePath path = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath optionsPath = path / mx::FilePath("_options.mtlx");
    RenderUtil::ShaderValidTestOptions options;
    if (!getTestOptions(optionsPath, options))
    {
        return false;
    }
    bool run = runTest(options);
    if (!run)
    {
        return false;
    }

    // Profiling times
    RenderUtil::ShaderValidProfileTimes profileTimes;
    // Global setup timer
    RenderUtil::AdditiveScopedTimer totalTime(profileTimes.totalTime, "Global total time");

#ifdef LOG_TO_FILE
    const std::string prefex = logPrefixName();
    std::ofstream logfile(prefex + "_render_test.txt");
    std::ostream& log(logfile);
    std::string docValidLogFilename = prefex + "_render_validate_doc.txt";
    std::ofstream docValidLogFile(docValidLogFilename);
    std::ostream& docValidLog(docValidLogFile);
    std::ofstream profilingLogfile(prefex + "__render_profiling_log.txt");
    std::ostream& profilingLog(profilingLogfile);
#else
    std::ostream& log(std::cout);
    std::string docValidLogFilename = "std::cout";
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
    // Make this generic --- TODO 
    //excludeFiles.insert("stdlib_" + mx::GlslShaderGenerator::LANGUAGE + "_impl.mtlx");
    //excludeFiles.insert("stdlib_" + mx::GlslShaderGenerator::LANGUAGE + "_ogsfx_impl.mtlx");

    const mx::StringVec libraries = { "stdlib", "pbrlib" };
    GenShaderUtil::loadLibraries(libraries, searchPath, dependLib, &excludeFiles);
    GenShaderUtil::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/standard_surface.mtlx"), dependLib);
    ioTimer.endTimer();

    // Create validators and generators
    RenderUtil::AdditiveScopedTimer setupTime(profileTimes.languageTimes.setupTime, "Setup time");

    createShaderGenerator();
    createValidator(log);

    mx::ColorManagementSystemPtr colorManagementSystem = mx::DefaultColorManagementSystem::create(_shaderGenerator->getLanguage());
    colorManagementSystem->loadLibrary(dependLib);
    _shaderGenerator->setColorManagementSystem(colorManagementSystem);

    mx::GenContext context(_shaderGenerator);
    context.registerSourceCodeSearchPath(searchPath);
    registerSourceCodeSearchPaths(context);

    setupTime.endTimer();

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
                        mx::InterfaceElementPtr impl = nodeDef->getImplementation(_shaderGenerator->getTarget(), _shaderGenerator->getLanguage());
                        renderableSearchTimer.endTimer();
                        if (impl)
                        {
                            if (options.checkImplCount)
                            {
                                mx::NodeGraphPtr nodeGraph = impl->asA<mx::NodeGraph>();
                                mx::InterfaceElementPtr nodeGraphImpl = nodeGraph ? nodeGraph->getImplementation() : nullptr;
                                usedImpls.insert(nodeGraphImpl ? nodeGraphImpl->getName() : impl->getName());
                            }
                            runValidator(elementName, element, context, doc, log, options, profileTimes, imageSearchPath, outputPath);
                        }
                    }
                }
            }
        }
    }

    // Dump out profiling information
    totalTime.endTimer();
    printRunLog(profileTimes, options, usedImpls, profilingLog, dependLib, context,
        _shaderGenerator->getLanguage());
    return true;
}

} // namespace RenderUtil

