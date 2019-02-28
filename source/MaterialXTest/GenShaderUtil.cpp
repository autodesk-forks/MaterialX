
#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/GenShaderUtil.h>

namespace mx = MaterialX;

namespace GenShaderUtil
{
//static const std::string LIGHT_SHADER_TYPE = "lightshader";

//
// Load in a library. Sets the URI before import
//
void loadLibrary(const mx::FilePath& file, mx::DocumentPtr doc)
{
    mx::DocumentPtr libDoc = mx::createDocument();
    mx::readFromXmlFile(libDoc, file);
    libDoc->setSourceUri(file);
    mx::CopyOptions copyOptions;
    copyOptions.skipDuplicateElements = true;
    doc->importLibrary(libDoc, &copyOptions);
}

//
// Loads all the MTLX files below a given library path
//
void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc,
                   const std::set<std::string>* excludeFiles)
{
    const std::string MTLX_EXTENSION("mtlx");
    for (const std::string& library : libraryNames)
    {
        mx::StringVec librarySubPaths;
        mx::FilePath libraryPath = searchPath / library;
        mx::getSubDirectories(libraryPath, librarySubPaths);

        for (auto path : librarySubPaths)
        {
            mx::StringVec filenames;
            mx::getFilesInDirectory(path, filenames, MTLX_EXTENSION);

            for (const std::string& filename : filenames)
            {
                if (excludeFiles && excludeFiles->count(filename))
                {
                    continue;
                }
                loadLibrary(mx::FilePath(path)/ filename, doc);
            }
        }
    }
    REQUIRE(doc->getNodeDefs().size() > 0);
}

//
// Get source content, source path and resolved paths for
// an implementation
//
bool getShaderSource(mx::ShaderGeneratorPtr generator,
                    const mx::ImplementationPtr implementation,
                    mx::FilePath& sourcePath,
                    mx::FilePath& resolvedPath,
                    std::string& sourceContents)
{
    if (implementation)
    {
        sourcePath = implementation->getFile();
        resolvedPath = generator->findSourceCode(sourcePath);
        if (mx::readFile(resolvedPath.asString(), sourceContents))
        {
            return true;
        }
    }
    return false;
}

void registerLightType(mx::DocumentPtr doc, mx::HwShaderGenerator& shadergen, const mx::GenOptions& options)
{
    // Scan for lights
    std::vector<mx::NodePtr> lights;
    for (mx::NodePtr node : doc->getNodes())
    {
        if (node->getType() == LIGHT_SHADER_TYPE)
        {
            lights.push_back(node);
        }
    }
    if (!lights.empty())
    {
        // Create a list of unique nodedefs and ids for them
        std::unordered_map<std::string, unsigned int> identifiers;
        mx::mapNodeDefToIdentiers(lights, identifiers);
        for (auto id : identifiers)
        {
            mx::NodeDefPtr nodeDef = doc->getNodeDef(id.first);
            if (nodeDef)
            {
                shadergen.bindLightShader(*nodeDef, id.second, options);
            }
        }
    }
}

