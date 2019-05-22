#ifndef MATERIALX_NODE_CMD_H
#define MATERIALX_NODE_CMD_H

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>

#include <vector>

class CreateMaterialXNodeCmd : MPxCommand
{
  public:
	CreateMaterialXNodeCmd();
	~CreateMaterialXNodeCmd() override;

	MStatus doIt( const MArgList& ) override;
	bool isUndoable() { return false; }

	static MSyntax newSyntax();
	static void* creator();

  private:
	void setAttributeValue(MObject &materialXObject, MObject &attr, std::vector<double> &values);
	void createAttribute(MObject &materialXObject, const std::string &name, const std::string &type, const std::string &value);

	MDGModifier m_dgMod;
};

#endif /* MATERIALX_NODE_CMD_H */
