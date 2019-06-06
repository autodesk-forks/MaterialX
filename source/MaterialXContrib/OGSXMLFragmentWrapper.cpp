//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXContrib/OGSXMLFragmentWrapper.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/PugiXML/pugixml.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

namespace MaterialX
{
OGSXMLFragmentWrapper::OGSXMLFragmentWrapper(GenContext* context) :
    _context(context)
{
    _xmlDocument = new pugi::xml_document();

    // Initialize mappings from MTLX to OGS
    _typeMap["boolean"] = "bool";
    _typeMap["float"] = "float";
    _typeMap["integer"] = "int";
    _typeMap["string"] = "int";
    _typeMap[MaterialX::TypedValue<MaterialX::Matrix44>::TYPE] = "float4x4";
    // There is no mapping for this, so binder needs to promote from 3x3 to 4x4 before binding.
    _typeMap[MaterialX::TypedValue<MaterialX::Matrix33>::TYPE] = "float4x4";
    _typeMap[MaterialX::TypedValue<MaterialX::Color2>::TYPE] = "float2";
    _typeMap[MaterialX::TypedValue<MaterialX::Color3>::TYPE] = "float3";
    // To determine if this reqiures a struct creation versus allowing for colorAlpha.
    // For now just use float4 as it's generic
    //_typeMap[MaterialX::TypedValue<MaterialX::Color4>::TYPE] = "colorAlpha";
    _typeMap[MaterialX::TypedValue<MaterialX::Color4>::TYPE] = "float4";
    _typeMap[MaterialX::TypedValue<MaterialX::Vector2>::TYPE] = "float2";
    _typeMap[MaterialX::TypedValue<MaterialX::Vector3>::TYPE] = "float3";
    _typeMap[MaterialX::TypedValue<MaterialX::Vector4>::TYPE] = "float4";
}

OGSXMLFragmentWrapper::~OGSXMLFragmentWrapper()
{
    delete (static_cast<pugi::xml_document*>(_xmlDocument));
}

namespace
{ 
// XML constant strings
const string XML_TAB_STRING("  ");
const string XML_NAME_STRING("name");
const string XML_VALUE_STRING("value");

// MaterialX constant strings
const string MTLX_GENHW_POSITION("i_position");
const string MTLX_GENHW_COLORSET("i_color_");
const string MTLX_GENHW_UVSET("i_texcoord_");
const string MTLX_GENHW_NORMAL("i_normal");
const string MTLX_GENHW_TANGENT("i_tangent");
const string MTLX_GENHW_BITANGENT("i_bitangent");
const string MTLX_GENSHADER_FILENAME("filename");

// OGS constant strings
const string OGS_FRAGMENT("fragment");
const string OGS_FRAGMENT_NAME("name");
const string OGS_FRAGMENT_UI_NAME("uiName");
const string OGS_FRAGMENT_TYPE("type");
const string OGS_FRAGMENT_CLASS("class");
const string OGS_FRAGMENT_VERSION("version");
const string OGS_FRAGMENT_TYPE_PLUMBING("plumbing");
const string OGS_FRAGMENT_CLASS_SHADERFRAGMENT("ShadeFragment");
const string OGS_FRAGMENT_DESCRIPTION("description");
const string OGS_PROPERTIES("properties");
const string OGS_GLOBAL_PROPERTY("isRequirementOnly");
const string OGS_VALUES("values");
const string OGS_OUTPUTS("outputs");
const string OGS_IMPLEMENTATION("implementation");
const string OGS_RENDER("render");
const string OGS_MAYA_RENDER("OGSRenderer"); // Name of the Maya renderer
const string OGS_LANGUAGE("language");
const string OGS_LANGUAGE_VERSION("lang_version");
const string OGS_GLSL_LANGUAGE("GLSL");
const string OGS_GLSL_LANGUAGE_VERSION("3.0");
const string OGS_FUNCTION_NAME("function_name");
const string OGS_FUNCTION_VAL("val");
const string OGS_SOURCE("source");
const string OGS_VERTEX_SOURCE("vertex_source");
// TODO: Texture and sampler strings need to be per language mappings 
const string OGS_TEXTURE2("texture2");
const string OGS_SAMPLER("sampler");
const string OGS_SEMANTIC("semantic");
const string OGS_FLAGS("flags");
const string OGS_VARYING_INPUT_PARAM("varyingInputParam");
const string OGS_POSITION_WORLD_SEMANTIC("Pw");
const string OGS_POSITION_OBJECT_SEMANTIC("Pm");
const string OGS_NORMAL_WORLD_SEMANTIC("Nw");
const string OGS_NORMAL_OBJECT_SEMANTIC("Nm");
const string OGS_COLORSET_SEMANTIC("colorset");
const string OGS_MAYA_BITANGENT_SEMANTIC("mayaBitangentIn"); // Maya bitangent semantic
const string OGS_MAYA_TANGENT_SEMANTIC("mayaTangentIn"); // Maya tangent semantic
const string OGS_MAYA_UV_COORD_SEMANTIC("mayaUvCoordSemantic");  // Maya uv semantic

void createOGSProperty(pugi::xml_node& propertiesNode, pugi::xml_node& valuesNode,
                                            const string& name,
                                            const string& type,
                                            const string& value,
                                            const string& semantic,
                                            const string& flags,
                                            StringMap& typeMap)
{
    // Special case filename
    if (type == MTLX_GENSHADER_FILENAME)
    {
        // TODO: GLSL is not taking texture as an input argument, only a sampler
        // so mark it as a global. Not for DirectX11 we need both but could still
        // be a global.
        pugi::xml_node txt = propertiesNode.append_child(OGS_TEXTURE2.c_str());
        txt.append_attribute(XML_NAME_STRING.c_str()) = (name + "_texture").c_str();
        txt.append_attribute(OGS_FLAGS.c_str()) = OGS_GLOBAL_PROPERTY.c_str();
        pugi::xml_node samp = propertiesNode.append_child(OGS_SAMPLER.c_str());
        samp.append_attribute(XML_NAME_STRING.c_str()) = (name).c_str();
        // TODO: Temporary make this a global for now to match current code generation
        samp.append_attribute(OGS_FLAGS.c_str()) = OGS_GLOBAL_PROPERTY.c_str();
    }
    else
    {
        string ogsType = typeMap[type];
        if (!typeMap.count(type))
            return;

        pugi::xml_node prop = propertiesNode.append_child(ogsType.c_str());
        prop.append_attribute(XML_NAME_STRING.c_str()) = name.c_str();
        string flagValue = flags;
        if (!semantic.empty())
        {
            prop.append_attribute(OGS_SEMANTIC.c_str()) = semantic.c_str();
        }
        if (!flagValue.empty())
        {
            prop.append_attribute(OGS_FLAGS.c_str()) = flagValue.c_str();
        }

        if (!value.empty())
        {
            pugi::xml_node val = valuesNode.append_child(ogsType.c_str());
            val.append_attribute(XML_NAME_STRING.c_str()) = name.c_str();
            val.append_attribute(XML_VALUE_STRING.c_str()) = value.c_str();
        }
    }
}

// Creates output children on "outputs" node
void createOGSOutput(pugi::xml_node& outputsNode,
                    const string& name,
                    const string& type,
                    const string& semantic,
                    StringMap& typeMap) 
{
    if (!typeMap.count(type))
        return;

    string ogsType = typeMap[type];

    pugi::xml_node prop = outputsNode.append_child(ogsType.c_str());
    prop.append_attribute(XML_NAME_STRING.c_str()) = name.c_str();
    if (!semantic.empty())
    {
        prop.append_attribute(OGS_SEMANTIC.c_str()) = semantic.c_str();
    }
}

void getFlags(const ShaderPort* /*port*/, const string& language, string& flags)
{
    // TODO: Determine how to tell if this is global or a signature argument?
    // For now just use language as OSL code has no globals
    bool isRequirement = (language == "genglsl");
    if (isRequirement)
    {
        flags = OGS_GLOBAL_PROPERTY;
    }
}

// Based on the input uniform name get the OGS semantic to use
// for auto-binding.
void getStreamSemantic(const string& name, string& semantic)
{
    if (name.find(MTLX_GENHW_POSITION) != string::npos)
    {
        // TODO: Determine how to tell if object / model space is required
        semantic = OGS_POSITION_WORLD_SEMANTIC;
    }
    else if (name.find(MTLX_GENHW_UVSET) != string::npos)
    {
        semantic = OGS_MAYA_UV_COORD_SEMANTIC;
    }
    else if (name.find(MTLX_GENHW_NORMAL) != string::npos)
    {
        // TODO: Determine how to tell if object / model space is required
        semantic = OGS_NORMAL_WORLD_SEMANTIC;
    }
    else if (name.find(MTLX_GENHW_TANGENT) != string::npos)
    {
        // TODO: Determine how to tell if object / model space is required
        semantic = OGS_MAYA_TANGENT_SEMANTIC;
    }
    else if (name.find(MTLX_GENHW_BITANGENT) != string::npos)
    {
        // TODO: Determine how to tell if object / model space is required
        semantic = OGS_MAYA_BITANGENT_SEMANTIC;
    }
    else if (name.find(MTLX_GENHW_COLORSET) != string::npos)
    {
        semantic = OGS_COLORSET_SEMANTIC;
    }
}

// Add a new implementation to the implementation list.
void addOGSImplementation(pugi::xml_node& impls, const string& language, const string& languageVersion,
                             const string& functionName, const string& pixelShader, const string& vertexShader)
{
    pugi::xml_node impl = impls.append_child(OGS_IMPLEMENTATION.c_str());
    {
        impl.append_attribute(OGS_RENDER.c_str()) = OGS_MAYA_RENDER.c_str();
        impl.append_attribute(OGS_LANGUAGE.c_str()) = language.c_str();
        impl.append_attribute(OGS_LANGUAGE_VERSION.c_str()) = languageVersion.c_str();
        pugi::xml_node func = impl.append_child(OGS_FUNCTION_NAME.c_str());
        {
            func.append_attribute(OGS_FUNCTION_VAL.c_str()) = functionName.c_str();
        }
        if (!vertexShader.empty())
        {
            pugi::xml_node vertexSource = impl.append_child(OGS_VERTEX_SOURCE.c_str());
            vertexSource.append_child(pugi::node_cdata).set_value(vertexShader.c_str());
        }
        if (!pixelShader.empty())
        {
            pugi::xml_node pixelSource = impl.append_child(OGS_SOURCE.c_str());
            // Outputs something but is the incorrect code currently
            pixelSource.append_child(pugi::node_cdata).set_value(pixelShader.c_str());
        }
    }
}

}

void OGSXMLFragmentWrapper::getVertexUniformSemantic(const string& name, string& semantic) const
{
/* This won't work as is not for GLSL since OGSFX has the string map
    ShaderGenerator& generator = _context->getShaderGenerator();
    const StringMap* semanticMap = generator.getSemanticMap();
    if (semanticMap)
    {
        auto val = semanticMap->find(name);
        if (val != semanticMap->end())
        {
            semantic = val->second;
        }
    }
*/
    const StringMap semanticMap  =
    {
        { "u_worldMatrix", "World" },
        { "u_worldInverseMatrix", "WorldInverse" },
        { "u_worldTransposeMatrix", "WorldTranspose" },
        { "u_worldInverseTransposeMatrix", "WorldInverseTranspose" },

        { "u_viewMatrix", "View" },
        { "u_viewInverseMatrix", "ViewInverse" },
        { "u_viewTransposeMatrix", "ViewTranspose" },
        { "u_viewInverseTransposeMatrix", "ViewInverseTranspose" },

        { "u_projectionMatrix", "Projection" },
        { "u_projectionInverseMatrix", "ProjectionInverse" },
        { "u_projectionTransposeMatrix", "ProjectionTranspose" },
        { "u_projectionInverseTransposeMatrix", "ProjectionInverseTranspose" },

        { "u_worldViewMatrix", "WorldView" },
        { "u_viewProjectionMatrix", "ViewProjection" },
        { "u_worldViewProjectionMatrix", "WorldViewProjection" },

        { "u_viewDirection", "ViewDirection" },
        { "u_viewPosition", "WorldCameraPosition" }
    };
    auto val = semanticMap.find(name);
    if (val != semanticMap.end())
    {
        semantic = val->second;
    }
}

#if 0 // TO DETERMINE IF STILL USEFUL
void OGSXMLFragmentWrapper::createWrapperFromNode(NodePtr node, std::vector<GenContext*> contexts)
{
    NodeDefPtr nodeDef = node->getNodeDef();
    if (!nodeDef)
    {
        return;
    }

    const string OGS_VERSION_STRING(node->getDocument()->getVersionString());

    pugi::xml_node xmlRoot = static_cast<pugi::xml_document*>(_xmlDocument)->append_child(OGS_FRAGMENT.c_str());
    const string& nodeName = node->getName();
    xmlRoot.append_attribute(OGS_FRAGMENT_UI_NAME.c_str()) = nodeName.c_str();
    xmlRoot.append_attribute(OGS_FRAGMENT_NAME.c_str()) = nodeName.c_str();
    _fragmentName = nodeName;
    xmlRoot.append_attribute(OGS_FRAGMENT_TYPE.c_str()) = OGS_FRAGMENT_TYPE_PLUMBING.c_str();
    xmlRoot.append_attribute(OGS_FRAGMENT_CLASS.c_str()) = OGS_FRAGMENT_CLASS_SHADERFRAGMENT.c_str();
    xmlRoot.append_attribute(OGS_FRAGMENT_VERSION.c_str()) = OGS_VERSION_STRING.c_str();

    string description("MaterialX generated code for element: " + nodeName);
    xmlRoot.append_child(OGS_FRAGMENT_DESCRIPTION).c_str()) = pugi::node_cdata).set_value(description.c_str()

    // Scan inputs and parameters and create "properties" and 
    // "values" children from the nodeDef
    string semantic;
    pugi::xml_node xmlProperties = xmlRoot.append_child(OGS_PROPERTIES.c_str());
    pugi::xml_node xmlValues = xmlRoot.append_child(OGS_VALUES.c_str());
    for (auto input : node->getInputs())
    {
        string value = input->getValue() ? input->getValue()->getValueString() : "";

        GeomPropDefPtr geomprop = input->getDefaultGeomProp();
        if (geomprop)
        {
            string geomNodeDefName = "ND_" + geomprop->getGeomProp() + "_" + input->getType();
            NodeDefPtr geomNodeDef = node->getDocument()->getNodeDef(geomNodeDefName);
            if (geomNodeDef)
            {
                string geompropString = geomNodeDef->getAttribute("node");
                if (geompropString == "texcoord")
                {
                    semantic = OGS_MAYA_UV_COORD_SEMANTIC;
                }
            }
        }
        string flags;
        createOGSProperty(xmlProperties, xmlValues,
            input->getName(), input->getType(), value, semantic, flags, _typeMap);
    }
    for (auto input : node->getParameters())
    {
        string value = input->getValue() ? input->getValue()->getValueString() : "";
        string flags;
        createOGSProperty(xmlProperties, xmlValues,
            input->getName(), input->getType(), value, "", flags, _typeMap);
    }

    // Scan outputs and create "outputs"
    pugi::xml_node xmlOutputs = xmlRoot.append_child(OGS_OUTPUTS.c_str());
    // Note: We don't want to attach a CM semantic here since code
    // generation should have added a transform already (i.e. mayaCMSemantic)
    semantic.clear();
    for (auto output : node->getActiveOutputs())
    {
        createOGSOutput(xmlOutputs, output->getName(), output->getType(), semantic, _typeMap);
    }

    pugi::xml_node impls = xmlRoot.append_child(OGS_IMPLEMENTATION.c_str());

    string shaderName(node->getName());
    // Work-around: Need to get a node which can be sampled. Should not be required.
    vector<PortElementPtr> samplers = node->getDownstreamPorts();
    if (!samplers.empty())
    {
        for (auto context : contexts)
        {
            PortElementPtr port = samplers[0];
            ShaderGenerator& generator = context->getShaderGenerator();
            ShaderPtr shader = nullptr;
            try
            {
                shader = generator.generate(shaderName, port, *context);
            }
            catch (Exception& e)
            {
                std::cerr << "Failed to generate source code: " << e.what() << std::endl;
                continue;
            }
            const string& code = shader->getSourceCode();

            // Need to get the actual code via shader generation.
            pugi::xml_node impl = impls.append_child(OGS_IMPLEMENTATION.c_str());
            {
                impl.append_attribute(OGS_RENDER) = OGS_MAYA_RENDER.c_str();
                impl.append_attribute(OGS_LANGUAGE.c_str()) = generator.getLanguage().c_str();
                impl.append_attribute(OGS_LANGUAGE_VERSION.c_str()) = generator.getTarget().c_str();
            }
            pugi::xml_node func = impl.append_child(OGS_FUNCTION_NAME.c_str());
            {
                func.append_attribute(OGS_FUNCTION_VAL.c_str()) = nodeDef->getName().c_str();
            }
            pugi::xml_node source = impl.append_child(OGS_SOURCE.c_str());
            {
                source.append_child(pugi::node_cdata).set_value(code.c_str());
            }
        }
    }
}
#endif

