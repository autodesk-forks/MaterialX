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
	libSearchPath = Plugin::instance().getLibrarySearchPath();
	createDocument(materialXDocument);

	if (doc)
	{
		element = doc->getDescendant(elementPath);
	}

    if (!element)
    {
        throw MaterialX::Exception("Element not found");
    }

    std::unique_ptr<MaterialX::GenContext> glslContext{
        new MaterialX::GenContext(MaterialX::GlslShaderGenerator::create())
    };

	// Stop emission of environment map lookups.
	glslContext->getOptions().hwSpecularEnvironmentMethod = MaterialX::SPECULAR_ENVIRONMENT_NONE;
	glslContext->registerSourceCodeSearchPath(libSearchPath);

    glslFragmentWrapper.reset(new MaterialX::OGSXMLFragmentWrapper(glslContext.get()));
	glslFragmentWrapper->setOutputVertexShader(true);

    contexts.push_back(std::move(glslContext));
}

MaterialXData::~MaterialXData()
{
}

void MaterialXData::createDocument(const std::string& materialXDocument)
{
	// Create document
	doc = MaterialX::createDocument();
	MaterialX::readFromXmlFile(doc, materialXDocument);

	// Load libraries
	const MaterialX::StringVec libraries = { "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl" };
	MaterialX::loadLibraries(libraries, libSearchPath, doc);
}

bool MaterialXData::isValidOutput()
{
    if (!element)
    {
        return false;
    }

	const std::string& elementPath = element->getNamePath();
	std::vector<MaterialX::TypedElementPtr> elements;
	try {
		MaterialX::findRenderableElements(doc, elements);
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

void MaterialXData::createXMLWrapper()
{
	MaterialX::OutputPtr output = element->asA<MaterialX::Output>();
	MaterialX::ShaderRefPtr shaderRef = element->asA<MaterialX::ShaderRef>();
	if (!output && !shaderRef)
	{
		// Should never occur as we pre-filter renderables before creating the node + override
		throw MaterialX::Exception("MaterialXTextureOverride: Invalid type to create wrapper for");
	}
	else
	{
		// TODO: This just indicates that lighting is required. As direct lighting
		// is not supported, the requirement means that indirect lighting is required.
		// bool requiresLighting = (shaderRef != nullptr);
		std::cout << "MaterialXTextureOverride: Create XML wrapper" << std::endl;
		glslFragmentWrapper->createWrapper(element);
	}
}

void MaterialXData::registerFragments()
{
	// Register fragments with the manager if needed
	//
	MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
	if (theRenderer)
	{
		MHWRender::MFragmentManager* fragmentMgr =
			theRenderer->getFragmentManager();
		if (fragmentMgr)
		{
			bool fragmentExists = (glslFragmentWrapper->getFragmentName().size() > 0) && fragmentMgr->hasFragment(glslFragmentWrapper->getFragmentName().c_str());
			if (!fragmentExists)
			{
				std::stringstream glslStream;
				glslFragmentWrapper->getDocument(glslStream);
				std::string xmlFileName(Plugin::instance().getResourcePath().asString() + "/tiledImage.xml");
				fragmentName = fragmentMgr->addShadeFragmentFromFile(xmlFileName.c_str(), false);
			}
		}
	}
}
