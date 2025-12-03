//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphDebug.h>
#include <MaterialXGenShader/ShaderNode.h>

#include <fstream>
#include <sstream>

MATERIALX_NAMESPACE_BEGIN

namespace
{

// Escape special characters for DOT labels
string escapeLabel(const string& str)
{
    string result;
    result.reserve(str.size());
    for (char c : str)
    {
        switch (c)
        {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '<':  result += "\\<"; break;
            case '>':  result += "\\>"; break;
            case '|':  result += "\\|"; break;
            case '{':  result += "\\{"; break;
            case '}':  result += "\\}"; break;
            default:   result += c; break;
        }
    }
    return result;
}

// Get a color based on node classification
string getNodeColor(const ShaderNode& node)
{
    if (node.hasClassification(ShaderNode::Classification::CLOSURE))
        return "#ffcccc";  // Light red for closures/BSDFs
    if (node.hasClassification(ShaderNode::Classification::SHADER))
        return "#ccffcc";  // Light green for shaders
    if (node.hasClassification(ShaderNode::Classification::TEXTURE))
        return "#ccccff";  // Light blue for textures
    if (node.hasClassification(ShaderNode::Classification::CONSTANT))
        return "#ffffcc";  // Light yellow for constants
    return "#ffffff";      // White for others
}

// Make a valid DOT identifier from a name
string makeDotId(const string& name)
{
    string result;
    result.reserve(name.size());
    for (char c : name)
    {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
            result += c;
        else
            result += '_';
    }
    return result;
}

} // anonymous namespace

void writeShaderGraphDot(const ShaderGraph& graph, std::ostream& os)
{
    os << "digraph \"" << escapeLabel(graph.getName()) << "\" {\n";
    os << "  rankdir=TB;\n";
    os << "  node [shape=record, fontname=\"Helvetica\", fontsize=10];\n";
    os << "  edge [fontname=\"Helvetica\", fontsize=8];\n";
    os << "\n";

    // Write nodes
    for (const ShaderNode* node : graph.getNodes())
    {
        const string nodeId = makeDotId(node->getName());
        const string color = getNodeColor(*node);

        // Build label with inputs and outputs
        std::ostringstream label;
        
        // Inputs section
        label << "{";
        bool firstInput = true;
        for (ShaderInput* input : node->getInputs())
        {
            if (!firstInput) label << "|";
            firstInput = false;
            label << "<" << makeDotId(input->getName()) << ">" 
                  << escapeLabel(input->getName());
            
            // Show value if no connection and has value
            if (!input->getConnection() && input->getValue())
            {
                label << "=" << escapeLabel(input->getValue()->getValueString());
            }
        }
        label << "}";
        
        // Node name section
        label << "|" << escapeLabel(node->getName());
        
        // Outputs section
        label << "|{";
        bool firstOutput = true;
        for (ShaderOutput* output : node->getOutputs())
        {
            if (!firstOutput) label << "|";
            firstOutput = false;
            label << "<" << makeDotId(output->getName()) << ">" 
                  << escapeLabel(output->getName());
        }
        label << "}";

        os << "  " << nodeId << " [label=\"{" << label.str() << "}\", "
           << "style=filled, fillcolor=\"" << color << "\"];\n";
    }

    os << "\n";

    // Write edges (connections)
    for (const ShaderNode* node : graph.getNodes())
    {
        const string downstreamId = makeDotId(node->getName());
        
        for (ShaderInput* input : node->getInputs())
        {
            ShaderOutput* upstream = input->getConnection();
            if (upstream && upstream->getNode())
            {
                const string upstreamId = makeDotId(upstream->getNode()->getName());
                const string upstreamPort = makeDotId(upstream->getName());
                const string downstreamPort = makeDotId(input->getName());
                
                os << "  " << upstreamId << ":" << upstreamPort 
                   << " -> " << downstreamId << ":" << downstreamPort;
                
                // Add edge label with type
                os << " [label=\"" << escapeLabel(input->getType().getName()) << "\"]";
                os << ";\n";
            }
        }
    }

    // Write graph output sockets
    os << "\n  // Graph outputs\n";
    os << "  graph_output [label=\"Output\", shape=ellipse, style=filled, fillcolor=\"#ccffcc\"];\n";
    
    for (ShaderGraphOutputSocket* outputSocket : graph.getOutputSockets())
    {
        ShaderOutput* upstream = outputSocket->getConnection();
        if (upstream && upstream->getNode())
        {
            const string upstreamId = makeDotId(upstream->getNode()->getName());
            const string upstreamPort = makeDotId(upstream->getName());
            
            os << "  " << upstreamId << ":" << upstreamPort 
               << " -> graph_output [style=bold];\n";
        }
    }

    os << "}\n";
}

void writeShaderGraphDot(const ShaderGraph& graph, const FilePath& path)
{
    std::ofstream file(path.asString());
    if (!file)
    {
        throw ExceptionShaderGenError("Could not open file for writing: " + path.asString());
    }
    writeShaderGraphDot(graph, file);
}

MATERIALX_NAMESPACE_END

