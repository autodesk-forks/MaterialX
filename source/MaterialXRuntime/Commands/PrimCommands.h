//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPRIMCOMMANDS_H
#define MATERIALX_RTPRIMCOMMANDS_H

/// @file
/// Commands for prim handling.

#include <MaterialXRuntime/RtCommandEngine.h>
#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>

namespace MaterialX
{


/// @class RtCreatePrimCmd
/// Command for creating a prim.
class RtCreatePrimCmd : public RtCommandBase
{
public:
    RtCreatePrimCmd(RtStagePtr stage, const RtToken& typeName, const RtPath& parentPath, const RtToken& name) :
        _stage(stage),
        _typeName(typeName),
        _parentPath(parentPath),
        _name(name)
    {}

    static RtCommandPtr create(RtStagePtr stage, const RtToken& typeName, const RtPath& parentPath, const RtToken& name);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;
    void redo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    const RtToken _typeName;
    const RtPath _parentPath;
    RtToken _name;
    RtPrim _prim;
};

/// @class RtRemovePrimCmd
/// Command for removing a prim.
class RtRemovePrimCmd : public RtCommandList
{
public:
    RtRemovePrimCmd(RtStagePtr stage, const RtPath& path):
        _stage(stage),
        _path(path)
    {}

    static RtCommandPtr create(RtStagePtr stage, const RtPath& path);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    const RtPath _path;
    RtPrim _prim;
};

/// @class RtRenamePrimCmd
/// Command for renaming a prim.
class RtRenamePrimCmd : public RtCommandBase
{
public:
    RtRenamePrimCmd(RtStagePtr stage, const RtPath& path, const RtToken& newName) :
        _stage(stage),
        _path(path),
        _newName(newName)
    {}

    static RtCommandPtr create(RtStagePtr stage, const RtPath& path, const RtToken& newName);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    RtPath _path;
    RtToken _newName;
};

/// @class RtReparentPrimCmd
/// Command for reparenting a prim.
class RtReparentPrimCmd : public RtCommandBase
{
public:
    RtReparentPrimCmd(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath) :
        _stage(stage),
        _path(path),
        _parentPath(newParentPath),
        _originalName(path.getName())
    {}

    static RtCommandPtr create(RtStagePtr stage, const RtPath& path, const RtPath& newParentPath);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

private:
    RtStagePtr _stage;
    RtPath _path;
    RtPath _parentPath;
    RtToken _originalName;
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
