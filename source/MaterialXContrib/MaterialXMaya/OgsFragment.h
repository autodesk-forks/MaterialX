#ifndef MATERIALX_MAYA_OGSFRAGMENT_H
#define MATERIALX_MAYA_OGSFRAGMENT_H

/// @file
/// OGS fragment wrapper.

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXRender/ImageHandler.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

/// @class OgsFragment
/// Wraps an OGS fragment generated for a specific MaterialX element.
/// The generated source is in an XML format specifying the fragment's inputs
/// and outputs and embedding source code in one or potentially multiple target
/// shading languages (GLSL is the only such language currently supported).
///
class OgsFragment
{
  public:
    OgsFragment(mx::DocumentPtr document,
                mx::ElementPtr element,
                const mx::FileSearchPath& librarySearchPath);

    OgsFragment(const OgsFragment&) = delete;
    OgsFragment(OgsFragment&&) = delete;

    ~OgsFragment();

    /// Returns the path to the renderable element within the document.
    std::string getElementPath() const
    {
        return _element ? _element->getNamePath() : mx::EMPTY_STRING;
    }

    OgsFragment& operator=(const OgsFragment&) = delete;
    OgsFragment& operator=(OgsFragment&&) = delete;

    /// Returns the MaterialX document.
    mx::DocumentPtr getDocument() const
    {
        return _document;
    }

    /// Return the source of the OGS fragment as a string.
    const std::string& getFragmentSource() const;

    /// Return the name of shader fragment. The name is automatically generated
    /// from the name of the MaterialX element and a hash of the generated
    /// fragment source. Can be used to register the fragment in VP2.
    const std::string& getFragmentName() const;

    /// Maps XML element paths of MaterialX inputs to their names in the generated shader.
    const mx::StringMap& getPathInputMap() const;

    /// Return whether the element to render represents a surface shader graph
    /// as opposed to a texture graph.
    bool elementIsAShader() const;

    /// Get image sampling properties for a given file parameter.
    mx::ImageSamplingProperties getImageSamplngProperties(const std::string& fileParameterName) const;

    /// Return whether the fragment represents a transparent surface, as
    /// determined by MaterialX at generation time.
    bool isTransparent() const { return _isTransparent; }

    /// OGS does not support matrix3. As such the matrix4 parameter name is derived from the matrix3 name.
    /// This utility performs this computation.
    static std::string getMatrix4Name(const std::string& matrix3Name);

  private:
    /// Generate the fragment.
    void generateFragment(const mx::FileSearchPath& librarySearchPath);
    
    mx::DocumentPtr _document;  ///< The MaterialX document.
    mx::ElementPtr _element;    ///< The MaterialX element.
    
    std::string _fragmentName;  ///< An automatically generated fragment name.

    std::string _fragmentSource;    ///< The ganerated fragment source.
    
    mx::StringMap _pathInputMap; ///< Maps MaterialX element paths to fragment input names.

    mx::ShaderPtr _shader;      ///< The MaterialX shader.

    bool _isTransparent = false;    ///< Whether the fragment represents a transparent surface.
};

} // namespace MaterialXMaya

#endif // MATERIALX_DATA_H