// Check that implementations exist for all nodedefs supported per generator
void checkImplementations(mx::ShaderGeneratorPtr generator, std::set<std::string> generatorSkipNodeTypes, 
                          std::set<std::string> generatorSkipNodeDefs)
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);

    std::string generatorId = generator->getLanguage() + "_" + generator->getTarget();
    std::string fileName = generatorId + "_implementation_check.txt";

    std::filebuf implDumpBuffer;
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    generator->registerSourceCodeSearchPath(searchPath);

    const std::string& language = generator->getLanguage();
    const std::string& target = generator->getTarget();

    // Node types to explicitly skip temporarily.
    std::set<std::string> skipNodeTypes =
    {
        "ambientocclusion",
        "arrayappend",
        "curveadjust",
    };
    skipNodeTypes.insert(generatorSkipNodeTypes.begin(), generatorSkipNodeTypes.end());

    // Explicit set of node defs to skip temporarily
    std::set<std::string> skipNodeDefs =
    {
        "ND_add_displacementshader",
        "ND_add_volumeshader",
        "ND_multiply_displacementshaderF",
        "ND_multiply_displacementshaderV",
        "ND_multiply_volumeshaderF",
        "ND_multiply_volumeshaderC",
        "ND_mix_displacementshader",
        "ND_mix_volumeshader"
    };
    skipNodeDefs.insert(generatorSkipNodeDefs.begin(), generatorSkipNodeDefs.end());

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Scanning language: " << language << ". Target: " << target << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    std::vector<mx::ImplementationPtr> impls = doc->getImplementations();
    implDumpStream << "Existing implementations: " << std::to_string(impls.size()) << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    for (auto impl : impls)
    {
        if (language == impl->getLanguage())
        {
            std::string msg("Impl: ");
            msg += impl->getName();
            std::string targetName = impl->getTarget();
            if (targetName.size())
            {
                msg += ", target: " + targetName;
            }
            else
            {
                msg += ", target: NONE ";
            }
            mx::NodeDefPtr nodedef = impl->getNodeDef();
            if (!nodedef)
            {
                std::string nodedefName = impl->getNodeDefString();
                msg += ". Does NOT have a nodedef with name: " + nodedefName;
            }
            implDumpStream << msg << std::endl;
        }
    }

    std::string nodeDefNode;
    std::string nodeDefType;
    unsigned int count = 0;
    unsigned int missing = 0;
    unsigned int skipped = 0;
    std::string missing_str;
    std::string found_str;

    // Scan through every nodedef defined
    for (mx::NodeDefPtr nodeDef : doc->getNodeDefs())
    {
        count++;

        const std::string& nodeDefName = nodeDef->getName();
        const std::string& nodeName = nodeDef->getNodeString();

        if (skipNodeTypes.count(nodeName))
        {
            found_str += "Temporarily skipping implementation required for nodedef: " + nodeDefName + ", Node : " + nodeName + ".\n";
            skipped++;
            continue;
        }
        if (skipNodeDefs.count(nodeDefName))
        {
            found_str += "Temporarily skipping implementation required for nodedef: " + nodeDefName + ", Node : " + nodeName + ".\n";
            skipped++;
            continue;
        }

        if (!requiresImplementation(nodeDef))
        {
            found_str += "No implementation required for nodedef: " + nodeDefName + ", Node: " + nodeName + ".\n";
            continue;
        }

        mx::InterfaceElementPtr inter = nodeDef->getImplementation(target, language);
        if (!inter)
        {
            missing++;
            missing_str += "Missing nodeDef implementation: " + nodeDefName + ", Node: " + nodeName + ".\n";

            std::vector<mx::InterfaceElementPtr> inters = doc->getMatchingImplementations(nodeDefName);
            for (auto inter2 : inters)
            {
                mx::ImplementationPtr impl = inter2->asA<mx::Implementation>();
                if (impl)
                {
                    std::string msg("\t Cached Impl: ");
                    msg += impl->getName();
                    msg += ", nodedef: " + impl->getNodeDefString();
                    msg += ", target: " + impl->getTarget();
                    msg += ", language: " + impl->getLanguage();
                    missing_str += msg + ".\n";
                }
            }

            for (auto childImpl : impls)
            {
                if (childImpl->getNodeDefString() == nodeDefName)
                {
                    std::string msg("\t Doc Impl: ");
                    msg += childImpl->getName();
                    msg += ", nodedef: " + childImpl->getNodeDefString();
                    msg += ", target: " + childImpl->getTarget();
                    msg += ", language: " + childImpl->getLanguage();
                    missing_str += msg + ".\n";
                }
            }

        }
        else
        {
            mx::ImplementationPtr impl = inter->asA<mx::Implementation>();
            if (impl)
            {
                // Test if the generator has an interal implementation first
                if (generator->implementationRegistered(impl->getName()))
                {
                    found_str += "Found generator impl for nodedef: " + nodeDefName + ", Node: "
                        + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                }

                // Check for an implementation explicitly stored
                else
                {
                    mx::FilePath sourcePath, resolvedPath;
                    std::string contents;
                    if (!getShaderSource(generator, impl, sourcePath, resolvedPath, contents))
                    {
                        missing++;
                        missing_str += "Missing source code: " + sourcePath.asString() + " for nodeDef: "
                            + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                    }
                    else
                    {
                        found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                            + nodeName + +". Impl: " + impl->getName() + "Path: " + resolvedPath.asString() + ".\n";
                    }
                }
            }
            else
            {
                mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                    + nodeName + ". Graph Impl: " + graph->getName();
                mx::InterfaceElementPtr graphNodeDefImpl = graph->getImplementation();
                if (graphNodeDefImpl)
                {
                    found_str += ". Graph Nodedef Impl: " + graphNodeDefImpl->getName();
                }
                found_str += ".\n";
            }
        }
    }

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Missing: " << missing << " implementations out of: " << count << " nodedefs. Skipped: " << skipped << std::endl;
    implDumpStream << missing_str << std::endl;
    implDumpStream << found_str << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    // Should have 0 missing including skipped
    REQUIRE(missing == 0);
    REQUIRE(skipped == 38);

    implDumpBuffer.close();
}

