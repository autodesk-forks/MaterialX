#include "MaterialXNodeOverride.h"

#include <maya/MFnDependencyNode.h>
#include <maya/MFragmentManager.h>
#include <maya/MRenderUtil.h>
#include <maya/MShaderManager.h>
#include <maya/MTextureManager.h>

MHWRender::MPxShadingNodeOverride* MaterialXNodeOverride::creator(const MObject& obj)
{
	return new MaterialXNodeOverride(obj);
}

MaterialXNodeOverride::MaterialXNodeOverride(const MObject& obj)
	: MPxShadingNodeOverride(obj)
	, m_object(obj)
{
}

MaterialXNodeOverride::~MaterialXNodeOverride()
{
}

MHWRender::DrawAPI MaterialXNodeOverride::supportedDrawAPIs() const
{
	return MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile;
}

MString MaterialXNodeOverride::fragmentName() const
{
	return "TempName";
}

void MaterialXNodeOverride::getCustomMappings(
	MHWRender::MAttributeParameterMappingList& /*mappings*/)
{
}

void MaterialXNodeOverride::updateDG()
{
	// Pull the file MaterialX file and target from the DG for use in updateShader
	MStatus status;
	MFnDependencyNode node(m_object, &status);
	if (status)
	{
		node.findPlug("materialXDocument", false, &status).getValue(m_materialXDocument);
		node.findPlug("target", false, &status).getValue(m_target);
	}
}

void MaterialXNodeOverride::updateShader(MHWRender::MShaderInstance& /*shader*/, const MHWRender::MAttributeParameterMappingList& /*mappings*/)
{
}
