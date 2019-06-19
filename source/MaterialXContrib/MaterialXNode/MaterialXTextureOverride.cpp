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

