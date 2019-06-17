#include "MaterialXSurfaceOverride.h"
#include "MaterialXNode.h"
#include "Plugin.h"
#include "Util.h"

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXRender/StbImageLoader.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFragmentManager.h>
#include <maya/MRenderUtil.h>
#include <maya/MShaderManager.h>
#include <maya/MTextureManager.h>

#include <fstream>

const MString
    MaterialXSurfaceOverride::REGISTRANT_ID = "materialXSurface",
    MaterialXSurfaceOverride::DRAW_CLASSIFICATION = "drawdb/shader/surface/materialX";

MHWRender::MPxShadingNodeOverride* MaterialXSurfaceOverride::creator(const MObject& obj)
{
	std::cout.rdbuf(std::cerr.rdbuf());
	return new MaterialXSurfaceOverride(obj);
}

MaterialXSurfaceOverride::MaterialXSurfaceOverride(const MObject& obj)
	: MPxShadingNodeOverride(obj)
	, _object(obj)
{
	MStatus status;
	MFnDependencyNode depNode(_object, &status);
	if (status)
	{
		depNode.findPlug(MaterialXNode::DOCUMENT_ATTRIBUTE_LONG_NAME, false, &status).getValue(_documentContent);
		depNode.findPlug(MaterialXNode::ELEMENT_ATTRIBUTE_LONG_NAME, false, &status).getValue(_element);
	}

    try
    {
        // Create document
        _document = MaterialX::createDocument();
        MaterialX::readFromXmlString(_document, _documentContent.asChar());

        // Load libraries - why is this done twice. Once in the command and once here??
        MaterialX::FilePath libSearchPath = Plugin::instance().getLibrarySearchPath();
        const MaterialX::StringVec libraries = { "stdlib", "pbrlib", "bxdf", "stdlib/genglsl", "pbrlib/genglsl" };
        MaterialX::loadLibraries(libraries, libSearchPath, _document);

        std::cout.rdbuf(std::cerr.rdbuf());

        std::vector<MaterialX::GenContext*> contexts;
        MaterialX::GenContext* glslContext = new MaterialX::GenContext(MaterialX::GlslShaderGenerator::create());

        // Stop emission of environment map lookups.
        glslContext->getOptions().hwSpecularEnvironmentMethod = MaterialX::SPECULAR_ENVIRONMENT_NONE;
        glslContext->registerSourceCodeSearchPath(libSearchPath);
        contexts.push_back(glslContext);

        _glslWrapper = new MaterialX::OGSXMLFragmentWrapper();
        _glslWrapper->setOutputVertexShader(true);

        MaterialX::ElementPtr element = _document->getDescendant(_element.asChar());
        MaterialX::ShaderRefPtr shaderRef = element->asA<MaterialX::ShaderRef>();
        MaterialX::OutputPtr output = element->asA<MaterialX::Output>();
        if (!output && !shaderRef)
        {
            // Should never occur as we pre-filter renderables before creating the node + override
            throw MaterialX::Exception("MaterialXSurfaceOverride: Invalid type to create wrapper for");
        }

        // TODO: This just indicates that lighting is required. As direct lighting
        // is not supported, the requirement means that indirect lighting is required.
        // bool requiresLighting = (shaderRef != nullptr);
        std::cout << "MaterialXSurfaceOverride: Create XML wrapper" << std::endl;
        _glslWrapper->generate(element->getName(), element, *glslContext);
        // Get the fragment name
        _fragmentName.set(_glslWrapper->getFragmentName().c_str());

        // Register fragments with the manager if needed
        //
        MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
        if (theRenderer)
        {
            MHWRender::MFragmentManager* fragmentMgr =
                theRenderer->getFragmentManager();
            if (fragmentMgr)
            {
                bool fragmentExists = (_fragmentName.length() > 0) && fragmentMgr->hasFragment(_fragmentName);
                if (!fragmentExists)
                {
                    std::stringstream glslStream;
                    _glslWrapper->getXML(glslStream);
                    std::string xmlFileName(Plugin::instance().getResourcePath().asString() + "/standard_surface_default.xml");

                    // TODO: This should not be hard-coded
                    fragmentMgr->setEffectOutputDirectory("d:/work/");
                    fragmentMgr->setIntermediateGraphOutputDirectory("d:/work/");
                    _fragmentName = fragmentMgr->addShadeFragmentFromFile(xmlFileName.c_str(), false);
                }
            }
            std::cout << "MaterialXSurfaceOverride: Add XML fragment to manager: " << _fragmentName << std::endl;
        }
    }
    catch (MaterialX::Exception& e)
    {
        std::cerr << "MaterialXSurfaceOverride: Failed to generate XML wrapper: " << e.what() << std::endl;
    }

    // TODO: Use our own image loader vs Mayas.
    // MaterialX::StbImageLoaderPtr stbLoader = MaterialX::StbImageLoader::create();
}

MaterialXSurfaceOverride::~MaterialXSurfaceOverride()
{
	// TODO: Free sampler state here!

	if (_glslWrapper)
	{
		delete _glslWrapper;
	}
}

MHWRender::DrawAPI MaterialXSurfaceOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kOpenGLCoreProfile;
}

MString MaterialXSurfaceOverride::fragmentName() const
{
	return _fragmentName;
}

#if 0
void MaterialXSurfaceOverride::getCustomMappings(MHWRender::MAttributeParameterMappingList& mappings)
{
	const MaterialX::StringMap& inputs = _glslWrapper->getPathInputMap();
	for (auto i : inputs)
	{
		std::cout.rdbuf(std::cerr.rdbuf());
		std::cout << "MaterialXSurfaceOverride: Get custom mappings: " << i.second.c_str() << std::endl;
		MHWRender::MAttributeParameterMapping mapping(i.second.c_str(), "", false, true);
		mappings.append(mapping);
	}
}
#endif

