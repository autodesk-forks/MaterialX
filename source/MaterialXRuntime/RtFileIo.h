//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTFILEIO_H
#define MATERIALX_RTFILEIO_H

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtStage.h>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

/// API for read and write of data from MaterialXCore documents
/// to MaterialXRuntime stages.
class RtFileIo : public RtApiBase
{
public:
    /// Filter function type used for filtering elements during read.
    /// If the filter returns false the element will not be read.
    using ReadFilter = std::function<bool(const ElementPtr& elem)>;

    /// Filter function type used for filtering objects during write.
    /// If the filter returns false the object will not be written.
    using WriteFilter = std::function<bool(const RtObject& obj)>;

public:
    /// Constructor attaching this API to a stage.
    RtFileIo(RtObject stage);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Read contents from a document.
    /// If a filter is used only elements accepted by the filter
    /// will be red from the document.
    void read(const DocumentPtr& doc, ReadFilter filter = nullptr);

    /// Write all stage contents to a document.
    /// If a filter is used only elements accepted by the filter
    /// will be written to the document.
    void write(DocumentPtr& doc, WriteFilter filter = nullptr);

    /// Read contents from a file path.
    /// If a filter is used only elements accepted by the filter
    /// will be red from the document.
    void read(const FilePath& documentPath, const FileSearchPath& searchPaths, ReadFilter filter = nullptr);

    /// Write all stage contents to a document.
    /// If a filter is used only elements accepted by the filter
    /// will be written to the document.
    void write(const FilePath& documentPath, const XmlWriteOptions* writeOptions, WriteFilter filter = nullptr);

    /// Read in dependent libraries into stage or as a stage reference
    void loadLibraries(const StringVec& libraryPaths, const FileSearchPath& searchPaths, const RtToken& referenceName);
};

}

#endif
