#include "ReloadMaterialXLibrariesCmd.h"
#include "Plugin.h"

#include <maya/MGlobal.h>
#include <maya/MSyntax.h>

#include <MaterialXCore/Document.h>

namespace MaterialXMaya
{

MString ReloadMaterialXLibrariesCmd::NAME("reloadMaterialXLibraries");

ReloadMaterialXLibrariesCmd::ReloadMaterialXLibrariesCmd()
{
}

ReloadMaterialXLibrariesCmd::~ReloadMaterialXLibrariesCmd()
{
}

MStatus ReloadMaterialXLibrariesCmd::doIt(const MArgList& /*args*/)
{
    return MS::kSuccess;
}

MSyntax ReloadMaterialXLibrariesCmd::newSyntax()
{
    MSyntax syntax;
    return syntax;
}

void* ReloadMaterialXLibrariesCmd::creator()
{
    return new ReloadMaterialXLibrariesCmd();
}

} // namespace MaterialXMaya
