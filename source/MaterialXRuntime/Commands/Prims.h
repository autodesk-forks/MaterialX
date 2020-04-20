//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSTAGECOMMANDS_H
#define MATERIALX_RTSTAGECOMMANDS_H

/// @file
/// Commands for stage handling.

#include <MaterialXRuntime/RtCommandEngine.h>
#include <MaterialXRuntime/RtStage.h>

namespace MaterialX
{

class RtCreatePrimCmd : public RtCommandBase
{
public:
    RtCreatePrimCmd(RtStagePtr stage, const RtToken& typeName, const RtPath& parentPath, const RtToken& name) :
        _stage(stage),
        _typeName(typeName),
        _parentPath(parentPath),
        _name(name)
    {}

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    const RtToken _typeName;
    const RtPath _parentPath;
    RtToken _name;
};

class RtRemovePrimCmd : public RtCommandBase
{
public:
    RtRemovePrimCmd(RtStagePtr stage, const RtPath& path) :
        _path(path)
    {}

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    const RtPath _path;
};

class RtRenamePrimCmd : public RtCommandBase
{
public:
    RtRenamePrimCmd(RtStagePtr stage, const RtPath& path, const RtToken& newName) :
        _path(path),
        _newName(newName)
    {}

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    RtPath _path;
    RtToken _newName;
};

class RtReparentPrimCmd : public RtCommandBase
{
public:
    RtReparentPrimCmd(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath) :
        _path(path),
        _newParentPath(newParentPath)
    {}

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    RtPath _path;
    RtPath _newParentPath;
};

namespace RtCommand
{
    /// Create a new prim at the root of the stage.
    void createPrim(RtStagePtr stage, const RtToken& typeName, RtCommandResult& result);

    /// Create a new prim at the given path.
    void createPrim(RtStagePtr stage, const RtToken& typeName, const RtPath& path, RtCommandResult& result);

    /// Create a new prim inside the parent given by path.
    /// If an empty name is given a name will be generated.
    void createPrim(RtStagePtr stage, const RtToken& typeName, const RtPath& parentPath, const RtToken& name, RtCommandResult& result);

    /// Remove a prim from a stage.
    void removePrim(RtStagePtr stage, const RtPath& path, RtCommandResult& result);

    /// Rename a prim.
    void renamePrim(RtStagePtr stage, const RtPath& path, const RtToken& newName, RtCommandResult& result);

    /// Reparent a prim.
    void reparentPrim(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath, RtCommandResult& result);
}

}

#endif
