//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTPATH_H
#define MATERIALX_RTPATH_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

namespace MaterialX
{

/// @class RtPath
/// Class representing a path which points to, and uniquely identifies,
/// an object in a stage.
/// Can be used to store references to objects in a stage hierarchy.
/// Note that an RtPath may become invalid if the object pointed to,
/// or any of its parents, are renamed, reparented or removed from the
/// stage. Use path.isValid() to query if the path is still valid if
/// paths are held on to while other stage operations may occure.
class RtPath
{
public:
    /// Construct a path from an object.
    RtPath(RtObject obj);

    /// Desctructor.
    ~RtPath();
    
    /// Return true if the path points to a valid object.
    bool isValid() const;

    /// Return the type for the object at the end of the path.
    RtObjType getObjType() const;

    /// Query if the given API type is supported by theobject
    /// at the end of this path.
    bool hasApi(RtApiType type) const;

    /// Return the object pointer to by this path.
    RtObject getObject() const;

    /// Return a string representation of this path.
    string getPathString() const;

    /// Push a child element on the path and make it the
    /// top element pointed to by the path.
    /// The given child must be an existing child object on the
    /// top most object pointed to by the path. If not the path
    /// becomes invalid after the push operation.
    void push(const RtToken& childName);

    /// Pop the top most object from the path.
    /// After this operation the path will point to
    /// the parent of the currently top most object.
    void pop();

    /// Equality operator.
    bool operator==(const RtPath& other) const;

    /// Inequality operator.
    bool operator!=(const RtPath& other) const
    {
        return !(*this == other);
    }

private:
    void* _ptr;
};

}

#endif
