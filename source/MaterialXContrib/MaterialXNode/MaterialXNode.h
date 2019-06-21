#ifndef MATERIALXNODE_H
#define MATERIALXNODE_H

#include "MaterialXData.h"
#include <MaterialXContrib/OGSXMLFragmentWrapper.h>

#include <maya/MDGModifier.h>
#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MImage.h>

/*
#include <maya/MFnPlugin.h>
#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MRenderUtil.h>
#include <maya/MFloatVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDrawRegistry.h>
#include <maya/MPxShadingNodeOverride.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFragmentManager.h>
#include <maya/MShaderManager.h>
#include <maya/MTextureManager.h>
#include <maya/MStateManager.h>
*/

// Node Declaration
class TestFileNode : public MPxNode
{
public:
    static void* creator();
    static MStatus initialize();

    TestFileNode();
    ~TestFileNode() override;

    MStatus setDependentsDirty(
        const MPlug& plug,
        MPlugArray& plugArray) override;
    MStatus compute(const MPlug&, MDataBlock&) override;

    // ID tag for use with binary file format
    static const MTypeId id;

private:
    MImage fImage;
    size_t fWidth;
    size_t fHeight;

    // Attributes
    static MObject aFileName;
    static MObject aCMConfigPath;
    static MObject aCMWorkingSpace;
    static MObject aColorSpace;
    static MObject aCMEnabled;
    static MObject aCMConfigEnabled;
    static MObject aUVCoord;
    static MObject aOutColor;
    static MObject aOutAlpha;

    friend class FileNodeOverride;
};

class MaterialXNode : public MPxNode
{
  public:
    MaterialXNode();
    ~MaterialXNode() override;

    static void* creator();
    static MStatus initialize();

    void createOutputAttr(MDGModifier& mdgModifier);
    MStatus setDependentsDirty(const MPlug &plugBeingDirtied, MPlugArray & affectedPlugs) override;
    MTypeId	typeId() const override;
    SchedulingType schedulingType() const override;
    bool setInternalValue(const MPlug &plug, const MDataHandle &dataHandle) override;
    void createAttributesFromDocument(MDGModifier& mdgModifier);

    void setMaterialXData(std::unique_ptr<MaterialXData>&& data)
    {
        materialXData = std::move(data);
    }

    static const MTypeId MATERIALX_NODE_TYPEID;
    static const MString MATERIALX_NODE_TYPENAME;

    /// Attribute holding a MaterialX document
    static MString DOCUMENT_ATTRIBUTE_LONG_NAME;
    static MString DOCUMENT_ATTRIBUTE_SHORT_NAME;
    static MObject DOCUMENT_ATTRIBUTE;
    /// Attribute holding a MaterialX element name
    static MString ELEMENT_ATTRIBUTE_LONG_NAME;
    static MString ELEMENT_ATTRIBUTE_SHORT_NAME;
    static MObject ELEMENT_ATTRIBUTE;

    std::unique_ptr<MaterialXData> materialXData;

  private:
    void setAttributeValue(MObject &materialXObject, MObject &attr, float* values, unsigned int size, MDGModifier& mdgModifier);

    MObject _outAttr;
    std::unordered_map<std::string, MaterialX::ElementPtr> _attributeElementPairMap;
};

class MaterialXTextureNode : public MaterialXNode
{
public:
    static void* creator();
    static MStatus initialize();
    MTypeId	typeId() const override;

    static const MTypeId MATERIALX_TEXTURE_NODE_TYPEID;
    static const MString MATERIALX_TEXTURE_NODE_TYPENAME;
};

class MaterialXSurfaceNode : public MaterialXNode
{
public:
    static void* creator();
    static MStatus initialize();
    MTypeId	typeId() const override;

    static const MTypeId MATERIALX_SURFACE_NODE_TYPEID;
    static const MString MATERIALX_SURFACE_NODE_TYPENAME;
};

#endif /* MATERIALX_NODE_H */
