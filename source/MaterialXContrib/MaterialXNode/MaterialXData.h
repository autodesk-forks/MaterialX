#ifndef MATERIALX_DATA_H
#define MATERIALX_DATA_H

/// @file
/// MaterialX Data wrapper

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXContrib/OGSXMLFragmentWrapper.h>

/// @class MaterialXData
/// Wrapper for MaterialX associated data. 
///
/// Keeps track of an element to render and it's associated document.
///
/// Can optionally create and cache an XML wrapper instance 
/// which wraps up the interface and shader code for code generated based 
/// on the specified element to render.
/// Currently the only language target available is GLSL.
///
struct MaterialXData
{
  public:
    /// The element path and document that the element resides in are passed in
    /// as input arguments
    MaterialXData(const std::string& materialXDocumentPath, const std::string& elementPath);
    ~MaterialXData();

    /// Set the MaterialX Document and selement path.
    bool setData(const std::string& materialXDocumentPath, const std::string& elementPath);

    /// Returns whether we have valid output element
    bool isValidOutput()
    {
        return (nullptr != _element);
    }

    /// Create the OGS XML wrapper for shader fragments associated
    /// with the element set to render
    void generateXML();

    /// Register the fragment(s)
    void registerFragments(const std::string& ogsXmlPath);

    MaterialXData& operator=(const MaterialXData&) = delete;
    MaterialXData& operator=(MaterialXData&&) = delete;

    /// Return MaterialX document 
    MaterialX::DocumentPtr getDocument() const
    {
        return _document;
    }

    /// Return XML string
    void getXML(std::ostream& stream) const;

    /// Return name of shader fragment
    const std::string& getFragmentName() const;

    /// Get list of global inputs which are not associated with any Element
    const MaterialX::StringVec&  getGlobalsList() const;

    /// Get list of Element paths and corresponding fragment input names
    const MaterialX::StringMap& getPathInputMap() const;

    /// Get list of Element paths and corresponding fragment output names
    /// If the output is a ShaderRef then the path is to that element
    /// as there are no associated child output Elements.
    const MaterialX::StringMap& getPathOutputMap() const;

    /// Return if the element to render represents a shader graph
    /// as opposed to a texture grraph.
    bool elementIsAShader() const;

  protected:
    /// Returns whether the element is renderable
    bool isRenderable();
    void createDocument(const std::string& materialXDocument);

  private:
    // Reference document and element
    MaterialX::FilePath _librarySearchPath;
    MaterialX::DocumentPtr _document;
    MaterialX::ElementPtr _element;

    // TODO: This is currently a prototype interface
    MaterialX::GenContext _genContext;
    MaterialX::OGSXMLFragmentWrapper _xmlFragmentWrapper;
};

#endif // MATERIALX_DATA_H
