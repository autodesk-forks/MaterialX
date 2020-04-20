//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/Prims.h>
#include <MaterialXRuntime/RtApi.h>

namespace MaterialX
{

void RtCreatePrimCmd::execute(RtCommandResult& result)
{
    try
    {
        RtPrim prim = _stage->createPrim(_parentPath, _name, _typeName);
        
        // Update the name if it was changed so we can undo later.
        _name = prim.getName();

        result = RtCommandResult(prim.asA<RtObject>());
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void RtCreatePrimCmd::undo(RtCommandResult& result)
{
    try
    {
        RtPath path(_parentPath);
        path.push(_name);
        _stage->removePrim(path);
        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void RtRemovePrimCmd::execute(RtCommandResult& result)
{
    try
    {
        _stage->removePrim(_path);
        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void RtRemovePrimCmd::undo(RtCommandResult& result)
{
    result = RtCommandResult(false, "RtRemovePrimCmd: Undo is not implemented!");
}

void RtRenamePrimCmd::execute(RtCommandResult& result)
{
    try
    {
        RtToken oldName = _path.getName();
        RtToken resultName = _stage->renamePrim(_path, _newName);

        // Update the path and name so we can undo later
        _path.pop();
        _path.push(resultName);
        _newName = oldName;

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void RtRenamePrimCmd::undo(RtCommandResult& result)
{
    try
    {
        // Path and name was updated after execution
        // so we can just re-execute the command.
        execute(result);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void RtReparentPrimCmd::execute(RtCommandResult& result)
{
    try
    {
        RtToken resultName = _stage->reparentPrim(_path, _newParentPath);

        // Update the paths so we can undo later
        RtPath newPath = _newParentPath;
        newPath.pop();
        newPath.push(resultName);
        _newParentPath = _path;
        _newParentPath.pop();
        _path = newPath;

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void RtReparentPrimCmd::undo(RtCommandResult& result)
{
    try
    {
        // Paths where updated after execution
        // so we can just re-execute the command.
        execute(result);

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

namespace RtCommand
{

    void createPrim(RtStagePtr stage, const RtToken& typeName, RtCommandResult& result)
    {
        RtCommandPtr cmd = std::make_shared<RtCreatePrimCmd>(stage, typeName, RtPath("/"), EMPTY_TOKEN);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void createPrim(RtStagePtr stage, const RtPath& path, const RtToken& typeName, RtCommandResult& result)
    {
        RtPath parentPath(path);
        parentPath.pop();
        RtCommandPtr cmd = std::make_shared<RtCreatePrimCmd>(stage, typeName, parentPath, path.getName());
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void createPrim(RtStagePtr stage, const RtPath& parentPath, const RtToken& name, const RtToken& typeName, RtCommandResult& result)
    {
        RtCommandPtr cmd = std::make_shared<RtCreatePrimCmd>(stage, typeName, parentPath, name);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void removePrim(RtStagePtr stage, const RtPath& path, RtCommandResult& result)
    {
        RtCommandPtr cmd = std::make_shared<RtRemovePrimCmd>(stage, path);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void renamePrim(RtStagePtr stage, const RtPath& path, const RtToken& newName, RtCommandResult& result)
    {
        RtCommandPtr cmd = std::make_shared<RtRenamePrimCmd>(stage, path, newName);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void reparentPrim(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath, RtCommandResult& result)
    {
        RtCommandPtr cmd = std::make_shared<RtReparentPrimCmd>(stage, path, newParentPath);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

}

}
