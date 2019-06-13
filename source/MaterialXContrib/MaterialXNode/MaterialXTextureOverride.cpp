#include "MaterialXTextureOverride.h"
#include "MaterialXNode.h"
#include "Plugin.h"
#include "Util.h"

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
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
    MaterialXTextureOverride::REGISTRANT_ID = "materialXTexture",
    MaterialXTextureOverride::DRAW_CLASSIFICATION = "drawdb/shader/texture/2d/materialX";

MHWRender::MPxShadingNodeOverride* MaterialXTextureOverride::creator(const MObject& obj)
{
	std::cout.rdbuf(std::cerr.rdbuf());
	return new MaterialXTextureOverride(obj);
}

MaterialXTextureOverride::MaterialXTextureOverride(const MObject& obj)
	: MPxShadingNodeOverride(obj)
	, _object(obj)
{
	/*
	MStatus status;
	MFnDependencyNode depNode(_object, &status);
	MaterialXNode* node = dynamic_cast<MaterialXNode*>(depNode.userNode());

        MaterialX::ElementPtr element = _document->getDescendant(_element.asChar());
        MaterialX::ShaderRefPtr shaderRef = element->asA<MaterialX::ShaderRef>();
        MaterialX::OutputPtr output = element->asA<MaterialX::Output>();
        if (!output && !shaderRef)
        {
            // Should never occur as we pre-filter renderables before creating the node + override
            throw MaterialX::Exception("MaterialXTextureOverride: Invalid type to create wrapper for");
        }

        std::vector<MaterialX::GenContext*> contexts;
        MaterialX::GenContext* glslContext = new MaterialX::GenContext(MaterialX::GlslShaderGenerator::create());

        // Stop emission of environment map lookups.
        glslContext->registerSourceCodeSearchPath(libSearchPath);
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

        _glslWrapper = new MaterialX::OGSXMLFragmentWrapper(glslContext);
        _glslWrapper->setOutputVertexShader(true);     

        // TODO: This just indicates that lighting is required. As direct lighting
        // is not supported, the requirement means that indirect lighting is required.
        // bool requiresLighting = (shaderRef != nullptr);
        std::cout << "MaterialXTextureOverride: Create XML wrapper" << std::endl;
        _glslWrapper->createWrapper(element);
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
                    _glslWrapper->getDocument(glslStream);
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
                }
            }
            std::cout << "MaterialXTextureOverride: Add XML fragment to manager: " << _fragmentName << std::endl;
        }
    }
    catch (MaterialX::Exception& e)
    {
        std::cerr << "MaterialXTextureOverride: Failed to generate XML wrapper: " << e.what() << std::endl;
    }
	}*/
}

MaterialXTextureOverride::~MaterialXTextureOverride()
{
	// TODO: Free sampler state here!
}

MHWRender::DrawAPI MaterialXTextureOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kOpenGLCoreProfile;
}

MString MaterialXTextureOverride::fragmentName() const
{
	MStatus status;
	MFnDependencyNode depNode(_object, &status);
	MaterialXNode* node = dynamic_cast<MaterialXNode*>(depNode.userNode());
	return node->materialXData->getFragmentName();
}

void MaterialXTextureOverride::updateDG()
{
}

