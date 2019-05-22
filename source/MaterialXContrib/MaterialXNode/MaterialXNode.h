#ifndef MATERIALXNODE_H
#define MATERIALXNODE_H

#include <maya/MPxNode.h>
#include <maya/MObject.h>

class MaterialXNode : public MPxNode
{
public:
	MaterialXNode();
	~MaterialXNode() override;

	static void* creator();
	static MStatus initialize();
	MTypeId	typeId() const override;

	static const MTypeId s_typeId;
	static const MString s_typeName;
};

#endif /* MATERIALXNODE_H */
