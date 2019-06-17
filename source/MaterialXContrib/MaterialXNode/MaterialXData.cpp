#include "MaterialXData.h"
#include "Plugin.h"
#include "Util.h"

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>

MaterialXData::MaterialXData(const std::string& materialXDocument, const std::string& elementPath)    
    : _genContext(MaterialX::GlslShaderGenerator::create())
{
	_librarySearchPath = Plugin::instance().getLibrarySearchPath();
    setData(materialXDocument, elementPath);
}

bool MaterialXData::setData(const std::string& materialXDocument, const std::string& elementPath)
{
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
    return _xmlFragmentWrapper.getFragmentName();
}

void MaterialXData::getXML(std::ostream& stream) const
{
    _xmlFragmentWrapper.getXML(stream);
}

const MaterialX::StringVec&  MaterialXData::getGlobalsList() const
{
    return _xmlFragmentWrapper.getGlobalsList();
}

const MaterialX::StringMap& MaterialXData::getPathInputMap() const
{
    return _xmlFragmentWrapper.getPathInputMap();
}

const MaterialX::StringMap& MaterialXData::getPathOutputMap() const
{
    return _xmlFragmentWrapper.getPathOutputMap();
}

void MaterialXData::createDocument(const std::string& materialXDocument)
{
	// Create document
	_document = MaterialX::createDocument();

	// Load libraries
	const MaterialX::StringVec libraries = { "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl" };
	MaterialX::loadLibraries(libraries, _librarySearchPath, _document);

    // Read document contents from disk
    MaterialX::readFromXmlFile(_document, materialXDocument);
}

bool MaterialXData::elementIsAShader() const
{
    return (_element ? _element->isA<MaterialX::ShaderRef>() : false);
}

void MaterialXData::generateXML()
{
    if (!_element)
    {
        return;
    }

	MaterialX::OutputPtr output = _element->asA<MaterialX::Output>();
	MaterialX::ShaderRefPtr shaderRef = _element->asA<MaterialX::ShaderRef>();
	if (!output && !shaderRef)
	{
		// Should never occur as we pre-filter renderables before creating the node + override
		throw MaterialX::Exception("Invalid element to create wrapper for " + _element->getName());
	}

    // Set up generator context. For shaders use FIS environment lookup,
    // but disable this for textures to avoid additional unneeded XML parameter
    // generation.
    _genContext.registerSourceCodeSearchPath(_librarySearchPath);
    if (shaderRef)
    {
        _genContext.getOptions().hwSpecularEnvironmentMethod = MaterialX::SPECULAR_ENVIRONMENT_FIS;
    }
    else
    {
        _genContext.getOptions().hwSpecularEnvironmentMethod = MaterialX::SPECULAR_ENVIRONMENT_NONE;
    }
    _genContext.getOptions().hwMaxActiveLightSources = 0;
    // For Maya we need to insert a V-flip fragment
    _genContext.getOptions().fileTextureVerticalFlip = true;
    // We do not reuqire vertex shader output to XML    
    _xmlFragmentWrapper.setOutputVertexShader(false);

    // Generator XML wrapper
    // TODO: Determine what is a suitable unique name for VP2 fragments using
    // this as a starting identifier.
    const std::string shaderName(_element->getName());
    _xmlFragmentWrapper.generate(shaderName, _element, _genContext);
}

// TODO: This does not belong here. To migrate out to another class.
void MaterialXData::registerFragments()
{
	// Register fragments with the manager if needed
	//	
	if (MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer())
	{
        if (MHWRender::MFragmentManager* fragmentMgr = theRenderer->getFragmentManager())
		{
			const bool fragmentExists = (getFragmentName().size() > 0)
                && fragmentMgr->hasFragment(getFragmentName().c_str());

			if (!fragmentExists)
			{
                std::stringstream glslStream;
                getXML(glslStream);
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
                MString fragmentName = fragmentMgr->addShadeFragmentFromFile(xmlFileName.c_str(), false);

                if (fragmentName.length() == 0)
                {
                    throw MaterialX::Exception("Failed to add OGS shader fragment from file.");
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