void testUniqueNames(mx::ShaderGeneratorPtr shaderGenerator, const std::string& stage)
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "unique_names";

    // Generate a shader with an internal node having the same name as the shader,
    // which will result in a name conflict between the shader output and the
    // internal node output
    const std::string shaderName = "unique_names";
    const std::string nodeName = shaderName;

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    mx::OutputPtr output1 = nodeGraph->addOutput("out", "color3");
    mx::NodePtr node1 = nodeGraph->addNode("noise2d", nodeName, "color3");

    output1->setConnectedNode(node1);

    // Set the output to a restricted name
    const std::string outputQualifier = shaderGenerator->getSyntax()->getOutputQualifier();
    output1->setName(outputQualifier);

    mx::GenOptions options;
    mx::ShaderPtr shader = shaderGenerator->generate(shaderName, output1, options);
    REQUIRE(shader != nullptr);
    REQUIRE(shader->getSourceCode(stage).length() > 0);

    // Make sure the output and internal node output has their variable names set
    mx::ShaderGraphOutputSocket* sgOutputSocket = shader->getGraph()->getOutputSocket();
    REQUIRE(sgOutputSocket->variable != outputQualifier);
    mx::ShaderNode* sgNode1 = shader->getGraph()->getNode(node1->getName());
    REQUIRE(sgNode1->getOutput()->variable == "unique_names_out");
}

bool generateCode(mx::ShaderGenerator& shaderGenerator, const std::string& shaderName, mx::TypedElementPtr element,
                  const mx::GenOptions& options, std::ostream& log, std::vector<std::string>testStages)
{
    mx::ShaderPtr shader = nullptr;
    try
    {
        //AdditiveScopedTimer genTimer(profileTimes.oslTimes.generationTime, "OSL generation time");
        shader = shaderGenerator.generate(shaderName, element, options);
    }
    catch (mx::ExceptionShaderGenError& e)
    {
        log << ">> " << e.what() << "\n";
        shader = nullptr;
    }
    CHECK(shader);
    if (!shader)
    {
        INFO(">> Failed to generate shader for element: " + element->getNamePath());
        log << ">> Failed to generate shader for element: " << element->getNamePath() << std::endl;
        return false;
    }
    
    bool stageFailed = false;
    for (auto stage : testStages)
    {
        bool noSource = shader->getSourceCode(stage).empty();
        CHECK(!noSource);
        if (noSource)
        {
            log << ">> Failed to generate source code for stage: " << stage << std::endl;
            stageFailed = true;
        }
    }
    return !stageFailed;
}

} 
