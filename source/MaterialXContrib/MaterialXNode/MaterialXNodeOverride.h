#ifndef MATERIALXNODEOVERRIDE_H
#define MATERIALXNODEOVERRIDE_H

#include <maya/MPxShadingNodeOverride.h>

// Override Declaration
class MaterialXNodeOverride : public MHWRender::MPxShadingNodeOverride
{
  public:
	static MHWRender::MPxShadingNodeOverride* creator(const MObject& obj);

	~MaterialXNodeOverride() override;

	MHWRender::DrawAPI supportedDrawAPIs() const override;

	MString fragmentName() const override;
	void getCustomMappings(
	MHWRender::MAttributeParameterMappingList& mappings) override;

	void updateDG() override;
	void updateShader(
	MHWRender::MShaderInstance& shader,
	const MHWRender::MAttributeParameterMappingList& mappings) override;

  private:
	MaterialXNodeOverride(const MObject& obj);

	MString m_materialXDocument;
	MString m_target;
	MObject m_object;
};

#endif /* MATERIALX_NODE_OVERRIDE_H */
