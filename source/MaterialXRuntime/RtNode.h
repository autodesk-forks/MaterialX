//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTNODE_H
#define MATERIALX_RTNODE_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtElement.h>

namespace MaterialX
{

/// @class RtPort
/// TODO: Docs
class RtPort
{
public:
    /// Default constructor.
    RtPort();

    /// Construct a port from a node and a portdef.
    RtPort(RtObject node, RtObject portdef);

    /// Return true if the port is valid.
    bool isValid() const;

    /// Returns true if the port is invalid.
    bool operator!() const
    {
        return !isValid();
    }

    /// Explicit bool conversion operator.
    /// Return true if the port is valid.
    explicit operator bool() const
    {
        return isValid();
    }

    /// Return the name of the port.
    const RtToken& getName() const;

    /// Return the type of the port.
    const RtToken& getType() const;

    /// Return the node the port belongs to.
    RtObject getNode() const;

    /// Return the attribute flags for this port.
    int32_t getFlags() const;

    /// Return true if this is an input port.
    bool isInput() const;

    /// Return true if this is an output port.
    bool isOutput() const;

    /// Return true if this port is connectable.
    bool isConnectable() const;

    /// Return the value for this port.
    const RtValue& getValue() const;

    /// Return the value for this port.
    RtValue& getValue();

    /// Return a string representation for the value of this port.
    string getValueString();

    /// Get the color space for this value.
    const RtToken& getColorSpace() const;

    /// Get the color space for this value.
    const RtToken& getUnit() const;

    /// Set a new value on the port.
    void setValue(const RtValue& v);

    /// Set a new bool value on the port.
    void setValue(bool v);

    /// Set a new integer value on the port.
    void setValue(int v);

    /// Set a new float value on the port.
    void setValue(float v);

    /// Set a new color3 value on the port.
    void setValue(const Color3& v);

    /// Set a new vector3 value on the port.
    void setValue(const Vector4& v);

    /// Set a new pointer value on the port.
    void setValue(void* v);

    /// Set the color space for this value.
    void setColorSpace(const RtToken& colorspace);

    /// Set the unit for this value.
    void setUnit(const RtToken& unit);

    /// Return true if this port is connected.
    bool isConnected() const;

    /// Connect this port to a destination input port.
    void connectTo(const RtPort& dest);

    /// Disconnect this port from a destination input port.
    void disconnectFrom(const RtPort& dest);

    /// Return the source port connected upstream.
    RtPort getSourcePort() const;

    /// Return the number of destination ports connected downstream.
    size_t numDestinationPorts() const;

    /// Return a destination port connected downstream.
    RtPort getDestinationPort(size_t index) const;

    /// Traverse the node network upstream starting from this port.
    RtGraphIterator traverseUpstream(RtTraversalFilter filter = nullptr) const;

    /// Equality operator
    bool operator==(const RtPort& other) const
    {
        return _data == other._data && _index == other._index;
    }

    /// Inequality operator
    bool operator!=(const RtPort& other) const
    {
        return _data != other._data || _index != other._index;
    }

    /// Less-than operator
    bool operator<(const RtPort& other) const
    {
        return _data != other._data ? 
            _data < other._data : _index < other._index;
    }

    /// Return the data handle.
    PrvObjectHandle data() const
    {
        return _data;
    }

private:
    RtPort(PrvObjectHandle data, size_t index);

    PrvObjectHandle _data;
    size_t _index;
    friend class PrvNode;
};

/// @class RtNode
/// TODO: Docs
class RtNode : public RtElement
{
public:
    /// Constructor attaching and object to the API.
    RtNode(const RtObject& obj);

    /// Create a new node instance of the given nodedef
    /// and add it to the parent object if specified.
    /// The parent must be a stage or a nodegraph object.
    static RtObject createNew(const RtToken& name, RtObject nodedef, RtObject parent = RtObject());

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Return the node category for this node.
    const RtToken& getCategory() const;

    /// Return the port count.
    size_t numPorts() const;

    /// Return the port count.
    size_t numOutputs() const;

    /// Return a port by index, or a null object 
    /// if no such port exists.
    RtPort getPort(size_t index) const;

    /// Return a port corresponding to the given portdef object,
    /// or a null object if no such port exists.
    RtPort getPort(RtObject portdef) const;

    /// Find a port by name, or a null object 
    /// if no such port is found.
    RtPort findPort(const RtToken& name) const;

    /// Make a new connection between two ports.
    static void connect(const RtPort& source, const RtPort& dest);

    /// Break a connection between two ports.
    static void disconnect(const RtPort& source, const RtPort& dest);
};

}

#endif
