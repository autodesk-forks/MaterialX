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

#include <MaterialXCore/Util.h>

namespace MaterialX
{

/// @class RtCommandResult
/// Class holding the results of a command execution.
class RtCommandResult
{
public:
    /// Empty constructor.
    RtCommandResult() :
        _success(false)
    {
    }

    /// Construct a command result from a boolean state and an optional error message.
    RtCommandResult(bool success, const string& message = EMPTY_STRING) :
        _success(success),
        _message(message)
    {
    }

    /// Construct a command result from a return object and optionally
    /// a boolean state and an error message.
    RtCommandResult(const RtObject& object, bool success = true, const string& message = EMPTY_STRING) :
        _success(success),
        _message(message),
        _object(object)
    {
    }

    /// Return true if the command executed succesfully.
    bool success() const
    {
        return _success;
    }

    /// Return true if the command executed succesfully.
    explicit operator bool() const
    {
        return success();
    }

    /// Return an error message if set when executing the command.
    const string& getMessage() const
    {
        return _message;
    }

    /// Return an object resulting from executing the command.
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

    RtCommandBase(const RtCommandBase&) = delete;
    RtCommandBase& operator=(const RtCommandBase&) = delete;

    /// Execute the command.
    virtual void execute(RtCommandResult& result) = 0;

    /// Undo the command.
    virtual void undo(RtCommandResult& result) = 0;

    /// Redo the command.
    virtual void redo(RtCommandResult& result)
    {
        execute(result);
    }

protected:
    RtCommandBase() {}
};

/// A shared pointer to a runtime command.
using RtCommandPtr = RtSharedPtr<RtCommandBase>;

/// @class RtBatchCommand
/// Class for executing batches of multiple commands.
class RtBatchCommand : public RtCommandBase
{
public:
    ~RtBatchCommand();

    RtBatchCommand(const RtBatchCommand&) = delete;
    RtBatchCommand& operator=(const RtBatchCommand&) = delete;

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

protected:
    RtBatchCommand();

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

    /// Flush the undo and redo queues.
    /// All commands previously executed will no longer be undoable.
    void flushUndoQueue();

private:
    void* _ptr;
};

}

#endif
