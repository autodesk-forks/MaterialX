//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MATERIALNODE_H
#define MATERIALX_MATERIALNODE_H

/// @file
/// Material node utilities 

#include <MaterialXCore/Library.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Element.h>
#include <MaterialXCore/Interface.h>

#include <unordered_set>

namespace MaterialX
{

/// Return a vector of all Shader nodes for a Material node.
/// @param materialNode Node to examine
/// @param shaderType Type of shader to return. If an empty string is specified then
///                   all shader node types are returned. The default argument value is an empty.
/// @param target Target attribute of shader to return. The default argument value is an empty string
///               which indicates to include shaders which match any target.
std::unordered_set<NodePtr> getShaderNodes(const NodePtr& materialNode,
                                           const string& shaderType = EMPTY_STRING,
                                           const string& target = EMPTY_STRING);

/// Return a vector of all MaterialAssign elements that bind this material node
/// to the given geometry string
/// @param materialNode Node to examine
/// @param geom The geometry for which material bindings should be returned.
///             By default, this argument is the universal geometry string "/",
///             and all material bindings are returned.
/// @return Vector of MaterialAssign elements
vector<MaterialAssignPtr> getGeometryBindings(const NodePtr& materialNode, const string& geom);


} // namespace MaterialX

#endif
