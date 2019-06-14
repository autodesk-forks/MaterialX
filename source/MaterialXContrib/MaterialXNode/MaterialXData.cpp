#include "MaterialXData.h"
#include "Plugin.h"
#include "Util.h"

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

MaterialXData::MaterialXData(const std::string& materialXDocument, const std::string& elementPath)
{
	_librarySearchPath = Plugin::instance().getLibrarySearchPath();
	createDocument(materialXDocument);

	if (_document)
	{
		_element = _document->getDescendant(elementPath);
	}

    if (!_element)
    {
        throw MaterialX::Exception("Element not found");
    }
}

MaterialXData::~MaterialXData()
{
}

void MaterialXData::createDocument(const std::string& materialXDocument)
{
	// Create document
	_document = MaterialX::createDocument();
	MaterialX::readFromXmlFile(_document, materialXDocument);

	// Load libraries
	const MaterialX::StringVec libraries = { "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl" };
	MaterialX::loadLibraries(libraries, _librarySearchPath, _document);
}

bool MaterialXData::isValidOutput()
{
    if (!_element)
    {
        return false;
    }

	const std::string& elementPath = _element->getNamePath();
	std::vector<MaterialX::TypedElementPtr> elements;
	try {
		MaterialX::findRenderableElements(_document, elements);
		for (MaterialX::TypedElementPtr currentElement : elements)
		{
			std::string pathCompare(currentElement->getNamePath());
			if (pathCompare == elementPath)
			{
				return true;
			}
		}
	}
	catch (MaterialX::Exception& e)
	{
		std::cerr << "Failed to find renderable element in document: " << e.what() << std::endl;
	}
	return false;
}

bool MaterialXData::elementIsAShader() const
{
    return (_element ? _element->isA<MaterialX::ShaderRef>() : false);
}

void MaterialXData::createXMLWrapper()
{
	MaterialX::OutputPtr output = _element->asA<MaterialX::Output>();
	MaterialX::ShaderRefPtr shaderRef = _element->asA<MaterialX::ShaderRef>();
	if (!output && !shaderRef)
	{
		// Should never occur as we pre-filter renderables before creating the node + override
		throw MaterialX::Exception("MaterialXTextureOverride: Invalid type to create wrapper for");
	}
	else
	{
        std::unique_ptr<MaterialX::GenContext> glslContext{
            new MaterialX::GenContext(MaterialX::GlslShaderGenerator::create())
        };

        // Stop emission of environment map lookups.
        glslContext->registerSourceCodeSearchPath(_librarySearchPath);
        if (shaderRef)
        {
            glslContext->getOptions().hwSpecularEnvironmentMethod = MaterialX::SPECULAR_ENVIRONMENT_FIS;
            glslContext->getOptions().hwMaxActiveLightSources = 0;
        }
        else
        {
            glslContext->getOptions().hwSpecularEnvironmentMethod = MaterialX::SPECULAR_ENVIRONMENT_NONE;
        }
        glslContext->getOptions().fileTextureVerticalFlip = true;

        _xmlFragmentWrapper.reset(new MaterialX::OGSXMLFragmentWrapper(glslContext.get()));
        _xmlFragmentWrapper->setOutputVertexShader(false);

        // TODO: This just indicates that lighting is required. As direct lighting
        // is not supported, the requirement means that indirect lighting is required.
        // bool requiresLighting = (shaderRef != nullptr);
        std::cout << "MaterialXTextureOverride: Create XML wrapper" << std::endl;
        _xmlFragmentWrapper->createWrapper(_element);
        // Get the fragment name
        _fragmentName.set(_xmlFragmentWrapper->getFragmentName().c_str());

        _contexts.push_back(std::move(glslContext));
        
        // TODO: This just indicates that lighting is required. As direct lighting
		// is not supported, the requirement means that indirect lighting is required.
		// bool requiresLighting = (shaderRef != nullptr);
		std::cout << "MaterialXTextureOverride: Create XML wrapper" << std::endl;
		_xmlFragmentWrapper->createWrapper(_element);
	}
}

void MaterialXData::registerFragments()
{
	// Register fragments with the manager if needed
	//	
	if (MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer())
	{
        if (MHWRender::MFragmentManager* fragmentMgr = theRenderer->getFragmentManager())
		{
			const bool fragmentExists = (_xmlFragmentWrapper->getFragmentName().size() > 0)
                && fragmentMgr->hasFragment(_xmlFragmentWrapper->getFragmentName().c_str());

			if (!fragmentExists)
			{
                std::stringstream glslStream;
                _xmlFragmentWrapper->getDocument(glslStream);
                std::string xmlFileName(Plugin::instance().getResourcePath().asString() + "/standard_surface_default.xml");
                //std::string xmlFileName(Plugin::instance().getResourcePath().asString() + "/tiledImage.xml");

                // TODO: This should not be hard-coded
                std::string dumpPath("d:/work/shader_dump/");
                MaterialX::FileSearchPath path = MaterialX::getEnvironmentPath("TEMP");
                if (path.size() > 0)
                {
                    dumpPath = path[0].asString();
                }
                fragmentMgr->setEffectOutputDirectory(dumpPath.c_str());
                fragmentMgr->setIntermediateGraphOutputDirectory(dumpPath.c_str());
                _fragmentName = fragmentMgr->addShadeFragmentFromFile(xmlFileName.c_str(), false);

                if (_fragmentName.length() == 0)
                {
                    throw MaterialX::Exception("Failed to add OGS shader fragment from file.");
                }
			}
		}
	}
}
