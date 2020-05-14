//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/PrimCommands.h>
#include <MaterialXRuntime/Commands/AttributeCommands.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtApi.h>
#include <MaterialXRuntime/Private/Commands/PvtCreatePrimCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtRemovePrimCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtRenamePrimCmd.h>
#include <MaterialXRuntime/Private/Commands/PvtReparentPrimCmd.h>

namespace MaterialX
{

namespace RtCommand
{

void createPrim(RtStagePtr stage, const RtToken& typeName, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtCreatePrimCmd::create(stage, typeName, RtPath("/"), EMPTY_TOKEN);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void createPrim(RtStagePtr stage, const RtToken& typeName, const RtPath& path, RtCommandResult& result)
{
    RtPath parentPath(path);
    parentPath.pop();
    PvtCommandPtr cmd = PvtCreatePrimCmd::create(stage, typeName, parentPath, path.getName());
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void createPrim(RtStagePtr stage, const RtToken& typeName, const RtPath& parentPath, const RtToken& name, RtCommandResult& result)
{
    static RtTokenSet LOOK_MANAGEMENT_NODES { RtToken("collection"),
                                              RtToken("look"),
                                              RtToken("lookgroup"),
                                              RtToken("materialassign")
                                            };
    static RtTokenSet MATERIAL_NODES { RtToken("surfacematerial"),
                                       RtToken("volumematerial")
                                     };
    RtPrim nodeDefPrim =
        RtApi::get().getMasterPrim(RtToken(typeName));

    // If the node does not have a NodeDef it's either a look management node or a material node
    if (!nodeDefPrim) {
        if (LOOK_MANAGEMENT_NODES.count(typeName) == 0 && MATERIAL_NODES.count(typeName) == 0) {
            throw ExceptionRuntimeError("Invalid typeName: " + typeName.str() + " encountered.");
        }
        else if (LOOK_MANAGEMENT_NODES.count(typeName) > 0 && parentPath != RtPath("/")) {
            throw ExceptionRuntimeError("Look management nodes of type: " + typeName.str() + " cannot be placed in compounds.");
        }
    }

    PvtCommandPtr cmd = PvtCreatePrimCmd::create(stage, typeName, parentPath, name);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void removePrim(RtStagePtr stage, const RtPath& path, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtRemovePrimCmd::create(stage, path);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void renamePrim(RtStagePtr stage, const RtPath& path, const RtToken& newName, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtRenamePrimCmd::create(stage, path, newName);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

void reparentPrim(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath, RtCommandResult& result)
{
    PvtCommandPtr cmd = PvtReparentPrimCmd::create(stage, path, newParentPath);
    PvtApi::cast(RtApi::get())->getCommandEngine().execute(cmd, result);
}

}

}
