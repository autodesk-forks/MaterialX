//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Commands/Prims.h>
#include <MaterialXRuntime/Commands/Connections.h>
#include <MaterialXRuntime/RtApi.h>

namespace MaterialX
{

RtCommandPtr RtCreatePrimCmd::create(RtStagePtr stage, const RtToken& typeName, const RtPath& parentPath, const RtToken& name)
{
    return std::make_shared<RtCreatePrimCmd>(stage, typeName, parentPath, name);
}

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


RtCommandPtr RtRemovePrimCmd::create(RtStagePtr stage, const RtPath& path)
{
    return std::make_shared<RtRemovePrimCmd>(stage, path);
}

void RtRemovePrimCmd::execute(RtCommandResult& result)
{
    try
    {
        _prim = _stage->getPrimAtPath(_path);
        if (!_prim)
        {
            throw ExceptionRuntimeError("RtRemovePrimCmd: Can't find a prim with path '" + _path.asString() + "'");
        }

        clearCommands();

        RtObjTypePredicate<RtInput> inputFilter;
        for (RtAttribute attr : _prim.getAttributes(inputFilter))
        {
            RtInput input = attr.asA<RtInput>();
            if (input.isConnected())
            {
                RtOutput output = input.getConnection();
                addCommand(RtBreakConnectionCmd::create(output, input));
            }
        }

        RtObjTypePredicate<RtOutput> outputFilter;
        for (RtAttribute attr : _prim.getAttributes(outputFilter))
        {
            RtOutput output = attr.asA<RtOutput>();
            if (output.isConnected())
            {
                for (RtObject inputObj : output.getConnections())
                {
                    RtInput input = inputObj.asA<RtInput>();
                    addCommand(RtBreakConnectionCmd::create(output, input));
                }
            }
        }

        // Execute all the break connection commands.
        RtBatchCommand::execute(result);

        // Dispose the prim.
        if (result.success())
        {
            _stage->disposePrim(_path);
        }

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}

void RtRemovePrimCmd::undo(RtCommandResult& result)
{
    try
    {
        // Bring the prim back to life.
        RtPath parentPath(_path);
        parentPath.pop();
        _stage->revivePrim(parentPath, _prim);

        // Undo all the break connection commands.
        RtBatchCommand::undo(result);
        clearCommands();

        result = RtCommandResult(true);
    }
    catch (const ExceptionRuntimeError& e)
    {
        result = RtCommandResult(false, string(e.what()));
    }
}


RtCommandPtr RtRenamePrimCmd::create(RtStagePtr stage, const RtPath& path, const RtToken& newName)
{
    return std::make_shared<RtRenamePrimCmd>(stage, path, newName);
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


RtCommandPtr RtReparentPrimCmd::create(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath)
{
    return std::make_shared<RtReparentPrimCmd>(stage, path, newParentPath);
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
        RtCommandPtr cmd = RtCreatePrimCmd::create(stage, typeName, RtPath("/"), EMPTY_TOKEN);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void createPrim(RtStagePtr stage, const RtToken& typeName, const RtPath& path, RtCommandResult& result)
    {
        RtPath parentPath(path);
        parentPath.pop();
        RtCommandPtr cmd = RtCreatePrimCmd::create(stage, typeName, parentPath, path.getName());
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void createPrim(RtStagePtr stage, const RtToken& typeName, const RtPath& parentPath, const RtToken& name, RtCommandResult& result)
    {
        RtCommandPtr cmd = RtCreatePrimCmd::create(stage, typeName, parentPath, name);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void removePrim(RtStagePtr stage, const RtPath& path, RtCommandResult& result)
    {
        RtCommandPtr cmd = RtRemovePrimCmd::create(stage, path);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void renamePrim(RtStagePtr stage, const RtPath& path, const RtToken& newName, RtCommandResult& result)
    {
        RtCommandPtr cmd = RtRenamePrimCmd::create(stage, path, newName);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

    void reparentPrim(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath, RtCommandResult& result)
    {
        RtCommandPtr cmd = RtReparentPrimCmd::create(stage, path, newParentPath);
        RtApi::get().getCommandEngine().execute(cmd, result);
    }

}

}
