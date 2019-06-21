#include "MaterialXData.h"
#include "Plugin.h"
#include "Util.h"

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

namespace mx = MaterialX;

MaterialXData::MaterialXData(const std::string& materialXDocumentPath, const std::string& elementPath)
    : _shaderGenerator(mx::GlslFragmentGenerator::create()),
      _genContext(_shaderGenerator)
{
    // This should be settable instead of hard-coded.
    _librarySearchPath = Plugin::instance().getLibrarySearchPath();
    setData(materialXDocumentPath, elementPath);
}

bool MaterialXData::setData(const std::string& materialXDocument, const std::string& elementPath)
{
    clearXml();
    createDocument(materialXDocument);
    if (_document)
    {
        _element = _document->getDescendant(elementPath);
    }

    // Check that the element is renderable
    return isRenderable();
}

MaterialXData::~MaterialXData()
{
}

const std::string& MaterialXData::getFragmentName() const
{
    return _fragmentName;
}

const std::string& MaterialXData::getFragmentWrapper() const
{
    return _fragmentWrapper;
}

const mx::StringMap& MaterialXData::getPathInputMap() const
{
    return _pathInputMap;
}

void MaterialXData::createDocument(const std::string& materialXDocumentPath)
{
    // Create document
    _document = mx::createDocument();

    // Load libraries
    static const mx::StringVec libraries = { "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl" };
    mx::loadLibraries(libraries, _librarySearchPath, _document);

    // Read document contents from disk
    mx::XmlReadOptions readOptions;
    readOptions.skipDuplicateElements = true;
    mx::readFromXmlFile(_document, materialXDocumentPath);
}

bool MaterialXData::elementIsAShader() const
{
    return (_element ? _element->isA<mx::ShaderRef>() : false);
}

void MaterialXData::clearXml()
{
    _pathInputMap.clear();
    _fragmentName.clear();
    _fragmentWrapper.clear();
}

void MaterialXData::generateXml()
{
    // Reset cached data
    clearXml();

    if (!_element)
    {
        return;
    }

    mx::OutputPtr output = _element->asA<mx::Output>();
    mx::ShaderRefPtr shaderRef = _element->asA<mx::ShaderRef>();
    if (!output && !shaderRef)
    {
        // Should never occur as we pre-filter renderables before creating the node + override
        throw mx::Exception("Invalid element to create wrapper for " + _element->getName());
    }

    // Set up generator context. For shaders use FIS environment lookup,
    // but disable this for textures to avoid additional unneeded XML parameter
    // generation.
    _genContext.registerSourceCodeSearchPath(_librarySearchPath);
    if (shaderRef)
    {
        _genContext.getOptions().hwSpecularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_FIS;
    }
    else
    {
        _genContext.getOptions().hwSpecularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_NONE;
    }
    _genContext.getOptions().hwMaxActiveLightSources = 0;
    // For Maya we need to insert a V-flip fragment
    _genContext.getOptions().fileTextureVerticalFlip = true;

    // Generate shader and put into XML wrapper.
    // TODO: Generate unique fragment names
    _fragmentName = _element->getNamePath();
    _fragmentName = mx::createValidName(_fragmentName);
    mx::ShaderPtr shader = _shaderGenerator->generate(_fragmentName, _element, _genContext);
    if (shader)
    {
        std::ostringstream stream;
        // Note: This name must match the the fragment name used for registration
        // or the registration will fail.
        _generator.generate(_fragmentName, shader.get(), nullptr, stream);
        _fragmentWrapper = stream.str();
        if (_fragmentWrapper.empty())
        {
            return;
        }
        else
        {
            // Strip out any '\r' characters.
            _fragmentWrapper.erase(std::remove(_fragmentWrapper.begin(), _fragmentWrapper.end(), '\r'), _fragmentWrapper.end());
        }

        // Extract out the input fragment parameter names along with their
        // associated Element paths to allow for value binding.
        const mx::ShaderStage& ps = shader->getStage(mx::Stage::PIXEL);
        for (const auto& blockMap : ps.getUniformBlocks())
        {
            const mx::VariableBlock& uniforms = *blockMap.second;

            // Skip light uniforms
            if (uniforms.getName() == mx::HW::LIGHT_DATA)
            {
                continue;
            }

            for (size_t i = 0; i < uniforms.size(); i++)
            {
                const mx::ShaderPort* port = uniforms[i];
                const std::string& path = port->getPath();
                if (!path.empty())
                {
                    std::string name = port->getVariable();
                    if (port->getType()->getSemantic() == mx::TypeDesc::SEMANTIC_FILENAME)
                    {
                        // Strip out the "sampler" post-fix to get the texture name.
                        size_t pos = name.find(mx::OgsXmlGenerator::SAMPLER_SUFFIX);
                        name.erase(pos, mx::OgsXmlGenerator::SAMPLER_SUFFIX.length());
                    }
                    _pathInputMap[path] = name;
                }
            }
        }
    }
}

void MaterialXData::registerFragments(const std::string& ogsXmlPath)
{
    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    MHWRender::MFragmentManager* fragmentManager = theRenderer ? theRenderer->getFragmentManager() : nullptr;
    if (!fragmentManager)
    {
        return;
    }
        
    // TODO: This should not be hard-coded
    std::string dumpPath("d:/work/shader_dump/");
    mx::FileSearchPath path = mx::getEnvironmentPath("TEMP");
    if (path.size() > 0)
    {
        dumpPath = path[0].asString();
    }
    fragmentManager->setEffectOutputDirectory(dumpPath.c_str());
    fragmentManager->setIntermediateGraphOutputDirectory(dumpPath.c_str());

    // Register fragments with the manager if needed
    const std::string& fragmentString = getFragmentWrapper();
    const std::string& fragmentName = getFragmentName();
    if (fragmentName.empty() || fragmentString.empty())
    {
        return;
    }

    const bool fragmentExists = fragmentManager->hasFragment(fragmentName.c_str());
    if (!fragmentExists)
    {
        MString fragmentNameM;

        // Allow for an explicit XML file to be specified.
        if (!ogsXmlPath.empty())
        {
            std::string xmlFileName(Plugin::instance().getResourcePath() / ogsXmlPath);
            fragmentNameM = fragmentManager->addShadeFragmentFromFile(xmlFileName.c_str(), false);
        }

        // When no override file is specified use the generated XML
        else
        {
            fragmentNameM = fragmentManager->addShadeFragmentFromBuffer(fragmentString.c_str(), false);
        }

        // TODO: Add a fallback shader.
        if (fragmentNameM.length() == 0)
        {
            throw mx::Exception("Failed to add shader fragment." + getFragmentName());
        }
    }
}

bool MaterialXData::isRenderable()
{
    if (!_element)
    {
        return false;
    }

    const std::string& elementPath = _element->getNamePath();
    std::vector<mx::TypedElementPtr> elements;
    try {
        mx::findRenderableElements(_document, elements);
        for (mx::TypedElementPtr currentElement : elements)
        {
            std::string pathCompare(currentElement->getNamePath());
            if (pathCompare == elementPath)
            {
                return true;
            }
        }
    }
    catch (mx::Exception& e)
    {
        std::cerr << "Failed to find renderable element in document: " << e.what() << std::endl;
    }
    return false;
}