MStatus bindFileTexture(MHWRender::MShaderInstance& shader, const std::string& parameterName, 
                        const MaterialX::FileSearchPath& searchPath, const std::string& fileName)
{
    MStatus status = MStatus::kFailure;

    MaterialX::FilePath imagePath = searchPath.find(fileName);
    bool imageFound = imagePath.exists();
    std::cout << "> Bind image file: " << imagePath.asString() << ": " << imageFound << std::endl;
    if (imageFound)
    {
        MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
        if (renderer)
        {
            MHWRender::MTextureManager* textureManager = renderer->getTextureManager();
            if (textureManager)
            {
                MHWRender::MTexture* texture = textureManager->acquireTexture(imagePath.asString().c_str(), "");
                if (texture)
                {
                    MHWRender::MTextureAssignment textureAssignment;
                    textureAssignment.texture = texture;
                    status = shader.setParameter(parameterName.c_str(), textureAssignment);
                    std::cout << "Bound file: " << imagePath.asString() << " to shader parameter:" << parameterName << ". Status: " << 
                        status << "\r\n";

                    // release our reference now that it is set on the shader
                    textureManager->releaseTexture(texture);
                }
            }
        }
    }

    // Bind sampler
    std::string samplerParameterName(parameterName + "Sampler");
    MHWRender::MSamplerStateDesc desc;
    desc.filter = MHWRender::MSamplerState::kAnisotropic;
    desc.maxAnisotropy = 16;
    const MSamplerState* samplerState = MHWRender::MStateManager::acquireSamplerState(desc);
    if (samplerState)
    {
        status = shader.setParameter(samplerParameterName.c_str(), *samplerState);
        std::cout << "Bind sampler: " << samplerParameterName << ". Status: " << status << "\r\n";
    }

    return status;
}

void MaterialXTextureOverride::updateShader(MHWRender::MShaderInstance& shader, 
                                            const MHWRender::MAttributeParameterMappingList& mappings)
{
    MStatus status;
    MFnDependencyNode depNode(_object, &status);
    MaterialXNode* node = dynamic_cast<MaterialXNode*>(depNode.userNode());
    if (!node)
    {
        return;
    }

    MStringArray params;
    shader.parameterList(params);
    for (unsigned int j = 0; j < params.length(); j++)
    {
        std::cout << "MaterialXTextureOverride: shader param: " << params[i].asChar() << "\n";
    }

    MaterialX::FileSearchPath imageSearchPath(Plugin::instance().getResourcePath() / MaterialX::FilePath("Images"));

    // Bind globals which are not associated with any document elements
    const MaterialX::StringVec& globals = node->materialXData->getFragmentWrapper()->getGlobalsList();
    for (auto global : globals)
    {
        // TODO: These should be options
        std::string envRadiancePath = "san_giuseppe_bridge.hdr";
        std::string envIrradiancePath = "san_giuseppe_bridge_diffuse.hdr";

        if (global == "u_envIrradiance")
        {
            status = bindFileTexture(shader, global, imageSearchPath, envIrradiancePath);
        }
        else if (global == "u_envRadiance")
        {
            status = bindFileTexture(shader, global, imageSearchPath, envRadiancePath);
        }
    }

    MaterialX::DocumentPtr document = node->materialXData->getDocument();
	const MaterialX::StringMap& inputs = node->materialXData->getFragmentWrapper()->getPathInputMap();
	for (auto i : inputs)
	{
        std::string inputName(i.second);
        MString resolvedName(inputName.c_str());
        const MHWRender::MAttributeParameterMapping* mapping = mappings.findByParameterName(i.second.c_str());
		if (mapping)
		{
			resolvedName = mapping->resolvedParameterName();
		}

		MaterialX::ElementPtr element = document->getDescendant(i.first);
		if (!element) continue;
		MaterialX::ValueElementPtr valueElement = element->asA<MaterialX::ValueElement>();
		if (valueElement)
		{
			if (valueElement->getType() == MaterialX::FILENAME_TYPE_STRING)
			{
                // This is the hard-cided OGS convention to associate a texture with a sampler (via post-fix "Sampler" string)
                std::string textureParameterName(resolvedName.asChar());

                // Bind texture
                std::string fileName; 
                const std::string& valueString = valueElement->getValueString();
                if (!valueString.empty())
                {
                    status = bindFileTexture(shader, textureParameterName, imageSearchPath, valueString);
                }
			}

            // Note: To find out how to remove a fragment otherwise we cannot edit/update.
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector2>::TYPE)
			{
				MaterialX::Vector2 vector2 = valueElement->getValue()->asA<MaterialX::Vector2>();
				MFloatVector floatVector(vector2[0], vector2[1]);
				status = shader.setParameter(resolvedName, floatVector);
				std::cout << "updateShader (vector2): " << resolvedName << ". Status: " << status << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
			{
				MaterialX::Vector3 vector3 = valueElement->getValue()->asA<MaterialX::Vector3>();
				MFloatVector floatVector(vector3[0], vector3[1], vector3[2]);
				status = shader.setParameter(resolvedName, floatVector);
				std::cout << "updateShader (vector3): " << resolvedName << ". Status: " << status << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector4>::TYPE)
			{
//				MaterialX::Vector4 vector4 = valueElement->getValue()->asA<MaterialX::Vector4>();
//				MFloatVector floatVector(vector4[0], vector4[1], vector4[2], vector4[3]);
//				status = shader.setParameter(resolvedName, floatVector);
//				std::cout << "updateShader (vector4): " << resolvedName << ". Status: " << status << std::endl;
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color2>::TYPE)
			{
				MaterialX::Color2 color2 = valueElement->getValue()->asA<MaterialX::Color2>();
				MFloatVector floatVector(color2[0], color2[1]);
				status = shader.setParameter(resolvedName, floatVector);
				std::cout << "updateShader (color2): " << resolvedName << ". Status: " << status << std::endl;
            }
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
			{
				MaterialX::Color3 color3 = valueElement->getValue()->asA<MaterialX::Color3>();
				MFloatVector floatVector(color3[0], color3[1], color3[2]);
				status = shader.setParameter(resolvedName, floatVector);
				std::cout << "updateShader (color3): " << resolvedName << std::endl;
            }
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color4>::TYPE)
			{
//				MaterialX::Color4 color4 = valueElement->getValue()->asA<MaterialX::Color4>();
//                status = shader.setArrayParameter(resolvedName, color4.data(), 4);
//                std::cout << "updateShader (color4): " << resolvedName << std::endl;
            }
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Matrix44>::TYPE)
			{
				MaterialX::Matrix44 mat44 = valueElement->getValue()->asA<MaterialX::Matrix44>();
                status = shader.setArrayParameter(resolvedName, mat44.data(), 16);
                std::cout << "updateShader (mat44): " << resolvedName << std::endl;
            }
        }
	}
}


