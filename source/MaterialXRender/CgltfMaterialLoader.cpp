//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#if defined(_MSC_VER)
    #pragma optimize( "", off )
#endif

#include <MaterialXCore/Value.h>
#include <MaterialXCore/Types.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXRender/CgltfMaterialLoader.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
#endif

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif

//#define CGLTF_IMPLEMENTATION -- don't set to avoid duplicate symbols
#include <MaterialXRender/External/Cgltf/cgltf.h>
#define CGLTF_WRITE_IMPLEMENTATION
#include <MaterialXRender/External/Cgltf/cgltf_write.h>

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

#include <cstring>
#include <iostream>
#include <limits>

MATERIALX_NAMESPACE_BEGIN

namespace
{

void initialize_cgltf_texture_view(cgltf_texture_view& textureview)
{
    textureview.texture = nullptr;
    textureview.texcoord = 0;
    textureview.scale = 1.0;
    textureview.has_transform = false;
    textureview.extras.start_offset = 0;
    textureview.extras.end_offset = 0;
    textureview.extensions_count = 0;
    textureview.extensions = nullptr;
}

void initialize_cgtlf_texture(cgltf_texture& texture, const string& name, const string& uri, 
                              cgltf_image* image)
{
    texture.has_basisu = false;
    texture.extras.start_offset = 0;
    texture.extras.end_offset = 0;
    texture.extensions_count = 0;
    texture.sampler = nullptr;
    texture.image = image;
    texture.image->extras.start_offset = 0;
    texture.image->extras.end_offset = 0;
    texture.image->extensions_count = 0;
    texture.image->buffer_view = nullptr;
    texture.image->mime_type = nullptr;
    texture.image->name = const_cast<char*>((new string(name))->c_str());
    texture.name = texture.image->name;
    texture.image->uri = const_cast<char*>((new string(uri))->c_str());
}

void get_image_filename(NodePtr imageNode, string& filename)
{
    InputPtr fileInput = imageNode->getInput("file");
    filename = fileInput && fileInput->getAttribute("type") == "filename" ?
        fileInput->getValueString() : EMPTY_STRING;
}

}

