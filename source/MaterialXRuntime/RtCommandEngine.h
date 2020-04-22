//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTCOMMAND_H
#define MATERIALX_RTCOMMAND_H

/// @file
/// Classes related to command execution.

#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtPointer.h>

namespace MaterialX
{

/// @class RtCommandResult
/// Class holding the results of a command execution.
class RtCommandResult
{
public:
    RtCommandResult() :
        _success(false)
    {
    }

    RtCommandResult(bool success, const string& message = "") :
        _success(success),
        _message(message)
    {
    }

    RtCommandResult(const RtObject& object, bool success = true, const string& message = "") :
        _success(success),
        _message(message),
        _object(object)
    {
    }

    bool success() const
    {
        return _success;
    }

    explicit operator bool() const
    {
        return success();
    }

    const string& getMessage() const
    {
        return _message;
    }

    const RtObject& getObject() const
    {
        return _object;
    }

private:
    bool _success;
    string _message;
    RtObject _object;
};

/// @class RtCommandBase
/// Base class for runtime commands.
class RtCommandBase
{
public:
    virtual ~RtCommandBase() {};

    /// Execute the command.
    virtual void execute(RtCommandResult& result) = 0;

    /// Undo the command.
    virtual void undo(RtCommandResult& result) = 0;

    /// Redo the command.
    virtual void redo(RtCommandResult& result)
    {
        execute(result);
    }
};

using RtCommandPtr = RtSharedPtr<RtCommandBase>;

/// @class RtBatchCommand
/// Class for executing batches of multiple commands.
class RtBatchCommand : public RtCommandBase
{
public:
    /// Constructor.
    RtBatchCommand();

    /// Denstructor.
    ~RtBatchCommand();

    /// Add a command to the batch.
    void addCommand(RtCommandPtr cmd);

    /// Clear all commands in the batch.
    void clearCommands();

    /// Execute the command.
    void execute(RtCommandResult& result) override;

    /// Undo the command.
    void undo(RtCommandResult& result)  override;

    /// Redo the command.
    void redo(RtCommandResult& result)  override;

private:
    void* _ptr;
};

/// @class RtCommandEngine
/// Class handling runtime command execution.
class RtCommandEngine
{
public:
    /// Constructor.
    RtCommandEngine();

    /// Destructor.
    ~RtCommandEngine();

    /// Execute a new command.
    void execute(RtCommandPtr cmd, RtCommandResult& result);

    /// Execute the command on top of the undo queue.
    void undo(RtCommandResult& result);

    /// Execute the command on top of the redo queue.
    void redo(RtCommandResult& result);

private:
    void* _ptr;
};

}

#endif
