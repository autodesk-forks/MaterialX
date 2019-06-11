#ifndef MaterialXTextureOverride_H
#define MaterialXTextureOverride_H

#include "../OGSXMLFragmentWrapper.h"

#include <MaterialXCore/Document.h>

#include <maya/MPxShadingNodeOverride.h>

/// VP2 Texture fragment override
class MaterialXTextureOverride : public MHWRender::MPxShadingNodeOverride
{
  public:
	static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);

	~MaterialXTextureOverride() override;

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
	MaterialXTextureOverride(const MObject& obj);

	MObject _object;
};

/////////////////////////////////////////////
class TestFileNodeOverride : public MHWRender::MPxShadingNodeOverride
{
public:
    static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);

    ~TestFileNodeOverride() override;

    MHWRender::DrawAPI supportedDrawAPIs() const override;

    MString fragmentName() const override;
    void getCustomMappings(
        MHWRender::MAttributeParameterMappingList& mappings) override;

    void updateDG() override;
    void updateShader(
        MHWRender::MShaderInstance& shader,
        const MHWRender::MAttributeParameterMappingList& mappings) override;

    bool valueChangeRequiresFragmentRebuild(const MPlug* plug) const override;

private:
    TestFileNodeOverride(const MObject& obj);

    MString fFragmentName;
    MObject fObject;
    MString fFileName;
    const MHWRender::MSamplerState* fSamplerState;

    mutable MString fResolvedMapName;
    mutable MString fResolvedSamplerName;
};


#endif /* MATERIALX_NODE_OVERRIDE_H */
