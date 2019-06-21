#include "MaterialXShadingNodeImpl.h"

#include "MaterialXNode.h"
#include "Plugin.h"

#include <maya/MShaderManager.h>
#include <maya/MTextureManager.h>
#include <maya/MPxShadingNodeOverride.h>
#include <maya/MPxSurfaceShadingNodeOverride.h>

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

namespace mx = MaterialX;

namespace
{
struct Vp2TextureDeleter
{
    void operator () (MHWRender::MTexture* texture)
    {
        if (!texture)
        {
            return;
        }

        MHWRender::MRenderer* const renderer = MRenderer::theRenderer();
        if (!renderer)
        {
            return;
        }

        MHWRender::MTextureManager* const
            textureMgr = renderer->getTextureManager();

        if (!textureMgr)
        {
            return;
        }

        textureMgr->releaseTexture(texture);
    };
};

using VP2TextureUniquePtr = std::unique_ptr<
    MHWRender::MTexture,
    Vp2TextureDeleter
>;

struct Vp2SamplerDeleter
{
    void operator () (const MHWRender::MSamplerState* sampler)
    {
        if (sampler)
        {
            MHWRender::MStateManager::releaseSamplerState(sampler);
        }
    };
};

using Vp2SamplerUniquePtr = std::unique_ptr<
    const MHWRender::MSamplerState,
    Vp2SamplerDeleter
>;

// This should be a shared utility
MStatus bindFileTexture(MHWRender::MShaderInstance& shader, 
                        const std::string& parameterName,
                        const MaterialX::FileSearchPath& searchPath, 
                        const std::string& fileName,
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
    }

    return status;
}

// This should be a shared utility
void bindEnvironmentLighting(MHWRender::MShaderInstance& shader,
                            const MStringArray parameterList,
                            const MaterialX::FileSearchPath imageSearchPath,
                            const std::string& envRadiancePath,
                            const std::string& envIrradiancePath)
{
    MHWRender::MSamplerStateDesc samplerDescription;
    samplerDescription.filter = MHWRender::MSamplerState::kAnisotropic;
    samplerDescription.maxAnisotropy = 16;

    MStatus status;

    // Set irradiance map
    MHWRender::MTextureDescription textureDescription;
    if (parameterList.indexOf(mx::HW::ENV_IRRADIANCE.c_str()) >= 0)
    {
        status = bindFileTexture(shader, mx::HW::ENV_IRRADIANCE, imageSearchPath, envIrradiancePath, samplerDescription, textureDescription);
    }

    // Set radiance map
    if (parameterList.indexOf(mx::HW::ENV_RADIANCE.c_str()) >= 0)
    {
        status = bindFileTexture(shader, mx::HW::ENV_RADIANCE, imageSearchPath, envRadiancePath, samplerDescription, textureDescription);
        if (status == MStatus::kSuccess)
        {
            if (parameterList.indexOf(mx::HW::ENV_RADIANCE_MIPS.c_str()) >= 0)
            {
                const int mipCount = (int)std::log2(std::max(textureDescription.fWidth, textureDescription.fHeight)) + 1;
                status = shader.setParameter(mx::HW::ENV_RADIANCE_MIPS.c_str(), mipCount);
            }

            if (parameterList.indexOf(mx::HW::ENV_RADIANCE_SAMPLES.c_str()) >= 0)
            {
                const int envSamples = 16;
                status = shader.setParameter(mx::HW::ENV_RADIANCE_SAMPLES.c_str(), int(envSamples));
            }
        }
    }

    // Environment matrix
    if (parameterList.indexOf(mx::HW::ENV_MATRIX.c_str()) >= 0)
    {
        const float yRotationPI[4][4]{
            -1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, -1, 0,
            0, 0, 0, 1
        };
        MFloatMatrix matrix(yRotationPI);
        status = shader.setParameter(mx::HW::ENV_MATRIX.c_str(), matrix);
    }
}

} // anonymous namespace

template <class BASE>
MaterialXShadingNodeImpl<BASE>::MaterialXShadingNodeImpl(const MObject& obj)
    : BASE(obj)
    , _object(obj)
{
}

template <class BASE>
MaterialXShadingNodeImpl<BASE>::~MaterialXShadingNodeImpl()
{
}

template <class BASE>
MString
MaterialXShadingNodeImpl<BASE>::fragmentName() const
{
    MStatus status;
    MFnDependencyNode depNode(_object, &status);
    const auto* const node = dynamic_cast<MaterialXNode*>(depNode.userNode());
    const MaterialXData* const data = node ? node->materialXData.get() : nullptr;
    return data ? data->getFragmentName().c_str() : "";
}

template <class BASE>
void MaterialXShadingNodeImpl<BASE>::updateDG()
{
}

template <class BASE>
void MaterialXShadingNodeImpl<BASE>::updateShader(MHWRender::MShaderInstance& shader,
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

    // Bind environment lighting
    // TODO: These should be options
    std::string envRadiancePath = "san_giuseppe_bridge.hdr";
    std::string envIrradiancePath = "san_giuseppe_bridge_diffuse.hdr";
    bindEnvironmentLighting(shader, parameterList, imageSearchPath,
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

class MaterialXTextureOverride;
class MaterialXSurfaceOverride;

namespace MHWRender
{
    class MPxShadingNodeOverride;
    class MPxSurfaceShadingNodeOverride;
};

template class MaterialXShadingNodeImpl<MHWRender::MPxShadingNodeOverride>;
template class MaterialXShadingNodeImpl<MHWRender::MPxSurfaceShadingNodeOverride>;
