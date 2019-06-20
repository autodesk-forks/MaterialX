#include "MaterialXTextureOverride.h"
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
    , _enableEditing(false)
{
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
	return node->materialXData->getFragmentName().c_str();
}

void MaterialXTextureOverride::updateDG()
{
}

// This should be a shared utility
MStatus bindFileTexture(MHWRender::MShaderInstance& shader, const std::string& parameterName, 
                        const MaterialX::FileSearchPath& searchPath, const std::string& fileName,
                        const MHWRender::MSamplerStateDesc& samplerDescription, 
                        MHWRender::MTextureDescription& textureDescription)
{
    MStatus status = MStatus::kFailure;

    // Bind file texture
    MaterialX::FilePath imagePath = searchPath.find(fileName);
    bool imageFound = imagePath.exists();
    if (imageFound)
    {
        MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
        if (renderer)
        {
            MHWRender::MTextureManager* textureManager = renderer->getTextureManager();
            if (textureManager)
            {
                MHWRender::MTexture* texture = textureManager->acquireTexture(imagePath.asString().c_str(), MaterialX::EMPTY_STRING.c_str());
                if (texture)
                {
                    MHWRender::MTextureAssignment textureAssignment;
                    textureAssignment.texture = texture;
                    status = shader.setParameter(parameterName.c_str(), textureAssignment);

                    std::cout << "Bind texture: " << parameterName
                        << ". image: " << imagePath.asString() << ". Status: "
                        << status << std::endl;

                    // Get back the texture description
                    texture->textureDescription(textureDescription);

                    // release our reference now that it is set on the shader
                    textureManager->releaseTexture(texture);
                }
            }
        }
    }

    // Bind sampler. This is not correct as it's not taking into account
    // the MaterialX sampler state.
    const std::string SAMPLE_PREFIX_STRING("Sampler");
    std::string samplerParameterName(parameterName + SAMPLE_PREFIX_STRING);
    const MSamplerState* samplerState = MHWRender::MStateManager::acquireSamplerState(samplerDescription);
    if (samplerState)
    {
        status = shader.setParameter(samplerParameterName.c_str(), *samplerState);
        std::cout << "Bind sampler: " << samplerParameterName
             << " Status: "
            << status << std::endl;

    }

    return status;
}