bool CgltfMaterialLoader::save(const FilePath& filePath)
{
    if (!_materials)
    {
        return false;
    }

    const string ext = stringToLower(filePath.getExtension());
    const string BINARY_EXTENSION = "glb";
    const string ASCII_EXTENSION = "gltf";
    if (ext != BINARY_EXTENSION && ext != ASCII_EXTENSION)
    {
        return false;
    }

    cgltf_options options;
    std::memset(&options, 0, sizeof(options));
    cgltf_data* data = new cgltf_data();
	data->file_type = (ext == BINARY_EXTENSION) ? cgltf_file_type::cgltf_file_type_glb : cgltf_file_type::cgltf_file_type_gltf;
    data->file_data = nullptr;
	//cgltf_asset asset;
	data->meshes = nullptr;
	data->meshes_count = 0;
	data->materials = nullptr;
	data->materials_count = 0;
	data->accessors = nullptr;
	data->accessors_count = 0;
	data->buffer_views = nullptr;
	data->buffer_views_count = 0;
	data->buffers = nullptr;
	data->buffers_count = 0;
	data->images = nullptr;
	data->images_count = 0;
	data->textures = nullptr;
	data->textures_count = 0;
	data->samplers = nullptr;
	data->samplers_count = 0;
	data->skins = nullptr;
    data->skins_count = 0;
    data->cameras = nullptr;
	data->cameras_count = 0;
	data->lights = nullptr;
	data->lights_count = 0;
	data->nodes = nullptr;
	data->nodes_count = 0;
	data->scenes = nullptr;
    data->scenes_count = 0;
	data->scene = nullptr;
	data->animations = nullptr;
	data->animations_count = 0;
	data->variants = nullptr;
	data->variants_count = 0;
	//cgltf_extras extras;
	data->data_extensions_count = 0;
	data->data_extensions = nullptr;
	data->extensions_used = nullptr;
	data->extensions_used_count = 0;
	data->extensions_required = nullptr;
	data->extensions_required_count = 0;
	data->json = nullptr;
	data->json_size = 0;
	data->bin = nullptr;
	data->bin_size = 0;

	data->asset.generator = const_cast<char*>((new string("MaterialX 1.38.4 to glTF generator"))->c_str());
    data->asset.version = const_cast<char*>((new string("2.0"))->c_str());
    data->asset.min_version = const_cast<char*>((new string("2.0"))->c_str());

    // Scan for PBR shader nodes
    const string PBR_CATEGORY_STRING("gltf_pbr");
    std::set<NodePtr> pbrNodes;
    for (const NodePtr& material : _materials->getMaterialNodes())
    {
        vector<NodePtr> shaderNodes = getShaderNodes(material);
        for (const NodePtr& shaderNode : shaderNodes)
        {
            if (shaderNode->getCategory() == PBR_CATEGORY_STRING &&
                pbrNodes.find(shaderNode) == pbrNodes.end())
            {
                pbrNodes.insert(shaderNode);
            }
        }
    }

    cgltf_size materials_count = pbrNodes.size();
    if (!materials_count)
    {
        return false;
    }

    // Write materials
    /*
    * typedef struct cgltf_material
    {
	    char* name;
	    cgltf_bool has_pbr_metallic_roughness;
	    cgltf_bool has_pbr_specular_glossiness;
	    cgltf_bool has_clearcoat;
	    cgltf_bool has_transmission;
	    cgltf_bool has_volume;
	    cgltf_bool has_ior;
	    cgltf_bool has_specular;
	    cgltf_bool has_sheen;
	    cgltf_bool has_emissive_strength;
	    cgltf_pbr_metallic_roughness pbr_metallic_roughness;
	    cgltf_pbr_specular_glossiness pbr_specular_glossiness;
	    cgltf_clearcoat clearcoat;
	    cgltf_ior ior;
	    cgltf_specular specular;
	    cgltf_sheen sheen;
	    cgltf_transmission transmission;
	    cgltf_volume volume;
	    cgltf_emissive_strength emissive_strength;
	    cgltf_texture_view normal_texture;
	    cgltf_texture_view occlusion_texture;
	    cgltf_texture_view emissive_texture;
	    cgltf_float emissive_factor[3];
	    cgltf_alpha_mode alpha_mode;
	    cgltf_float alpha_cutoff;
	    cgltf_bool double_sided;
	    cgltf_bool unlit;
	    cgltf_extras extras;
	    cgltf_size extensions_count;
	    cgltf_extension* extensions;
    } cgltf_material;
    */
    cgltf_material* materials = new cgltf_material[materials_count];
    data->materials = materials;
    data->materials_count = materials_count;

    // Set of image nodes.
    cgltf_texture* textures = new cgltf_texture[4*materials_count];
    cgltf_image* images = new cgltf_image[4*materials_count];

    size_t i = 0;
    size_t imageIndex = 0;
    for (const NodePtr& pbrNode : pbrNodes)
    {
        cgltf_material* material = &(materials[i]);
	    material->has_pbr_metallic_roughness = false;
	    material->has_pbr_specular_glossiness = false;
	    material->has_clearcoat = false;
	    material->has_transmission = false;
	    material->has_volume = false;
	    material->has_ior = false;
	    material->has_specular = false;
	    material->has_sheen = false;
	    material->has_emissive_strength = false;
	    material->extensions_count = 0;
        material->extensions = nullptr;
        material->emissive_texture.texture = nullptr;
        material->normal_texture.texture = nullptr;
        material->occlusion_texture.texture = nullptr;
        material->unlit = false;
        material->double_sided = false;

        material->emissive_factor[0] = 0.0;
        material->emissive_factor[1] = 0.0;
        material->emissive_factor[2] = 0.0;

        string* name = new string(pbrNode->getNamePath());
        material->name = const_cast<char*>(name->c_str());
        
        material->has_pbr_metallic_roughness = true;
        cgltf_pbr_metallic_roughness& roughness = material->pbr_metallic_roughness;
        initialize_cgltf_texture_view(roughness.base_color_texture);

        // Handle base color
        string filename;
        NodePtr imageNode = pbrNode->getConnectedNode("base_color");
        if (imageNode)
        {
            get_image_filename(imageNode, filename);
            if (!filename.empty())
            {
                cgltf_texture* texture = &(textures[imageIndex]);
                roughness.base_color_texture.texture = texture;
                initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                                         &(images[imageIndex]));

                roughness.base_color_factor[0] = 1.0;
                roughness.base_color_factor[1] = 1.0;
                roughness.base_color_factor[2] = 1.0;
                roughness.base_color_factor[3] = 1.0;

                imageIndex++;
            }
        }
        else
        {
            ValuePtr value = pbrNode->getInputValue("base_color");
            if (value)
            {
                Color3 color = value->asA<Color3>();
                roughness.base_color_factor[0] = color[0];
                roughness.base_color_factor[1] = color[1];
                roughness.base_color_factor[2] = color[2];
            }

            value = pbrNode->getInputValue("alpha");
            if (value)
            {
                roughness.base_color_factor[3] = value->asA<float>();
            }
        }

        // Handle emissive texture
        imageNode = nullptr;
        imageNode = pbrNode->getConnectedNode("emissive");
        if (imageNode)
        {
            get_image_filename(imageNode, filename);
            if (!filename.empty())
            {
                cgltf_texture_view& emissive = material->emissive_texture;
                initialize_cgltf_texture_view(emissive);

                cgltf_texture* texture = &(textures[imageIndex]);
                emissive.texture = texture;
                initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                                         &(images[imageIndex]));

                ValuePtr value = pbrNode->getInputValue("emissive_strength");
                float emissiveStrength = value->asA<float>();

                material->emissive_factor[0] = emissiveStrength;
                material->emissive_factor[1] = emissiveStrength;
                material->emissive_factor[2] = emissiveStrength;

                imageIndex++; 
            }
        }

        // Handle normals
        NodePtr normalNode = nullptr;
        imageNode = nullptr;
        normalNode = pbrNode->getConnectedNode("normal");
        if (normalNode)
        {
            imageNode = normalNode->getConnectedNode("in");
            if (imageNode)
            {
                get_image_filename(imageNode, filename);
                if (!filename.empty())
                {
                    cgltf_texture_view& normal = material->normal_texture;
                    initialize_cgltf_texture_view(normal);

                    cgltf_texture* texture = &(textures[imageIndex]);
                    normal.texture = texture;
                    initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                                             &(images[imageIndex]));

                    imageIndex++;
                }
            }
        }

        // Handle metallic, roughness, occlusion
        initialize_cgltf_texture_view(roughness.metallic_roughness_texture);
        ValuePtr value;
        string extractInputs[3] =
        {
            "metallic",
            "roughness",
            "occlusion"
        };
        cgltf_float* roughnessInputs[3] =
        {
            &roughness.metallic_factor,
            &roughness.roughness_factor,
            nullptr
        };

        NodePtr ormNode = nullptr;
        imageNode = nullptr;
        const string extractCategory("extract");
        for (size_t e=0; e<3; e++)
        { 
            const string& inputName = extractInputs[e];
            InputPtr pbrInput = pbrNode->getInput(inputName);            
            if (pbrInput)
            {
                // Read past any extract node
                NodePtr extractNode = pbrNode->getConnectedNode(inputName);
                if (extractNode && extractNode->getCategory() == extractCategory)
                {
                    imageNode = extractNode->getConnectedNode("in");
                }

                if (imageNode)
                {
                    // Only create the ORM texture once
                    if (!ormNode)
                    {
                        ormNode = imageNode;

                        cgltf_texture_view& occlusion = material->occlusion_texture;
                        initialize_cgltf_texture_view(occlusion);

                        get_image_filename(imageNode, filename);

                        cgltf_texture* texture = &(textures[imageIndex]);
                        roughness.metallic_roughness_texture.texture = texture;
                        // Assign occlusion texture
                        // TODO: occlusion is not always assigned via ORM texture
                        occlusion.texture = texture;
                        initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                            &(images[imageIndex]));

                        imageIndex++;
                    }

                    if (roughnessInputs[e])
                    {
                        *(roughnessInputs[e]) = 1.0f;
                    }
                }
                else
                {
                    if (roughnessInputs[e])
                    {
                        value = pbrInput->getValue();
                        *(roughnessInputs[e]) = value->asA<float>();
                    }
                }
            }
        }
        /* if (roughnessInput)
        {
            value = roughnessInput->getValue();
            roughness.roughness_factor = value->asA<float>();
        }*/

        i++;
    }

    // Set image and texture lists
    data->images_count = imageIndex;
    data->textures_count = imageIndex; 
    data->textures = textures;
    data->images = images;

    // Write to disk
    cgltf_result result = cgltf_write_file(&options, filePath.asString().c_str(), data);
    if (result != cgltf_result_success)
    {
        return false;
    }
    return true;
}