////////////////////////////////////////////////////////////////////////////
// Override Implementation
////////////////////////////////////////////////////////////////////////////
MHWRender::MPxShadingNodeOverride* TestFileNodeOverride::creator(
    const MObject& obj)
{
    return new TestFileNodeOverride(obj);
}

TestFileNodeOverride::TestFileNodeOverride(const MObject& obj)
    : MPxShadingNodeOverride(obj)
    , fObject(obj)
    , fFragmentName("")
    , fFileName("")
    , fSamplerState(NULL)
    , fResolvedMapName("")
    , fResolvedSamplerName("")
{
    // Define fragments and fragment graph needed for VP2 version of shader.
    //
    static const MString sFragmentOutputName("myFragOutput");
    static const MString sFragmentName("fileTexturePluginFragment");
    static const MString sFragmentGraphName("fileTexturePluginGraph");

    // Register fragments with the manager if needed
    //
    MHWRender::MRenderer* theRenderer = MHWRender::MRenderer::theRenderer();
    if (theRenderer)
    {
        MHWRender::MFragmentManager* fragmentMgr =
            theRenderer->getFragmentManager();
        if (fragmentMgr)
        {
            // Add fragments if needed
            bool fragAdded = fragmentMgr->hasFragment(sFragmentName);
            bool structAdded = fragmentMgr->hasFragment(sFragmentOutputName);
            bool graphAdded = fragmentMgr->hasFragment(sFragmentGraphName);
            if (!fragAdded)
            {
                std::string body(Plugin::instance().getResourcePath().asString() + "/" + sFragmentName.asChar() + ".xml");
                fragAdded = (sFragmentName == fragmentMgr->addShadeFragmentFromFile(body.c_str(), false));
            }
            if (!structAdded)
            {
                std::string structS(Plugin::instance().getResourcePath().asString() + "/" + sFragmentOutputName.asChar() + ".xml");
                structAdded = (sFragmentOutputName == fragmentMgr->addShadeFragmentFromFile(structS.c_str(), false));
            }
            if (!graphAdded)
            {
                std::string graphS(Plugin::instance().getResourcePath().asString() + "/" + sFragmentGraphName.asChar() + ".xml");
                graphAdded = (sFragmentGraphName == fragmentMgr->addFragmentGraphFromFile(graphS.c_str()));
            }

            // If we have them all, use the final graph for the override
            if (fragAdded && structAdded && graphAdded)
            {
                fFragmentName = sFragmentGraphName;
            }
        }
    }
}