void MaterialXSurfaceOverride::updateDG()
{
}

void MaterialXSurfaceOverride::updateShader(MHWRender::MShaderInstance& shader,
                                            const MHWRender::MAttributeParameterMappingList& mappings)
{
    MStringArray params;
    shader.parameterList(params);
    for (unsigned int i = 0; i < params.length(); i++)
    {
        std::cout << "MaterialXSurfaceOverride: shader param: " << params[i].asChar() << std::endl;
    }

    MStatus status;
	const MaterialX::StringMap& inputs = _glslWrapper->getPathInputMap();
	for (auto i : inputs)
	{
        std::string inputName(i.second);
        MString resolvedName(inputName.c_str());
        const MHWRender::MAttributeParameterMapping* mapping = mappings.findByParameterName(i.second.c_str());
		if (mapping)
		{
			resolvedName = mapping->resolvedParameterName();
		}

		MaterialX::ElementPtr element = _document->getDescendant(i.first);
		if (!element) continue;
		MaterialX::ValueElementPtr valueElement = element->asA<MaterialX::ValueElement>();
		if (valueElement)
		{
			if (valueElement->getType() == MaterialX::FILENAME_TYPE_STRING)
			{
                // This is the hard-cided OGS convention to associate a texture with a sampler (via post-fix "Sampler" string)
                std::string textureParameterName(resolvedName.asChar());
                std::string samplerParameterName(textureParameterName + "Sampler");

                // Bind texture
                std::string fileName; 
                std::string valueString = valueElement->getValueString();
                if (!valueString.empty())
                {
                    MaterialX::FileSearchPath searchPath(Plugin::instance().getResourcePath() / MaterialX::FilePath("Images"));
                    MaterialX::FilePath imagePath = searchPath.find(valueString);
                    if (imagePath.exists())
                    {
                        fileName = imagePath.asString();
                        MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
                        if (renderer)
                        {
                            MHWRender::MTextureManager* textureManager = renderer->getTextureManager();
                            if (textureManager)
                            {
                                MHWRender::MTexture* texture = textureManager->acquireTexture(fileName.c_str(), "");
                                if (texture)
                                {
                                    MHWRender::MTextureAssignment textureAssignment;
                                    textureAssignment.texture = texture;
                                    status = shader.setParameter(textureParameterName.c_str(), textureAssignment);
                                    std::cout << "Bind texture " << textureParameterName << ". Status: " << status << std::endl;

                                    // release our reference now that it is set on the shader
                                    textureManager->releaseTexture(texture);
                                }
                            }
                        }
                    }
                }

                // Bind sampler
                MHWRender::MSamplerStateDesc desc;
                desc.filter = MHWRender::MSamplerState::kAnisotropic;
                desc.maxAnisotropy = 16;
                const MSamplerState* samplerState = MHWRender::MStateManager::acquireSamplerState(desc);
                if (samplerState)
                {
                    status = shader.setParameter(samplerParameterName.c_str(), *samplerState);
                    std::cout << "Bind sampler: " << samplerParameterName << ". Status: " << status << std::endl;
                }
			}
#if 0
            // TODO: setArrayParameter is the incorrect call so disable all of these for now.
            // Also none of these need to be set since they are supposed to be immutable.
            // To modify the MaterialX document should be modified and a new shader generated.
            // Note: To find out how to remove a fragment otherwise we cannot edit/update.
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector2>::TYPE)
			{
				MaterialX::Vector2 vector2 = valueElement->getValue()->asA<MaterialX::Vector2>();
                status = shader.setArrayParameter(resolvedName, vector2.data(), 2);
				std::cout << "updateShader (vector2): " << resolvedName << ". Status: " << status << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
			{
				MaterialX::Vector3 vector3 = valueElement->getValue()->asA<MaterialX::Vector3>();
                status = shader.setArrayParameter(resolvedName, vector3.data(), 3);
				std::cout << "updateShader (vector3): " << resolvedName << ". Status: " << status << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector4>::TYPE)
			{
				MaterialX::Vector4 vector4 = valueElement->getValue()->asA<MaterialX::Vector4>();
                status = shader.setArrayParameter(resolvedName, vector4.data(), 4);
				std::cout << "updateShader (vector4): " << resolvedName << ". Status: " << status << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color2>::TYPE)
			{
				MaterialX::Color2 color2 = valueElement->getValue()->asA<MaterialX::Color2>();
                status = shader.setArrayParameter(resolvedName, color2.data(), 2);
                std::cout << "updateShader (color2): " << resolvedName << ". Status: " << status << std::endl;
            }
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
			{
				MaterialX::Color3 color3 = valueElement->getValue()->asA<MaterialX::Color3>();
                status = shader.setArrayParameter(resolvedName, color3.data(), 3);
                std::cout << "updateShader (color3): " << resolvedName << std::endl;
            }
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color4>::TYPE)
			{
				MaterialX::Color4 color4 = valueElement->getValue()->asA<MaterialX::Color4>();
                status = shader.setArrayParameter(resolvedName, color4.data(), 4);
                std::cout << "updateShader (color4): " << resolvedName << std::endl;
            }
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Matrix44>::TYPE)
			{
				MaterialX::Matrix44 mat44 = valueElement->getValue()->asA<MaterialX::Matrix44>();
                status = shader.setArrayParameter(resolvedName, mat44.data(), 16);
                std::cout << "updateShader (mat44): " << resolvedName << std::endl;
            }
#endif
        }
	}
}
