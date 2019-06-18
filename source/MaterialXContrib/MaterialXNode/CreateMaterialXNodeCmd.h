#ifndef CREATE_MATERIALX_NODE_CMD_H
#define CREATE_MATERIALX_NODE_CMD_H

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>

#include <MaterialXCore/Document.h>
#include <MaterialXRender/Util.h>

#include <vector>

///
///
///
class CreateMaterialXNodeCmd : MPxCommand
{
  public:
	CreateMaterialXNodeCmd();
	~CreateMaterialXNodeCmd() override;

	MStatus doIt( const MArgList& ) override;
	bool isUndoable() { return false; }

	static MSyntax newSyntax();
	static void* creator();

    static MString NAME;

  private:
	void setAttributeValue(MObject &materialXObject, MObject &attr, const float* values, unsigned int size);
	void setAttributeValue(MObject &materialXObject, MObject &attr, const std::string& stringValue);
	void createAttribute(MObject &materialXObject, const std::string& name, const MaterialX::UIPropertyItem& propertyItem);
	void createAttributes(MObject &materialXObject, const MaterialX::UIPropertyGroup& groups, const MaterialX::UIPropertyGroup& unnamedGroups);

	MDGModifier _dgModifier;
};

#endif /* CREATE_MATERIALX_NODE_CMD_H */