// This should be a shared utility
void bindEnvironmentLighting(MHWRender::MShaderInstance& shader, 
                             const MaterialX::StringVec& globals,
                             const MStringArray parameterList,
                             const MaterialX::FileSearchPath imageSearchPath, 
                             const std::string& envRadiancePath,
                             const std::string& envIrradiancePath)
{
    static std::string IRRADIANCE_PARAMETER("u_envIrradiance");
    static std::string RADIANCE_PARAMETER("u_envRadiance");
    static std::string RADIANCE_MIPS_PARAMETER("u_envRadianceMips");
    static std::string ENVIRONMENT_MATRIX_PARAMETER("u_envMatrix");

    MHWRender::MSamplerStateDesc samplerDescription;
    samplerDescription.filter = MHWRender::MSamplerState::kAnisotropic;
    samplerDescription.maxAnisotropy = 16;

    // Bind globals which are not associated with any document elements
    MStatus status;
    for (auto global : globals)
    {
        // Set irradiance map
        MHWRender::MTextureDescription textureDescription;
        if (global == IRRADIANCE_PARAMETER)
        {
            if (parameterList.indexOf(global.c_str()) >= 0)
            {
                status = bindFileTexture(shader, global, imageSearchPath, envIrradiancePath,
                                         samplerDescription, textureDescription);
            }
        }

        // Set radiance map
        else if (global == RADIANCE_PARAMETER)
        {
            if (parameterList.indexOf(global.c_str()) >= 0)
            {
                status = bindFileTexture(shader, global, imageSearchPath, envRadiancePath, 
                                         samplerDescription, textureDescription);
                if (status == MStatus::kSuccess)
                {
                    if (parameterList.indexOf(RADIANCE_MIPS_PARAMETER.c_str()) >= 0)
                    {
                        int mipCount = (int)std::log2(std::max(textureDescription.fWidth, textureDescription.fHeight)) + 1;
                        status = shader.setParameter(global.c_str(), mipCount);

                        std::cout << "Bind mip levels: " << global
                            << ". Val: " << std::to_string(mipCount) << ". Status: "
                            << status << std::endl;
                    }
                }
            }
        }

        // Environment matrix
        else if (global == ENVIRONMENT_MATRIX_PARAMETER)
        {
            if (parameterList.indexOf(global.c_str()) >= 0)
            {
                const float yRotationPI[4][4]{
                    -1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, -1, 0,
                    0, 0, 0, 1
                };
                MFloatMatrix matrix(yRotationPI);
                matrix.setToIdentity();
                status = shader.setParameter(global.c_str(), matrix);

                std::cout << "Bind env matrix: " << global
                    << ". Status: " << status << std::endl;

            }
        }
    }
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

    // Get the parameter list fo checking against.
    MStringArray parameterList;    
    shader.parameterList(parameterList);

    // Set up image file name search path. Assume we are using built in images located in resource path
    // TODO: Be able to add more image search paths.
    static std::string IMAGE_FOLDER("Images");
    MaterialX::FileSearchPath imageSearchPath(Plugin::instance().getResourcePath() / MaterialX::FilePath(IMAGE_FOLDER));

    const MaterialX::StringVec& globals = node->materialXData->getGlobalsList();

    // Bind environment lighting
    // TODO: These should be options
    std::string envRadiancePath = "san_giuseppe_bridge.hdr";
    std::string envIrradiancePath = "san_giuseppe_bridge_diffuse.hdr";
    bindEnvironmentLighting(shader, globals, parameterList, imageSearchPath, 
                            envRadiancePath, envIrradiancePath);

    MaterialX::DocumentPtr document = node->materialXData->getDocument();
	const MaterialX::StringMap& inputs = node->materialXData->getPathInputMap();
	for (auto i : inputs)
	{		
        MaterialX::ElementPtr element = document->getDescendant(i.first);
        MaterialX::ValueElementPtr valueElement = element ? element->asA<MaterialX::ValueElement>() : nullptr;
		if (valueElement)
		{
            std::string inputName(i.second);
            MString resolvedName(inputName.c_str());
            const MHWRender::MAttributeParameterMapping* mapping = mappings.findByParameterName(i.second.c_str());
            if (mapping)
            {
                resolvedName = mapping->resolvedParameterName();
            }
            
            if (valueElement->getType() == MaterialX::FILENAME_TYPE_STRING)
			{
                // This is the hard-coded OGS convention to associate a texture with a sampler (via post-fix "Sampler" string)
                std::string textureParameterName(resolvedName.asChar());

                // Bind texture and sampler
                std::string fileName; 
                const std::string& valueString = valueElement->getValueString();
                if (!valueString.empty())
                {
                    MHWRender::MTextureDescription textureDescription;

                    // TODO: This should come from the element and not hard-coded.
                    MHWRender::MSamplerStateDesc samplerDescription;
                    samplerDescription.filter = MHWRender::MSamplerState::kAnisotropic;
                    samplerDescription.maxAnisotropy = 16;

                    status = bindFileTexture(shader, textureParameterName, imageSearchPath, valueString, 
                                             samplerDescription, textureDescription);
                }
			}

            // This is unnecessary overhead if we are read-only. Updates should be based on whats
            // dirty and not everythhing. There are a lot of attributes on shader graph and this is
            // a waste of effort currently.
            if (!_enableEditing)
            {
                continue;
            }

            if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector2>::TYPE)
			{
				MaterialX::Vector2 vector2 = valueElement->getValue()->asA<MaterialX::Vector2>();
				MFloatVector floatVector(vector2[0], vector2[1]);
				status = shader.setParameter(resolvedName, floatVector);
			}
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Vector3>::TYPE)
			{
				MaterialX::Vector3 vector3 = valueElement->getValue()->asA<MaterialX::Vector3>();
				MFloatVector floatVector(vector3[0], vector3[1], vector3[2]);
				status = shader.setParameter(resolvedName, floatVector);
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
            }
			else if (valueElement->getType() == MaterialX::TypedValue<MaterialX::Color3>::TYPE)
			{
				MaterialX::Color3 color3 = valueElement->getValue()->asA<MaterialX::Color3>();
				MFloatVector floatVector(color3[0], color3[1], color3[2]);
				status = shader.setParameter(resolvedName, floatVector);
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
                    textureManager->acquireTexture(fFileName, "", 0);
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

