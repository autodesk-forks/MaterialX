#include <MaterialXData.h>
#include <Util.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenShader/Util.h>

MaterialXData::MaterialXData(const std::string& materialXDocumentPath, 
                             const std::string& elementPath, 
                             const MaterialX::FileSearchPath& librarySearchPath)
    : _shaderGenerator(mx::GlslFragmentGenerator::create()),
      _genContext(_shaderGenerator),
      _librarySearchPath(librarySearchPath)
{
    setRenderableElement(materialXDocumentPath, elementPath);
}

bool MaterialXData::setRenderableElement(const std::string& materialXDocument, const std::string& elementPath)
{
    clearXml();
    _document = nullptr;
    _element = nullptr;

    createDocument(materialXDocument);
    if (_document)
    {
        // Nothing specified. Find the first renderable element and use that
        if (elementPath.length() == 0)
        {
            std::vector<mx::TypedElementPtr> elements;
            mx::findRenderableElements(_document, elements);
            if (!elements.empty())
            {
                _element = elements[0];
                return true;
            }
        }
        else
        {
            _element = _document->getDescendant(elementPath);
            return isRenderable();
        }
    }
    return false;
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
    MaterialXMaya::loadLibraries(libraries, _librarySearchPath, _document);

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

bool MaterialXData::isRenderable()
{
    if (!_element)
    {
        return false;
    }

    const std::string& elementPath = _element->getNamePath();
    std::vector<mx::TypedElementPtr> elements;
    try 
    {
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
        throw mx::Exception("Failed to find renderable element in document: " + std::string(e.what()));
    }
    return false;
}