void OGSXMLFragmentWrapper::createWrapper(ElementPtr element)
{
    _pathInputMap.clear();

    string shaderName(element->getName());
    ShaderGenerator& generator = _context->getShaderGenerator();
    ShaderPtr shader = nullptr;
    try
    {
        shader = generator.generate(shaderName, element, *_context);
    }
    catch (Exception& e)
    {
        std::cerr << "Failed to generate source code: " << e.what() << std::endl;
        return;
    }
    if (!shader)
    {
        return;
    }
    const string& pixelShaderCode = shader->getSourceCode();
    if (pixelShaderCode.empty())
    {
        return;
    }

    const string OGS_VERSION_STRING(element->getDocument()->getVersionString());

    pugi::xml_node xmlRoot = static_cast<pugi::xml_document*>(_xmlDocument)->append_child(OGS_FRAGMENT.c_str());
    const string& elementName = element->getName();
    xmlRoot.append_attribute(OGS_FRAGMENT_UI_NAME.c_str()) = elementName.c_str();
    xmlRoot.append_attribute(OGS_FRAGMENT_NAME.c_str()) = elementName.c_str();
    // TODO: determine what is a good unique fragment name to use.
    _fragmentName = elementName.c_str();
    xmlRoot.append_attribute(OGS_FRAGMENT_TYPE.c_str()) = OGS_FRAGMENT_TYPE_PLUMBING.c_str();
    xmlRoot.append_attribute(OGS_FRAGMENT_CLASS.c_str()) = OGS_FRAGMENT_CLASS_SHADERFRAGMENT.c_str();
    xmlRoot.append_attribute(OGS_FRAGMENT_VERSION.c_str()) = OGS_VERSION_STRING.c_str();

    string description("MaterialX generated code for element: " + elementName);
    pugi::xml_node xmlDescription = xmlRoot.append_child(OGS_FRAGMENT_DESCRIPTION.c_str());
    xmlDescription.append_child(pugi::node_cdata).set_value(description.c_str());

    // Scan uniform inputs and create "properties" and "values" children.
    pugi::xml_node xmlProperties = xmlRoot.append_child(OGS_PROPERTIES.c_str());
    pugi::xml_node xmlValues = xmlRoot.append_child(OGS_VALUES.c_str());

    const ShaderStage& ps = shader->getStage(Stage::PIXEL);
    for (auto uniformsIt : ps.getUniformBlocks())
    {
        const VariableBlock& uniforms = *uniformsIt.second;
        // Skip light uniforms
        if (uniforms.getName() == HW::LIGHT_DATA)
        {
            continue;
        }

        for (size_t i=0; i<uniforms.size(); i++)
        {
            const ShaderPort* uniform = uniforms[i];
            if (!uniform)
            {
                continue;
            }
            string name = uniform->getName();
            if (name.empty())
            {
                continue;
            }

            string path = uniform->getPath();
            ValuePtr valuePtr = uniform->getValue();
            string value = valuePtr ? valuePtr->getValueString() : EMPTY_STRING;
            string typeString = uniform->getType()->getName();
            string semantic = uniform->getSemantic();
            string variable = uniform->getVariable();
            string flags;
            getFlags(uniform, generator.getLanguage(), flags);

            createOGSProperty(xmlProperties, xmlValues,
                name, typeString, value, semantic, flags, _typeMap);

            _pathInputMap[path] = name;
        }
    }

    // Set geometric inputs 
    if (shader->hasStage(Stage::VERTEX))
    {
        const ShaderStage& vs = shader->getStage(Stage::VERTEX);
        const VariableBlock& vertexInputs = vs.getInputBlock(HW::VERTEX_INPUTS);
        if (!vertexInputs.empty())
        {
            for (size_t i = 0; i < vertexInputs.size(); ++i)
            {
                const ShaderPort* vertexInput = vertexInputs[i];
                if (!vertexInput)
                {
                    continue;
                }
                // TODO: This name isn't correct since it the vertex shader name
                // and not the pixel shader name. Need to figure out what to
                // do with code gen so that we get the correct name.
                string name = vertexInput->getName();
                if (name.empty())
                {
                    continue;
                }
                ValuePtr valuePtr = vertexInput->getValue();
                string value = valuePtr ? valuePtr->getValueString() : EMPTY_STRING;
                string typeString = vertexInput->getType()->getName();
                string semantic = vertexInput->getSemantic();
                getStreamSemantic(name, semantic);
                // TODO: For now we assume all vertex inputs are "global properties"
                // and not part of the function argument list.
                string flags;
                getFlags(vertexInput, generator.getLanguage(), flags);
                if (!flags.empty())
                {
                    flags += ", ";
                }
                flags += OGS_VARYING_INPUT_PARAM.c_str();

                createOGSProperty(xmlProperties, xmlValues,
                    name, typeString, value, semantic, flags, _typeMap);
            }

            // Output vertex uniforms
            if (_outputVertexShader)
            {
                const VariableBlock& uniformInputs = vs.getUniformBlock(HW::PRIVATE_UNIFORMS);
                for (size_t i = 0; i < uniformInputs.size(); ++i)
                {
                    const ShaderPort* uniformInput = uniformInputs[i];
                    if (!uniformInput)
                    {
                        continue;
                    }
                    string name = uniformInput->getName();
                    if (name.empty())
                    {
                        continue;
                    }
                    ValuePtr valuePtr = uniformInput->getValue();
                    string value = valuePtr ? valuePtr->getValueString() : EMPTY_STRING;
                    string typeString = uniformInput->getType()->getName();
                    string semantic = uniformInput->getSemantic();
                    getVertexUniformSemantic(name, semantic);
                    // TODO: For now we assume all vertex inputs are "global properties"
                    // and not part of the function argument list.
                    string flags = OGS_GLOBAL_PROPERTY;

                    createOGSProperty(xmlProperties, xmlValues,
                        name, typeString, value, semantic, flags, _typeMap);

                }               
            }
        }
    }

    // Scan outputs and create "outputs"
    pugi::xml_node xmlOutputs = xmlRoot.append_child(OGS_OUTPUTS.c_str());
    for (auto uniformsIt : ps.getOutputBlocks())
    {
        const VariableBlock& uniforms = *uniformsIt.second;
        for (size_t i = 0; i < uniforms.size(); ++i)
        {
            const ShaderPort* v = uniforms[i];
            string name = v->getName();
            if (name.empty())
            {
                continue;
            }
            string path = v->getPath();
            string typeString = v->getType()->getName();
            // Note: We don't want to attach a CM semantic here since code
            // generation should have added a transform already (i.e. mayaCMSemantic)
            string semantic = v->getSemantic();
            createOGSOutput(xmlOutputs, name, typeString, semantic, _typeMap);
        }
    }

    pugi::xml_node impls = xmlRoot.append_child(OGS_IMPLEMENTATION.c_str());

    // Need to get the actual code via shader generation.
    string language = generator.getLanguage();
    bool isGLSL = (language == "genglsl");
    string language_version = generator.getTarget(); // This isn't really the version
    if (isGLSL)
    {
        // The language capabilities in OGS are fixed so map to what is available.
        language = OGS_GLSL_LANGUAGE;
        language_version = OGS_GLSL_LANGUAGE_VERSION;
    }

    const string& functionName = ps.getSignature();
    const string& vertexShaderCode = _outputVertexShader ? shader->getSourceCode(Stage::VERTEX) : EMPTY_STRING;

    addOGSImplementation(impls, language, language_version, functionName, pixelShaderCode, vertexShaderCode);
    if (isGLSL)
    {
        // This is not correct code. Only to add in slots
        addOGSImplementation(impls, "HLSL", "11.0", functionName, pixelShaderCode, vertexShaderCode);
        addOGSImplementation(impls, "Cg", "2.1", functionName, pixelShaderCode, vertexShaderCode);
    }
}

void OGSXMLFragmentWrapper::getDocument(std::ostream& stream)
{
    static_cast<pugi::xml_document*>(_xmlDocument)->save(stream, XML_TAB_STRING.c_str());
}

void OGSXMLFragmentWrapper::readDocument(std::istream& istream, std::ostream& ostream)
{
    pugi::xml_document document;
    pugi::xml_parse_result result = document.load(istream);
    if (!result)
    {
        throw ExceptionParseError("Error parsing input XML stream");
    }
    document.save(ostream, XML_TAB_STRING.c_str());
}


} // namespace MaterialX
