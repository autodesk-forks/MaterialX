#ifndef MaterialXSurfaceOverride_H
#define MaterialXSurfaceOverride_H

#include "../OGSXMLFragmentWrapper.h"

#include <MaterialXCore/Document.h>

#include <maya/MPxShadingNodeOverride.h>

class MaterialXSurfaceOverride : public MHWRender::MPxShadingNodeOverride
{
  public:
	static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);

	~MaterialXSurfaceOverride() override;

	MHWRender::DrawAPI supportedDrawAPIs() const override;

	MString fragmentName() const override;
	//void getCustomMappings(MHWRender::MAttributeParameterMappingList& mappings) override;

	void updateDG() override;
	void updateShader(MHWRender::MShaderInstance& shader,
	                  const MHWRender::MAttributeParameterMappingList& mappings) override;

    bool valueChangeRequiresFragmentRebuild(const MPlug* /*plug*/) const override
    {
        return false;
    }

	static const MString REGISTRANT_ID, DRAW_CLASSIFICATION;

private:
	MaterialXSurfaceOverride(const MObject& obj);

	MaterialX::OGSXMLFragmentWrapper* _glslWrapper;
	MaterialX::DocumentPtr _document;
	MString _fragmentName;
	MString _documentContent;
	MString _element;
	MObject _object;
};

#endif /* MATERIALX_NODE_OVERRIDE_H */
