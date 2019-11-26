//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTELEMENT_H
#define MATERIALX_RTELEMENT_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>
#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtTraversal.h>

namespace MaterialX
{

class PrvElement;

/// @class RtAttribute
/// Class representing an attribute on an element. An attribute
/// holds a name, a type and a value and is used to store data,
/// or metadata, on an element. Any data that is not explicitly 
/// expressed by elements and sub-elements needs to be stored as
/// attributes.
class RtAttribute
{
public:
    /// Get attribute name.
    const RtToken& getName() const
    {
        return _name;
    }

    /// Get attribute type.
    const RtToken& getType() const
    {
        return _type;
    }

    /// Get attribute value.
    const RtValue& getValue() const
    {
        return _value;
    }

    /// Get attribute value.
    RtValue& getValue()
    {
        return _value;
    }

    /// Set attribute value.
    void setValue(const RtValue& value)
    {
        _value = value;
    }

    /// Return a string representation for the value of this attribute.
    string getValueString() const;

    /// Set attribute value from a string representation.
    void setValueString(const string& v);

private:
    /// Private constructor.
    RtAttribute(const RtToken& name, const RtToken& type, RtObject parent);

    RtToken _name;
    RtToken _type;
    RtValue _value;
    friend class PrvElement;
};


/// @class RtElement
/// API for accessing an element. This API can be
/// attached to objects of all element types.
class RtElement : public RtApiBase
{
public:
    /// Constructor attaching an object to the API.
    RtElement(const RtObject& obj);

    /// Return the type for this API.
    RtApiType getApiType() const override;

    /// Get element name.
    const RtToken& getName() const;

    /// Set element name.
    /// The name may be changed to form a unique name within
    /// the scope of this element and its siblings.
    void setName(const RtToken& name);

    /// Add an attribute.
    RtAttribute* addAttribute(const RtToken& name, const RtToken& type);

    /// Return an attribute by name, or a nullptr
    /// if no such attribute exists.
    const RtAttribute* getAttribute(const RtToken& name) const;

    /// Return an attribute by name, or a nullptr
    /// if no such attribute exists.
    RtAttribute* getAttribute(const RtToken& name);

    /// Return an attribute by index, or a nullptr
    /// if no such attribute exists.
    const RtAttribute* getAttribute(size_t index) const;

    /// Return an attribute by name, or a nullptr
    /// if no such attribute exists.
    RtAttribute* getAttribute(size_t index);

    /// Return the attribute count.
    size_t numAttributes() const;

    /// Return an iterator traversing all children
    /// of this element.
    /// If a filter is set it will be called to restrict
    /// which objects to return.
    RtTreeIterator traverseTree(RtTraversalFilter filter = nullptr);
};

}

#endif