bool CgltfMaterialLoader::load(const FilePath& filePath)
{
    const string input_filename = filePath.asString();
    const string ext = stringToLower(filePath.getExtension());
    const string BINARY_EXTENSION = "glb";
    const string ASCII_EXTENSION = "gltf";
    if (ext != BINARY_EXTENSION && ext != ASCII_EXTENSION)
    {
        return false;
    }

    cgltf_options options;
    std::memset(&options, 0, sizeof(options));
    cgltf_data* data = new cgltf_data;

    // Read file
    cgltf_result result = cgltf_parse_file(&options, input_filename.c_str(), &data);
    if (result != cgltf_result_success)
    {
        return false;
    }
    if (cgltf_load_buffers(&options, data, input_filename.c_str()) != cgltf_result_success)
    {
        return false;
    }

    loadMaterials(data);

    if (data)
    {
        cgltf_free(data);
    }

    return true;
}

// Utilities
namespace
{
NodePtr createTexture(DocumentPtr& doc, const string & nodeName, const string & fileName,
                   const string & textureType, const string & colorspace)
{
    string newTextureName = doc->createValidChildName(nodeName);
    NodePtr newTexture = doc->addNode("tiledimage", newTextureName, textureType);
    newTexture->setAttribute("nodedef", "ND_image_" + textureType);
    newTexture->addInputsFromNodeDef();
    InputPtr fileInput = newTexture->getInput("file");
    fileInput->setValue(fileName, "filename");
    if (!colorspace.empty())
    {
        fileInput->setAttribute("colorspace", colorspace);
    }
    return newTexture;
}

const string SPACE_STRING = " ";

void setColorInput(DocumentPtr materials, NodePtr shaderNode, const string& inputName, 
                   const Color3& colorFactor, const cgltf_texture_view* textureView,
                   const string& inputImageNodeName)
{
    ValuePtr color3Value = Value::createValue<Color3>(colorFactor);
    InputPtr colorInput = shaderNode->getInput(inputName);
    if (colorInput)
    {
        cgltf_texture* texture = textureView ? textureView->texture : nullptr;
        if (texture && texture->image)
        {
            string imageNodeName = texture->image->name ? texture->image->name : inputImageNodeName;
            imageNodeName = materials->createValidChildName(imageNodeName);
            string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
            NodePtr newTexture = createTexture(materials, imageNodeName, uri,
                "color3", "srgb_texture");
            colorInput->setAttribute("nodename", newTexture->getName());
        }
        else
        {
            colorInput->setValueString(color3Value->getValueString());
        }
    }
}

void setFloatInput(DocumentPtr materials, NodePtr shaderNode, const string& inputName, 
                   float floatFactor, const cgltf_texture_view* textureView,
                   const string& inputImageNodeName)
{
    InputPtr floatInput = shaderNode->getInput(inputName);
    if (floatInput)
    {
        cgltf_texture* texture = textureView ? textureView->texture : nullptr;
        if (texture && texture->image)
        {
            string imageNodeName = texture->image->name ? texture->image->name :
                "image_sheen_roughness";
            imageNodeName = materials->createValidChildName(inputImageNodeName);
            string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
            NodePtr newTexture = createTexture(materials, imageNodeName, uri,
                "float", EMPTY_STRING);
            floatInput->setAttribute("nodename", newTexture->getName());
        }
        else
        {
            floatInput->setValue<float>(floatFactor);
        }
    }
}

}