TestFileNodeOverride::~TestFileNodeOverride()
{
    MHWRender::MStateManager::releaseSamplerState(fSamplerState);
    fSamplerState = NULL;
}

MHWRender::DrawAPI TestFileNodeOverride::supportedDrawAPIs() const
{
    return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString TestFileNodeOverride::fragmentName() const
{
    // Reset cached parameter names since the effect is being rebuilt
    fResolvedMapName = "";
    fResolvedSamplerName = "";

    return fFragmentName;
}

void TestFileNodeOverride::getCustomMappings(
    MHWRender::MAttributeParameterMappingList& mappings)
{
    // Set up some mappings for the parameters on the file texture fragment,
    // there is no correspondence to attributes on the node for the texture
    // parameters.
    MHWRender::MAttributeParameterMapping mapMapping(
        "map", "", false, true);
    mappings.append(mapMapping);

    MHWRender::MAttributeParameterMapping textureSamplerMapping(
        "textureSampler", "", false, true);
    mappings.append(textureSamplerMapping);
}

void TestFileNodeOverride::updateDG()
{
    // Pull the file name from the DG for use in updateShader
    MStatus status;
    MFnDependencyNode node(fObject, &status);
    if (status)
    {
        MString name;
        node.findPlug("fileName", true).getValue(name);
        MRenderUtil::exactFileTextureName(name, false, "", "", fFileName);
    }
}

void TestFileNodeOverride::updateShader(
    MHWRender::MShaderInstance& shader,
    const MHWRender::MAttributeParameterMappingList& mappings)
{
    // Handle resolved name caching
    if (fResolvedMapName.length() == 0)
    {
        const MHWRender::MAttributeParameterMapping* mapping =
            mappings.findByParameterName("map");
        if (mapping)
        {
            fResolvedMapName = mapping->resolvedParameterName();
        }
    }
    if (fResolvedSamplerName.length() == 0)
    {
        const MHWRender::MAttributeParameterMapping* mapping =
            mappings.findByParameterName("textureSampler");
        if (mapping)
        {
            fResolvedSamplerName = mapping->resolvedParameterName();
        }
    }

    // Set the parameters on the shader
    if (fResolvedMapName.length() > 0 && fResolvedSamplerName.length() > 0)
    {
        // Set sampler to linear-wrap
        if (!fSamplerState)
        {
            MHWRender::MSamplerStateDesc desc;
            desc.filter = MHWRender::MSamplerState::kAnisotropic;
            desc.maxAnisotropy = 16;
            fSamplerState = MHWRender::MStateManager::acquireSamplerState(desc);
        }
        if (fSamplerState)
        {
            shader.setParameter(fResolvedSamplerName, *fSamplerState);
        }

        // Set texture
        MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
        if (renderer)
        {
            MHWRender::MTextureManager* textureManager =
                renderer->getTextureManager();
            if (textureManager)
            {
                MHWRender::MTexture* texture =
                    textureManager->acquireTexture(fFileName, "");
                if (texture)
                {
                    MHWRender::MTextureAssignment textureAssignment;
                    textureAssignment.texture = texture;
                    shader.setParameter(fResolvedMapName, textureAssignment);

                    // release our reference now that it is set on the shader
                    textureManager->releaseTexture(texture);
                }
            }
        }
    }
}

bool TestFileNodeOverride::valueChangeRequiresFragmentRebuild(const MPlug* /*plug*/) const
{
    return true;
    //return MHWRender::MPxShadingNodeOverride::valueChangeRequiresFragmentRebuild(plug);
}

