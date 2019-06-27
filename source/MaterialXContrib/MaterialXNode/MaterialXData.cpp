#include "MaterialXData.h"
#include "MaterialXUtil.h"

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenShader/Util.h>

namespace
{
mx::DocumentPtr createDocument( const std::string& materialXDocumentPath,
                                const MaterialX::FileSearchPath& librarySearchPath )
{
    // Create document
    mx::DocumentPtr document = mx::createDocument();
    if (!document)
    {
        throw mx::Exception("Failed to create a MaterialX document");
    }

    // Load libraries
    static const mx::StringVec libraries = { "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl" };
    MaterialXMaya::loadLibraries(libraries, librarySearchPath, document);

    // Read document contents from disk
    mx::XmlReadOptions readOptions;
    readOptions.skipDuplicateElements = true;
    mx::readFromXmlFile(document, materialXDocumentPath, mx::EMPTY_STRING, &readOptions);

    return document;
}
}

MaterialXData::MaterialXData(   mx::DocumentPtr document,
                                const std::string& elementPath, 
                                const MaterialX::FileSearchPath& librarySearchPath )
    : _document(document)
    , _shaderGenerator(mx::GlslFragmentGenerator::create())
    , _genContext(_shaderGenerator)
{
    if (!_document)
    {
        throw mx::Exception("No document specified");
    }

    std::vector<mx::TypedElementPtr> renderableElements;
    mx::findRenderableElements(_document, renderableElements);

    // Nothing specified. Find the first renderable element and use that
    if (elementPath.length() == 0)
    {
        if (renderableElements.empty())
        {
            throw mx::Exception(
                "No element path specified and no renderable elements in the document."
            );
        }

        _element = renderableElements.front();
    }
    else
    {
        _element = _document->getDescendant(elementPath);

        auto it = std::find_if(
            renderableElements.begin(),
            renderableElements.end(),
            [this](mx::TypedElementPtr renderableElement) -> bool
            {
                return _element->getNamePath() == renderableElement->getNamePath();
            }
        );

        if (it == renderableElements.end())
        {
            throw mx::Exception("The specified element is not renderable");
        }
    }

    _genContext.registerSourceCodeSearchPath(librarySearchPath);
}

MaterialXData::MaterialXData(   const std::string& materialXDocumentPath,
                                const std::string& elementPath,
                                const MaterialX::FileSearchPath& librarySearchPath )
    : MaterialXData(::createDocument(materialXDocumentPath, librarySearchPath),
                    elementPath,
                    librarySearchPath )
{}


MaterialXData::~MaterialXData()
{
}

const std::string& MaterialXData::getFragmentName() const
{
    return _fragmentName;
}

const std::string& MaterialXData::getFragmentSource() const
{
    return _fragmentSource;
}

const mx::StringMap& MaterialXData::getPathInputMap() const
{
    return _pathInputMap;
}

bool MaterialXData::elementIsAShader() const
{
    return _element && _element->isA<mx::ShaderRef>();
}

void MaterialXData::generateXml()
{
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
    _genContext.getOptions().hwSpecularEnvironmentMethod =
        shaderRef ? mx::SPECULAR_ENVIRONMENT_FIS : mx::SPECULAR_ENVIRONMENT_NONE;

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
        _fragmentSource = stream.str();
        if (_fragmentSource.empty())
        {
            return;
        }
        else
        {
            // Strip out any '\r' characters.
            _fragmentSource.erase(
                std::remove(_fragmentSource.begin(), _fragmentSource.end(), '\r'), _fragmentSource.end()
            );
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
