//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXFormat/XmlExport.h>

#include <MaterialXFormat/Util.h>

namespace MaterialX
{

namespace
{

void mergeLooks(DocumentPtr doc, const XmlExportOptions* exportOptions)
{
    if (exportOptions && exportOptions->mergeLooks)
    {
        doc->mergeLooks(exportOptions->lookGroupToMerge);
    }
}

} // anonymous namespace

//
// XmlExportOptions methods
//

XmlExportOptions::XmlExportOptions() :
    XmlWriteOptions(),
    mergeLooks(false),
    flattenFilenames(true)
{
}

//
// Exporting
//

void exportToXmlStream(DocumentPtr doc, std::ostream& stream, const XmlExportOptions* exportOptions)
{
    DocumentPtr exportDoc = doc;
    if (exportOptions && !exportOptions->modifyInPlace)
    {
        exportDoc = doc->copy();
    }

    mergeLooks(exportDoc, exportOptions);
    if (exportOptions && exportOptions->flattenFilenames)
    {
        flattenFilenames(exportDoc, exportOptions->resolvedTexturePath, exportOptions->stringResolver);
    }
    writeToXmlStream(exportDoc, stream, exportOptions);
}

void exportToXmlFile(DocumentPtr doc, const FilePath& filename, const XmlExportOptions* exportOptions)
{
    DocumentPtr exportDoc = doc;
    if (exportOptions && !exportOptions->modifyInPlace)
    {
        exportDoc = doc->copy();
    }

    mergeLooks(doc, exportOptions);
    if (exportOptions)
    {
        if (exportOptions->flattenFilenames)
        {
            flattenFilenames(doc, exportOptions->resolvedTexturePath, exportOptions->stringResolver);
        }
        for (ExportResolverPtr exportResolver : exportOptions->exportResolvers)
        {
            exportResolver->resolve(doc);
        }
    }
    writeToXmlFile(doc, filename, exportOptions);
}

string exportToXmlString(DocumentPtr doc, const XmlExportOptions* exportOptions)
{
    DocumentPtr exportDoc = doc;
    if (exportOptions && !exportOptions->modifyInPlace)
    {
        exportDoc = doc->copy();
    }

    mergeLooks(doc, exportOptions);
    if (exportOptions)
    {
        if (exportOptions->flattenFilenames)
        {
            flattenFilenames(doc, exportOptions->resolvedTexturePath, exportOptions->stringResolver);
        }
        for (ExportResolverPtr exportResolver : exportOptions->exportResolvers)
        {
            exportResolver->resolve(doc);
        }
    }
    return writeToXmlString(doc, exportOptions);
}

} // namespace MaterialX
