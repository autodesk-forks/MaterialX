#ifndef MATERIALX_DATA_H
#define MATERIALX_DATA_H

/// @file
/// MaterialX Data wrapper

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>
#include <MaterialXGenShader/Shader.h>

namespace mx = MaterialX;

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
    MaterialXData(const std::string& materialXDocumentPath, const std::string& elementPath, const mx::FileSearchPath& librarySearchPath);
    ~MaterialXData();

    /// Set the element to render with.
    /// If the element specified is an empty string then an attempt will be
    /// make to find the first renderable element.
    bool setRenderableElement(const std::string& materialXDocumentPath, const std::string& elementPath);

    /// Returns the path of the element to render
    std::string getElementPath() const
    {
        if (_element)
        {
            return _element->getNamePath();
        }
        return mx::EMPTY_STRING;
    }

    /// Create the OGS XML wrapper for shader fragments associated
    /// with the element set to render
    void generateXml();

    MaterialXData& operator=(const MaterialXData&) = delete;
    MaterialXData& operator=(MaterialXData&&) = delete;

    /// Return MaterialX document 
    mx::DocumentPtr getDocument() const
    {
        return _document;
    }

    /// Return XML string
    const std::string& getFragmentWrapper() const;

    /// Return name of shader fragment
    const std::string& getFragmentName() const;

    /// Get list of Element paths and corresponding XML input names
    const mx::StringMap& getPathInputMap() const;

    /// Return if the element to render represents a shader graph
    /// as opposed to a texture grraph.
    bool elementIsAShader() const;

  protected:
    /// Returns whether the element is renderable
    bool isRenderable();

    /// Create a new document from disk
    void createDocument(const std::string& materialXDocument);

    /// Reset any Xml related data.
    void clearXml();

  private:
    // Reference document and element
    mx::FileSearchPath _librarySearchPath;
    mx::DocumentPtr _document;
    mx::ElementPtr _element;

    // TODO: This is currently a prototype interface
    mx::ShaderGeneratorPtr _shaderGenerator;
    mx::GenContext _genContext;
    mx::OgsXmlGenerator _generator;

    // XML fragment name
    std::string _fragmentName;

    // XML fragment wrapper
    std::string _fragmentWrapper;

    // Mapping from MaterialX Element paths to XML input names
    mx::StringMap _pathInputMap;
};

#endif // MATERIALX_DATA_H
