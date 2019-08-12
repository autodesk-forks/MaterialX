#ifndef MATERIALX_MAYA_MATERIALXNODE_H
#define MATERIALX_MAYA_MATERIALXNODE_H

/// @file
/// Maya shading node classes.

#include <MaterialXCore/Document.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

class OgsFragment;

/// @class MaterialXNode
/// The base class for both surface and texture shading nodes.
///
class MaterialXNode : public MPxNode
{
  public:
    MaterialXNode();
    ~MaterialXNode() override;

    /// @name Maya API methods
    /// @{
    static void* creator();
    static MStatus initialize();

    MTypeId	typeId() const override;
    SchedulingType schedulingType() const override;

    bool getInternalValue(const MPlug&, MDataHandle&) override;
    bool setInternalValue(const MPlug&, const MDataHandle&) override;
    /// @}

    /// Sets the attributes values and the OGS fragment owned by the node
    /// when creating the node with CreateMaterialXNodeCmd.
    /// @param documentFilePath The path to the MaterialX document file.
    /// @param elementPath The path to the MaterialX element within the document.
    /// @param envRadianceFileName The file name of the environment map to use for specular shading.
    /// @param envIrradianceFileName The file name of the environment map to use for diffuse shading.
    /// @param ogsFragment An object representing an OGS shade fragment created by the command for this node.
    ///     The ownership of the fragment is transfered to the node.
    ///
    void setData(   const MString& documentFilePath,
                    const MString& elementPath,
                    const MString& envRadianceFileName,
                    const MString& envIrradianceFileName,
                    std::unique_ptr<OgsFragment>&& ogsFragment );

    /// Reloads the document, rebuilds the OGS fragment and refreshes the node in the viewport.
    void reloadDocument();

    /// Returns the OGS fragment.
    const OgsFragment* getOgsFragment() const
    {
        return _ogsFragment.get();
    }

    /// Returns the document file path.
    const MString& getDocumentFilePath() const
    {
        return _documentFilePath;
    }

    /// Returns the file name of the environment map to use for specular shading.
    const MString& getEnvRadianceFileName() const
    {
        return _envRadianceFileName;
    }

    /// Returns the file name of the environment map to use for diffuse shading.
    const MString& getEnvIrradianceFileName() const
    {
        return _envIrradianceFileName;
    }

    static const MTypeId MATERIALX_NODE_TYPEID;
    static const MString MATERIALX_NODE_TYPENAME;

    /// @name Attribute holding the path to the MaterialX document file.
    /// @{
    static const MString DOCUMENT_ATTRIBUTE_LONG_NAME;
    static const MString DOCUMENT_ATTRIBUTE_SHORT_NAME;
    static MObject DOCUMENT_ATTRIBUTE;

    /// @name Attribute holding the path to the MaterialX element within the document.
    /// @{
    static const MString ELEMENT_ATTRIBUTE_LONG_NAME;
    static const MString ELEMENT_ATTRIBUTE_SHORT_NAME;
    static MObject ELEMENT_ATTRIBUTE;
    /// @}

    /// @name Attribute holding the file name of the environment map to use for specular shading.
    /// @{
    static const MString ENV_RADIANCE_ATTRIBUTE_LONG_NAME;
    static const MString ENV_RADIANCE_ATTRIBUTE_SHORT_NAME;
    static MObject ENV_RADIANCE_ATTRIBUTE;
    /// @}

    /// @name Attribute holding the file name of the environment map to use for diffuse shading.
    /// @{
    static const MString ENV_IRRADIANCE_ATTRIBUTE_LONG_NAME;
    static const MString ENV_IRRADIANCE_ATTRIBUTE_SHORT_NAME;
    static MObject ENV_IRRADIANCE_ATTRIBUTE;
    /// @}

    /// The output color attribute, required to correctly connect the node to a shading group.
    /// Maps onto 
    static MObject OUT_ATTRIBUTE;

  protected:
    std::unique_ptr<OgsFragment> _ogsFragment;

  private:
    void createAndRegisterFragment();

    MString _documentFilePath, _elementPath;

    MString _envRadianceFileName = "goegap_4k_dim.hdr";
    MString _envIrradianceFileName = "goegap_4k_dim.convolved.hdr";

    /// The OgsFragment keeps a shared pointer to the document it was created
    /// from but we also keep another shared pointer here to avoid reloading
    /// the document when the element path becomes invalid and the OgsFragment
    /// doesn't exist.
    mx::DocumentPtr _document;
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

    bool getInternalValue(const MPlug&, MDataHandle&) override;

    static const MTypeId MATERIALX_SURFACE_NODE_TYPEID;
    static const MString MATERIALX_SURFACE_NODE_TYPENAME;

    static MObject VP2_TRANSPARENCY_ATTRIBUTE;
};

} // namespace MaterialXMaya

#endif /* MATERIALX_NODE_H */