void CgltfMaterialLoader::loadMaterials(void *vdata)
{
    cgltf_data* data = static_cast<cgltf_data*>(vdata);

    // Scan materials
    /*
    * typedef struct cgltf_material
    {
	    char* name;
	    cgltf_bool has_pbr_metallic_roughness;
	    cgltf_bool has_pbr_specular_glossiness;
	    cgltf_bool has_clearcoat;
	    cgltf_bool has_transmission;
	    cgltf_bool has_volume;
	    cgltf_bool has_ior;
	    cgltf_bool has_specular;
	    cgltf_bool has_sheen;
	    cgltf_bool has_emissive_strength;
	    cgltf_pbr_metallic_roughness pbr_metallic_roughness;
	    cgltf_pbr_specular_glossiness pbr_specular_glossiness;
	    cgltf_clearcoat clearcoat;
	    cgltf_ior ior;
	    cgltf_specular specular;
	    cgltf_sheen sheen;
	    cgltf_transmission transmission;
	    cgltf_volume volume;
	    cgltf_emissive_strength emissive_strength;
	    cgltf_texture_view normal_texture;
	    cgltf_texture_view occlusion_texture;
	    cgltf_texture_view emissive_texture;
	    cgltf_float emissive_factor[3];
	    cgltf_alpha_mode alpha_mode;
	    cgltf_float alpha_cutoff;
	    cgltf_bool double_sided;
	    cgltf_bool unlit;
	    cgltf_extras extras;
	    cgltf_size extensions_count;
	    cgltf_extension* extensions;
    } cgltf_material;
    */
    if (data->materials_count)
    {
        _materials = Document::createDocument<Document>();
        _materials->importLibrary(_definitions);
    }
    size_t materialId = 0;
    const string SHADER_PREFIX = "Shader_";
    const string MATERIAL_PREFIX = "MATERIAL_";
    for (size_t m = 0; m < data->materials_count; m++)
    {
        cgltf_material* material = &(data->materials[m]);
        if (!material)
        {
            continue;
        }

        // Create a default gltf_pbr node
        string matName = material->name ? material->name : EMPTY_STRING;
        if (!matName.empty() && std::isdigit(matName[0]))
        {
            matName = SHADER_PREFIX + matName;
        }
        string shaderName = matName.empty() ? SHADER_PREFIX + std::to_string(materialId) : matName;
        shaderName = _materials->createValidChildName(shaderName);
        NodePtr shaderNode = _materials->addNode("gltf_pbr", shaderName, "surfaceshader");
        shaderNode->setAttribute("nodedef", "ND_gltf_pbr_surfaceshader");
        shaderNode->addInputsFromNodeDef();

        // Create a surface material for the shader node
        string materialName = matName.empty() ? MATERIAL_PREFIX + std::to_string(materialId) : MATERIAL_PREFIX + matName;
        materialName = _materials->createValidChildName(materialName);
        NodePtr materialNode = _materials->addNode("surfacematerial", materialName, "material");
        InputPtr shaderInput = materialNode->addInput("surfaceshader", "surfaceshader");
        shaderInput->setAttribute("nodename", shaderNode->getName());

        if (material->has_pbr_metallic_roughness)
        {
            cgltf_pbr_metallic_roughness& roughness = material->pbr_metallic_roughness;

            Color3 colorFactor(roughness.base_color_factor[0],
                roughness.base_color_factor[1],
                roughness.base_color_factor[2]);
            setColorInput(_materials, shaderNode, "base_color",
                colorFactor, &roughness.base_color_texture, "image_basecolor");


            // Alpha
            InputPtr alphaInput = shaderNode->getInput("alpha");
            if (alphaInput)
            {
                alphaInput->setValue<float>(roughness.base_color_factor[3]);
            }

            // Set metalic, roughness, and occlusion
            InputPtr metallicInput = shaderNode->getInput("metallic");
            InputPtr roughnessInput = shaderNode->getInput("roughness");
            InputPtr occlusionInput = shaderNode->getInput("occlusion");

            // Check for occlusion/metallic/roughness texture
            cgltf_texture_view& textureView = roughness.metallic_roughness_texture;
            cgltf_texture* texture = textureView.texture;
            if (texture && texture->image)
            {
                string imageNodeName = texture->image->name ? texture->image->name :
                    "image_orm";
                imageNodeName = _materials->createValidChildName(imageNodeName);
                string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
                NodePtr textureNode = createTexture(_materials, imageNodeName, uri,
                    "vector3", EMPTY_STRING);

                // Add extraction nodes. Note that order matters
                StringVec extractNames =
                {
                    _materials->createValidChildName("extract_occlusion"),
                    _materials->createValidChildName("extract_roughness"),
                    _materials->createValidChildName("extract_metallic")
                };
                std::vector<InputPtr> inputs =
                {
                    occlusionInput, roughnessInput, metallicInput
                };
                for (size_t i = 0; i < extractNames.size(); i++)
                {
                    NodePtr extractNode = _materials->addNode("extract", extractNames[i], "float");
                    extractNode->setAttribute("nodedef", "ND_extract_vector3");
                    extractNode->addInputsFromNodeDef();
                    extractNode->getInput("in")->setAttribute("nodename", textureNode->getName());
                    extractNode->getInput("in")->setType("vector3");
                    extractNode->getInput("index")->setAttribute("value", std::to_string(i));
                    if (inputs[i])
                    {
                        inputs[i]->setAttribute("nodename", extractNode->getName());
                        inputs[i]->setType("float");
                    }
                }
            }
            else
            {
                metallicInput->setValue<float>(roughness.metallic_factor);
                roughnessInput->setValue<float>(roughness.roughness_factor);
            }
        }

        // Normal texture
        InputPtr normalInput = shaderNode->getInput("normal");

        cgltf_texture_view& normalView = material->normal_texture;
        cgltf_texture* normalTexture = normalView.texture;
        if (normalTexture && normalTexture->image)
        {
            string imageNodeName = normalTexture->image->name ? normalTexture->image->name :
                "image_normal";
            imageNodeName = _materials->createValidChildName(imageNodeName);
            string uri = normalTexture->image->uri ? normalTexture->image->uri : SPACE_STRING;
            NodePtr newTexture = createTexture(_materials, imageNodeName, uri,
                "vector3", EMPTY_STRING);

            string normalMapName = _materials->createValidChildName("pbr_normalmap");
            NodePtr normalMap = _materials->addNode("normalmap", normalMapName, "vector3");
            normalMap->setAttribute("nodedef", "ND_normalmap");
            normalMap->addInputsFromNodeDef();
            normalMap->getInput("in")->setAttribute("nodename", newTexture->getName());
            normalMap->getInput("in")->setType("vector3");

            normalInput->setAttribute("nodename", normalMap->getName());
        }

        // Handle sheen
        if (material->has_sheen)
        {
            cgltf_sheen& sheen = material->sheen;

            Color3 colorFactor(sheen.sheen_color_factor[0],
                                    sheen.sheen_color_factor[1],
                                    sheen.sheen_color_factor[2]);
            setColorInput(_materials, shaderNode, "sheen_color",
                colorFactor, &sheen.sheen_color_texture, "image_sheen");

            setFloatInput(_materials, shaderNode, "sheen_roughness",
                sheen.sheen_roughness_factor, &sheen.sheen_roughness_texture,
                "image_sheen_roughness");
        }

        // Handle clearcoat
        if (material->has_clearcoat)
        {
            cgltf_clearcoat& clearcoat = material->clearcoat;

            setFloatInput(_materials, shaderNode, "clearcoat",
                clearcoat.clearcoat_factor, 
                &clearcoat.clearcoat_texture,
                "image_clearcoat");

            setFloatInput(_materials, shaderNode, "clearcoat_roughness",
                clearcoat.clearcoat_roughness_factor, 
                &clearcoat.clearcoat_roughness_texture,
                "image_clearcoat_roughness");           

            // TODO: Handle clearcoat_normal
        }

        // Handle tranmission
        if (material->has_transmission)
        {
            cgltf_transmission& transmission = material->transmission;

            setFloatInput(_materials, shaderNode, "transmission",
                transmission.transmission_factor, 
                &transmission.transmission_texture,
                "image_transmission");
        }

        // Handle specular and specular color
        if (material->has_specular)
        {
            cgltf_specular& specular = material->specular;

            Color3 colorFactor(specular.specular_color_factor[0],
                specular.specular_color_factor[1],
                specular.specular_color_factor[2]);
            setColorInput(_materials, shaderNode, "specular_color",
                colorFactor, 
                &specular.specular_color_texture, 
                "image_specularcolor");

            setFloatInput(_materials, shaderNode, "specular",
                specular.specular_factor, 
                &specular.specular_texture,
                "image_specular");
        }

        // Set ior
        if (material->has_ior)
        {
            InputPtr iorInput = shaderNode->getInput("ior");
            if (iorInput)
            {
                cgltf_ior& ior = material->ior;
                iorInput->setValue<float>(ior.ior);
            }
        }

        // Set emissive inputs
        Color3 colorFactor(material->emissive_factor[0],
            material->emissive_factor[1],
            material->emissive_factor[2]);
        setColorInput(_materials, shaderNode, "emissive",
            colorFactor, &material->emissive_texture, "image_emission");

        if (material->has_emissive_strength)
        {
            InputPtr input = shaderNode->getInput("emissive_strength");
            if (input)
            {
                cgltf_emissive_strength& emissive_strength = material->emissive_strength;
                input->setValue<float>(emissive_strength.emissive_strength);
            }
        }

        // Volume inputs
        if (material->has_volume)
        {
            cgltf_volume& volume = material->volume;

            setFloatInput(_materials, shaderNode, "thickness",
                volume.thickness_factor,
                &volume.thickness_texture,
                "thickness");

            Color3 attenFactor(volume.attenuation_color[0],
                volume.attenuation_color[1],
                volume.attenuation_color[2]);
            setColorInput(_materials, shaderNode, "attenuation_color",
                          attenFactor, nullptr, EMPTY_STRING);

            setFloatInput(_materials, shaderNode, "attenuation_distance",
                volume.attenuation_distance, nullptr, EMPTY_STRING);
        }
    }
}

MATERIALX_NAMESPACE_END

#if defined(_MSC_VER)
    #pragma optimize( "", on )
#endif