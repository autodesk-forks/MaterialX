// Compile if module flags were set
#if defined(MATERIALX_TEST_VIEW) && defined(MATERIALX_BUILD_VIEW)

// Run only on supported platforms
#include <MaterialXView/Window/HardwarePlatform.h>
#if defined(OSWin_) || defined(OSLinux_) || defined(OSMac_)

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Swizzle.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/HwLightHandler.h>

#include <fstream>

namespace mx = MaterialX;

#include <iostream>
#include <MaterialXView/ShaderValidators/Glsl/GlslValidator.h>
#include <MaterialXView/Handlers/TinyEXRImageHandler.h>

#define M_PI 3.14159265358979323846

#define LOG_TO_FILE

extern void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc);
extern void createLightRig(mx::DocumentPtr doc, mx::HwLightHandler& lightHandler, mx::HwShaderGenerator& shadergen);
extern void createExampleMaterials(mx::DocumentPtr doc, std::vector<mx::MaterialPtr>& materials);

TEST_CASE("GLSL Source", "[shadervalid]")
{
#ifdef LOG_TO_FILE
    std::ofstream logfile("log_shadervalid_glsl_source.txt");
    std::ostream& log(logfile);
#else
    std::ostream& log(std::cout);
#endif

    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);

    mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
    createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

    // Initialize a GLSL validator and set image handler.
    // Validator initiazation will create a offscreen
    // window and offscreen OpenGL context for usage.
    mx::GlslValidatorPtr validator = mx::GlslValidator::create();
    mx::TinyEXRImageHandlerPtr handler = mx::TinyEXRImageHandler::create();
    bool initialized = false;
    bool orthographicsView = true;
    try
    {
        validator->initialize();
        validator->setImageHandler(handler);
        // Set geometry to draw with
        const std::string geometryFile(mx::FilePath::getCurrentPath().asString() + "/documents/Geometry/sphere.obj");
        mx::GeometryHandlerPtr geometryHandler = validator->getGeometryHandler();
        geometryHandler->setIdentifier(geometryFile);
        if (geometryHandler->getIdentifier() == geometryFile)
        {
            orthographicsView = false;
        }
        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError e)
    {
        for (auto error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(initialized);

    // Test through set of fragment and vertex shader stage pairs
    // of files
    const std::vector<std::string> shaderNames =
    {
        "conditionals",
        "hello_world_graph",
        "hello_world_node",
        "hello_world_shaderref",
        "geometric_nodes",
        "subgraph_ex1",
        "subgraph_ex2",
        "test_noise2d",
        "test_noise3d",
        "test_cellnoise2d",
        "test_cellnoise3d",
        "test_fractal3d",
        "example1_surface",
        "example2_surface",
        "example3_surface",
        "example4_surface"
    };

    const std::set<std::string> shadersUseLighting =
    {
        "subgraph_ex2",
        "example1_surface",
        "example2_surface",
        "example3_surface",
        "example4_surface"
    };

    for (auto shaderName : shaderNames)
    {
        log << "------------ Validate shader from source: " << shaderName << std::endl;
        std::string vertexShaderPath = shaderName + ".vert";
        std::string pixelShaderPath = shaderName + ".frag";

        unsigned int stagesFound = 0;
        std::stringstream vertexShaderStream;
        std::stringstream pixelShaderStream;
        std::ifstream shaderFile;
        shaderFile.open(vertexShaderPath);
        if (shaderFile.is_open())
        {
            vertexShaderStream << shaderFile.rdbuf();
            shaderFile.close();
            stagesFound++;
        }
        shaderFile.open(pixelShaderPath);
        if (shaderFile.is_open())
        {
            pixelShaderStream << shaderFile.rdbuf();
            shaderFile.close();
            stagesFound++;
        }

        // To do: Make the dependence on ShaderGen test generated files more explicit
        // so as to avoid the possibility of failure here. For now skip tests if files not
        // found.
        //REQUIRE(stagesFound == 2);
        if (stagesFound != 2)
        {
            continue;
        }

        if (shadersUseLighting.count(shaderName))
        {
            validator->setLightHandler(lightHandler);
        }
        else
        {
            validator->setLightHandler(nullptr);
        }

        // Check program compilation
        bool programCompiled = false;
        mx::GlslProgramPtr program = validator->program();
        try {
            // Set stages and validate.
            // Note that pixel stage is first, then vertex stage
            std::vector<std::string> stages;
            stages.push_back(pixelShaderStream.str());
            stages.push_back(vertexShaderStream.str());

            validator->validateCreation(stages);
            validator->validateInputs();

            programCompiled = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }

            std::string stage = program->getStage(mx::HwShader::VERTEX_STAGE);
            log << ">> Failed vertex stage code:\n";
            log << stage;
            stage = program->getStage(mx::HwShader::PIXEL_STAGE);
            log << ">> Failed pixel stage code:\n";
            log << stage;
        }
        REQUIRE(programCompiled);

        // Check getting uniforms list
        bool uniformsParsed = false;
        try
        {
            program->printUniforms(log);
            uniformsParsed = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(uniformsParsed);

        // Check getting attributes list
        bool attributesParsed = false;
        try
        {
            program->printAttributes(log);
            attributesParsed = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(attributesParsed);

        // Check rendering which includes checking binding
        bool renderSucceeded = false;
        try
        {
            validator->validateRender(orthographicsView);
            renderSucceeded = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(renderSucceeded);

        try
        {
            std::string fileName = shaderName + ".exr";
            validator->save(fileName);
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
        }
    }
}

//
// Create a validator with an image and geometry handler
// If a filename is supplied then a stock geometry of that name will be used if it can be loaded.
// By default if the file can be loaded it is assumed that rendering is done using a perspective
// view vs an orthographic view. This flag argument is updated and returned.
//
static mx::GlslValidatorPtr createValidator(bool& orthographicView, const std::string& fileName,
                                            std::ostream& log)
{
    bool initialized = false;
    orthographicView = true;
    mx::GlslValidatorPtr validator = mx::GlslValidator::create();
    mx::TinyEXRImageHandlerPtr imageHandler = mx::TinyEXRImageHandler::create();
    try
    {
        validator->initialize();
        validator->setImageHandler(imageHandler);
        validator->setLightHandler(nullptr);
        mx::GeometryHandlerPtr geometryHandler = validator->getGeometryHandler();
        std::string geometryFile;
        if (fileName.length())
        {
            geometryFile =  mx::FilePath::getCurrentPath().asString() + "/documents/Geometry/" + fileName;
            geometryHandler->setIdentifier(geometryFile);
        }
        if (geometryHandler->getIdentifier() == geometryFile)
        {
            orthographicView = false;
        }
        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError e)
    {
        for (auto error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(initialized);

    return validator;
}

// Iterator through each node to test by connecting it to a supplied out
// 1. Create the shader and checks for source generation
// 2. Writes doc to disk if valid
// 3. Writes vertex and pixel shaders to disk
// 4. Validates creation / compilation of shader program
// 5. Validates that inputs were created properly
// 6. Validates rendering
// 7. Saves rendered image to disk
//
// Outputs error log if validation fails
//
static void runValidation(const std::string& shaderName, std::vector<mx::ElementPtr>& elements,
                          mx::GlslValidatorPtr validator, mx::ShaderGeneratorPtr shaderGenerator,
                          bool orthographicView, std::vector<mx::DocumentPtr>& docs, std::ostream& log)
{
    mx::SgOptions options;

    for (size_t i = 0; i < elements.size(); i++)
    {
        auto elemPtr = elements[i];
        if (!elemPtr)
        {
            continue;
        }
        auto doc = docs[i];
        if (!doc)
        {
            continue;
        }
        log << "------------ Run validation with element: " << elemPtr->getName() << std::endl;

        mx::ShaderPtr shader = shaderGenerator->generate(shaderName, elemPtr, options);
        mx::HwShaderPtr hwShader = std::dynamic_pointer_cast<mx::HwShader>(shader);
        REQUIRE(hwShader != nullptr);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        mx::writeToXmlFile(doc, elemPtr->getName() + ".mtlx");

        std::ofstream file;
        file.open(elemPtr->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(elemPtr->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();

        // Validate
        MaterialX::GlslProgramPtr program = validator->program();
        bool validated = false;
        try
        {
            validator->validateCreation(hwShader);
            validator->validateInputs();

            program->printUniforms(log);
            program->printAttributes(log);

            validator->validateRender(orthographicView);
            std::string fileName = elemPtr->getName() + ".exr";
            validator->save(fileName);

            validated = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }

            std::string stage = program->getStage(mx::HwShader::VERTEX_STAGE);
            log << ">> Failed vertex stage code:\n";
            log << stage;
            stage = program->getStage(mx::HwShader::PIXEL_STAGE);
            log << ">> Failed pixel stage code:\n";
            log << stage;
        }
        REQUIRE(validated);
    }
}

TEST_CASE("GLSL geometry", "[shadervalid]")
{
#ifdef LOG_TO_FILE
    std::ofstream logfile("log_shadervalid_glsl_geometry.txt");
    std::ostream& log(logfile);
#else
    std::ostream& log(std::cout);
#endif

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    // Stores our documents
    std::vector<mx::DocumentPtr> docs;

    // Store graphs outputs that test some geometric nodes
    std::vector<mx::ElementPtr> outputList;

    // Normal stream test
    mx::DocumentPtr normalDoc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, normalDoc);
    mx::NodeGraphPtr normalNodeGraph = normalDoc->addNodeGraph("normal_nodegraph");
    mx::NodePtr normal1 = normalNodeGraph->addNode("normal", "normal1", "vector3");
    normal1->setParameterValue("space", std::string("world"));
    mx::OutputPtr normalOutput = normalNodeGraph->addOutput("normal_output", "vector3");
    normalOutput->setConnectedNode(normal1);
    docs.push_back(normalDoc);
    outputList.push_back(normalOutput);

    // Position stream test
    mx::DocumentPtr positionDoc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, positionDoc);
    mx::NodeGraphPtr positionNodeGraph = positionDoc->addNodeGraph("position_nodegraph");
    mx::NodePtr position1 = positionNodeGraph->addNode("position", "position1", "vector3");
    position1->setParameterValue("space", std::string("world"));
    mx::OutputPtr positionOutput = positionNodeGraph->addOutput("position_output", "vector3");
    positionOutput->setConnectedNode(position1);
    docs.push_back(positionDoc);
    outputList.push_back(positionOutput);

    // Color stream test
    mx::DocumentPtr colorDoc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, colorDoc);
    mx::NodeGraphPtr colorNodeGraph = colorDoc->addNodeGraph("color_nodegraph");
    mx::NodePtr geomcolor1 = colorNodeGraph->addNode("geomcolor", "geomcolor_set0", "color3");
    geomcolor1->setParameterValue("index", 0, "integer");
    mx::OutputPtr geomcolor1Output = colorNodeGraph->addOutput("geomcolor1_output", "vector3");
    geomcolor1Output->setConnectedNode(geomcolor1);
    docs.push_back(colorDoc);
    outputList.push_back(geomcolor1Output);

    // Second color stream test
    mx::DocumentPtr color2Doc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, color2Doc);
    mx::NodeGraphPtr color2NodeGraph = color2Doc->addNodeGraph("color2_nodegraph");
    mx::NodePtr geomcolor2 = color2NodeGraph->addNode("geomcolor", "geomcolor_set1", "color3");
    geomcolor2->setParameterValue("index", 1, "integer");
    mx::OutputPtr geomcolor2Output = color2NodeGraph->addOutput("geomcolor2_output", "vector3");
    geomcolor2Output->setConnectedNode(geomcolor2);
    docs.push_back(color2Doc);
    outputList.push_back(geomcolor2Output);

    // Tangent stream test.
    mx::DocumentPtr tangentDoc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, tangentDoc);
    mx::NodeGraphPtr tangentNodeGraph = tangentDoc->addNodeGraph("tangent_nodegraph");
    mx::NodePtr tangent1 = tangentNodeGraph->addNode("tangent", "tangent1", "vector3");
    tangent1->setParameterValue("index", 0, "integer");
    mx::OutputPtr tangent1Output = tangentNodeGraph->addOutput("tangent1_output", "vector3");
    tangent1Output->setConnectedNode(tangent1);
    docs.push_back(tangentDoc);
    outputList.push_back(tangent1Output);

    // Bitangent stream test
    mx::DocumentPtr bitangentDoc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, bitangentDoc);
    mx::NodeGraphPtr bitangentNodeGraph = bitangentDoc->addNodeGraph("bitangent_nodegraph");
    mx::NodePtr bitangent1 = bitangentNodeGraph->addNode("bitangent", "bitangent1", "vector3");
    bitangent1->setParameterValue("index", 0, "integer");
    mx::OutputPtr bitangent1Output = bitangentNodeGraph->addOutput("bitangent1_output", "vector3");
    bitangent1Output->setConnectedNode(bitangent1);
    docs.push_back(bitangentDoc);
    outputList.push_back(bitangent1Output);

    // UV stream test
    mx::DocumentPtr texcoordDoc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, texcoordDoc);
    mx::NodeGraphPtr texcoordNodeGraph = texcoordDoc->addNodeGraph("texcoord_nodegraph");
    mx::NodePtr texcoord1 = texcoordNodeGraph->addNode("texcoord", "texcoord1", "vector2");
    texcoord1->setParameterValue("index", 0, "integer");
    mx::NodePtr swizzle1 = texcoordNodeGraph->addNode("swizzle", "uv_set0", "vector3");
    swizzle1->setConnectedNode("in", texcoord1);
    swizzle1->setParameterValue("channels", std::string("xy0"));
    mx::OutputPtr texcoord1Output = texcoordNodeGraph->addOutput("texcoord1_output", "vector3");
    texcoord1Output->setConnectedNode(swizzle1);
    docs.push_back(texcoordDoc);
    outputList.push_back(texcoord1Output);

    // Second UV stream test
    mx::DocumentPtr texcoord2Doc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, texcoord2Doc);
    mx::NodeGraphPtr texcoord2NodeGraph = texcoord2Doc->addNodeGraph("texcoord2_nodegraph");
    mx::NodePtr texcoord2 = texcoord2NodeGraph->addNode("texcoord", "texcoord2", "vector2");
    texcoord2->setParameterValue("index", 1, "integer");
    mx::NodePtr swizzle2 = texcoord2NodeGraph->addNode("swizzle", "uv_set1", "vector3");
    swizzle2->setConnectedNode("in", texcoord2);
    swizzle2->setParameterValue("channels", std::string("xy0"));
    mx::OutputPtr texcoord2Output = texcoord2NodeGraph->addOutput("texcoord2_output", "vector3");
    texcoord2Output->setConnectedNode(swizzle2);
    docs.push_back(texcoord2Doc);
    outputList.push_back(texcoord2Output);

    // Image test
    mx::DocumentPtr imageDoc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, imageDoc);
    mx::NodeGraphPtr imageNodeGraph = imageDoc->addNodeGraph("image_nodegraph");
    mx::FilePath imagePath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Images/MaterialXLogo.exr");
    std::string imageName = imagePath.asString();
    mx::NodePtr image1 = imageNodeGraph->addNode("image", "image1", "color3");
    image1->setParameterValue("file", imageName, "filename");
    mx::OutputPtr imageOutput = imageNodeGraph->addOutput("image1_output", "vector3");
    imageOutput->setConnectedNode(image1);
    docs.push_back(imageDoc);
    outputList.push_back(imageOutput);

    // Create a validator
    bool orthographicView = true;
    mx::GlslValidatorPtr validator = createValidator(orthographicView, "shaderball.obj", log);

    // Set up shader generator
    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Run validation
    runValidation("geometry_attributes", outputList, validator, shaderGenerator, orthographicView, docs, log);
}

TEST_CASE("GLSL math", "[shadervalid]")
{
  #ifdef LOG_TO_FILE
      std::ofstream logfile("log_shadervalid_glsl_geometry.txt");
      std::ostream& log(logfile);
  #else
      std::ostream& log(std::cout);
  #endif

      mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

      // Stores our documents
      std::vector<mx::DocumentPtr> docs;

      // Store graph outputs that test some math operator nodes
      std::vector<mx::ElementPtr> outputList;

      // sqrt (float)
      mx::DocumentPtr sqrtDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, sqrtDoc);
      mx::NodeGraphPtr sqrtNodeGraph = sqrtDoc->addNodeGraph("sqrt_nodegraph");
      mx::NodePtr sqrtConstant1 = sqrtNodeGraph->addNode("constant", "sqrtConstant1", "float");
      sqrtConstant1->setParameterValue("value", 0.25);
      mx::NodePtr sqrt1 = sqrtNodeGraph->addNode("sqrt", "sqrt1", "float");
      sqrt1->setConnectedNode("in", sqrtConstant1);
      mx::OutputPtr sqrtOutput = sqrtNodeGraph->addOutput("sqrt_output", "float");
      sqrtOutput->setConnectedNode(sqrt1);
      docs.push_back(sqrtDoc);
      outputList.push_back(sqrtOutput);

      // ln (float)
      mx::DocumentPtr lnDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, lnDoc);
      mx::NodeGraphPtr lnNodeGraph = lnDoc->addNodeGraph("ln_nodegraph");
      mx::NodePtr lnConstant1 = lnNodeGraph->addNode("constant", "lnConstant1", "float");
      lnConstant1->setParameterValue("value", 1.0);
      mx::NodePtr ln1 = lnNodeGraph->addNode("ln", "ln1", "float");
      ln1->setConnectedNode("in", lnConstant1);
      mx::OutputPtr lnOutput = lnNodeGraph->addOutput("ln_output", "float");
      lnOutput->setConnectedNode(ln1);
      docs.push_back(lnDoc);
      outputList.push_back(lnOutput);

      // exp (float)
      mx::DocumentPtr expDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, expDoc);
      mx::NodeGraphPtr expNodeGraph = expDoc->addNodeGraph("exp_nodegraph");
      mx::NodePtr expConstant1 = expNodeGraph->addNode("constant", "expConstant1", "float");
      expConstant1->setParameterValue("value", -0.5);
      mx::NodePtr exp1 = expNodeGraph->addNode("exp", "exp1", "float");
      exp1->setConnectedNode("in", expConstant1);
      mx::OutputPtr expOutput = expNodeGraph->addOutput("exp_output", "float");
      expOutput->setConnectedNode(exp1);
      docs.push_back(expDoc);
      outputList.push_back(expOutput);

      // sign (float)
      mx::DocumentPtr signFloatDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, signFloatDoc);
      mx::NodeGraphPtr signFloatNodeGraph = signFloatDoc->addNodeGraph("sign_float_nodegraph");
      mx::NodePtr signFloatConstant1 = signFloatNodeGraph->addNode("constant", "signFloatConstant1", "float");
      signFloatConstant1->setParameterValue("value", -0.5);
      mx::NodePtr signFloat1 = signFloatNodeGraph->addNode("sign", "signFloat1", "float");
      signFloat1->setConnectedNode("in", signFloatConstant1);
      mx::OutputPtr signFloatOutput = signFloatNodeGraph->addOutput("sign_float_output", "float");
      signFloatOutput->setConnectedNode(signFloat1);
      docs.push_back(signFloatDoc);
      outputList.push_back(signFloatOutput);

      // sign (color2)
      mx::DocumentPtr signColor2Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, signColor2Doc);
      mx::NodeGraphPtr signColor2NodeGraph = signColor2Doc->addNodeGraph("sign_color2_nodegraph");
      mx::NodePtr signColor2Constant1 = signColor2NodeGraph->addNode("constant", "signColor2Constant1", "color2");
      mx::Color2 signColor2Color(0.25, -0.5);
      signColor2Constant1->setParameterValue("value", signColor2Color);
      mx::NodePtr signColor2_1 = signColor2NodeGraph->addNode("sign", "signColor2_1", "color2");
      signColor2_1->setConnectedNode("in", signColor2Constant1);
      mx::OutputPtr signColor2Output = signColor2NodeGraph->addOutput("sign_color2_output", "color2");
      signColor2Output->setConnectedNode(signColor2_1);
      docs.push_back(signColor2Doc);
      outputList.push_back(signColor2Output);

      // sign (color3)
      mx::DocumentPtr signColor3Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, signColor3Doc);
      mx::NodeGraphPtr signColor3NodeGraph = signColor3Doc->addNodeGraph("sign_color3_nodegraph");
      mx::NodePtr signColor3Constant1 = signColor3NodeGraph->addNode("constant", "signColor3Constant1", "color3");
      mx::Color3 signColor3Color(0.25, -0.5, -0.75);
      signColor3Constant1->setParameterValue("value", signColor3Color);
      mx::NodePtr signColor3_1 = signColor3NodeGraph->addNode("sign", "signColor3_1", "color3");
      signColor3_1->setConnectedNode("in", signColor3Constant1);
      mx::OutputPtr signColor3Output = signColor3NodeGraph->addOutput("sign_color3_output", "color3");
      signColor3Output->setConnectedNode(signColor3_1);
      docs.push_back(signColor3Doc);
      outputList.push_back(signColor3Output);

      // sign (color4)
      mx::DocumentPtr signColor4Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, signColor4Doc);
      mx::NodeGraphPtr signColor4NodeGraph = signColor4Doc->addNodeGraph("sign_color4_nodegraph");
      mx::NodePtr signColor4Constant1 = signColor4NodeGraph->addNode("constant", "signColor4Constant1", "color4");
      mx::Color4 signColor4Color(0.25, -0.5, -0.75, 1.0);
      signColor4Constant1->setParameterValue("value", signColor4Color);
      mx::NodePtr signColor4_1 = signColor4NodeGraph->addNode("sign", "signColor4_1", "color4");
      signColor4_1->setConnectedNode("in", signColor4Constant1);
      mx::OutputPtr signColor4Output = signColor4NodeGraph->addOutput("sign_color4_output", "color4");
      signColor4Output->setConnectedNode(signColor4_1);
      docs.push_back(signColor4Doc);
      outputList.push_back(signColor4Output);

      // sign (vector2)
      mx::DocumentPtr signVector2Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, signVector2Doc);
      mx::NodeGraphPtr signVector2NodeGraph = signVector2Doc->addNodeGraph("sign_vector2_nodegraph");
      mx::NodePtr signVector2Constant1 = signVector2NodeGraph->addNode("constant", "signVector2Constant1", "vector2");
      mx::Vector2 signVector2Vector(0.25, -0.5);
      signVector2Constant1->setParameterValue("value", signVector2Vector);
      mx::NodePtr signVector2_1 = signVector2NodeGraph->addNode("sign", "signVector2_1", "vector2");
      signVector2_1->setConnectedNode("in", signVector2Constant1);
      mx::OutputPtr signVector2Output = signVector2NodeGraph->addOutput("sign_vector2_output", "vector2");
      signVector2Output->setConnectedNode(signVector2_1);
      docs.push_back(signVector2Doc);
      outputList.push_back(signVector2Output);

      // sign (vector3)
      mx::DocumentPtr signVector3Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, signVector3Doc);
      mx::NodeGraphPtr signVector3NodeGraph = signVector3Doc->addNodeGraph("sign_vector3_nodegraph");
      mx::NodePtr signVector3Constant1 = signVector3NodeGraph->addNode("constant", "signVector3Constant1", "vector3");
      mx::Vector3 signVector3Vector(0.25, -0.5, -0.75);
      signVector3Constant1->setParameterValue("value", signVector3Vector);
      mx::NodePtr signVector3_1 = signVector3NodeGraph->addNode("sign", "signVector3_1", "vector3");
      signVector3_1->setConnectedNode("in", signVector3Constant1);
      mx::OutputPtr signVector3Output = signVector3NodeGraph->addOutput("sign_vector3_output", "vector3");
      signVector3Output->setConnectedNode(signVector3_1);
      docs.push_back(signVector3Doc);
      outputList.push_back(signVector3Output);

      // sign (vector4)
      mx::DocumentPtr signVector4Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, signVector4Doc);
      mx::NodeGraphPtr signVector4NodeGraph = signVector4Doc->addNodeGraph("sign_vector4_nodegraph");
      mx::NodePtr signVector4Constant1 = signVector4NodeGraph->addNode("constant", "signVector4Constant1", "vector4");
      mx::Vector4 signVector4Vector(0.25, -0.5, -0.75, 1.0);
      signVector4Constant1->setParameterValue("value", signVector4Vector);
      mx::NodePtr signVector4_1 = signVector4NodeGraph->addNode("sign", "signVector4_1", "vector4");
      signVector4_1->setConnectedNode("in", signVector4Constant1);
      mx::OutputPtr signVector4Output = signVector4NodeGraph->addOutput("sign_vector4_output", "vector4");
      signVector4Output->setConnectedNode(signVector4_1);
      docs.push_back(signVector4Doc);
      outputList.push_back(signVector4Output);

      // floor (float)
      mx::DocumentPtr floorFloatDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, floorFloatDoc);
      mx::NodeGraphPtr floorFloatNodeGraph = floorFloatDoc->addNodeGraph("floor_float_nodegraph");
      mx::NodePtr floorFloatConstant1 = floorFloatNodeGraph->addNode("constant", "floorFloatConstant1", "float");
      floorFloatConstant1->setParameterValue("value", 0.5);
      mx::NodePtr floorFloat1 = floorFloatNodeGraph->addNode("floor", "floorFloat1", "float");
      floorFloat1->setConnectedNode("in", floorFloatConstant1);
      mx::OutputPtr floorFloatOutput = floorFloatNodeGraph->addOutput("floor_float_output", "float");
      floorFloatOutput->setConnectedNode(floorFloat1);
      docs.push_back(floorFloatDoc);
      outputList.push_back(floorFloatOutput);

      // floor (color2)
      mx::DocumentPtr floorColor2Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, floorColor2Doc);
      mx::NodeGraphPtr floorColor2NodeGraph = floorColor2Doc->addNodeGraph("floor_color2_nodegraph");
      mx::NodePtr floorColor2Constant1 = floorColor2NodeGraph->addNode("constant", "floorColor2Constant1", "color2");
      mx::Color2 floorColor2Color(0.5, 1.5);
      floorColor2Constant1->setParameterValue("value", floorColor2Color);
      mx::NodePtr floorColor2_1 = floorColor2NodeGraph->addNode("floor", "floorColor2_1", "color2");
      floorColor2_1->setConnectedNode("in", floorColor2Constant1);
      mx::OutputPtr floorColor2Output = floorColor2NodeGraph->addOutput("floor_color2_output", "color2");
      floorColor2Output->setConnectedNode(floorColor2_1);
      docs.push_back(floorColor2Doc);
      outputList.push_back(floorColor2Output);

      // floor (color3)
      mx::DocumentPtr floorColor3Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, floorColor3Doc);
      mx::NodeGraphPtr floorColor3NodeGraph = floorColor3Doc->addNodeGraph("floor_color3_nodegraph");
      mx::NodePtr floorColor3Constant1 = floorColor3NodeGraph->addNode("constant", "floorColor3Constant1", "color3");
      mx::Color3 floorColor3Color(0.5, 1.5, 0.25);
      floorColor3Constant1->setParameterValue("value", floorColor3Color);
      mx::NodePtr floorColor3_1 = floorColor3NodeGraph->addNode("floor", "floorColor3_1", "color3");
      floorColor3_1->setConnectedNode("in", floorColor3Constant1);
      mx::OutputPtr floorColor3Output = floorColor3NodeGraph->addOutput("floor_color3_output", "color3");
      floorColor3Output->setConnectedNode(floorColor3_1);
      docs.push_back(floorColor3Doc);
      outputList.push_back(floorColor3Output);

      // floor (color4)
      mx::DocumentPtr floorColor4Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, floorColor4Doc);
      mx::NodeGraphPtr floorColor4NodeGraph = floorColor4Doc->addNodeGraph("floor_color4_nodegraph");
      mx::NodePtr floorColor4Constant1 = floorColor4NodeGraph->addNode("constant", "floorColor4Constant1", "color4");
      mx::Color4 floorColor4Color(0.5, 1.5, 0.25, 1.25);
      floorColor4Constant1->setParameterValue("value", floorColor4Color);
      mx::NodePtr floorColor4_1 = floorColor4NodeGraph->addNode("floor", "floorColor4_1", "color4");
      floorColor4_1->setConnectedNode("in", floorColor4Constant1);
      mx::OutputPtr floorColor4Output = floorColor4NodeGraph->addOutput("floor_color4_output", "color4");
      floorColor4Output->setConnectedNode(floorColor4_1);
      docs.push_back(floorColor4Doc);
      outputList.push_back(floorColor4Output);

      // floor (vector2)
      mx::DocumentPtr floorVector2Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, floorVector2Doc);
      mx::NodeGraphPtr floorVector2NodeGraph = floorVector2Doc->addNodeGraph("floor_vector2_nodegraph");
      mx::NodePtr floorVector2Constant1 = floorVector2NodeGraph->addNode("constant", "floorVector2Constant1", "vector2");
      mx::Vector2 floorVector2Vector(0.5, 1.5);
      floorVector2Constant1->setParameterValue("value", floorVector2Vector);
      mx::NodePtr floorVector2_1 = floorVector2NodeGraph->addNode("floor", "floorVector2_1", "vector2");
      floorVector2_1->setConnectedNode("in", floorVector2Constant1);
      mx::OutputPtr floorVector2Output = floorVector2NodeGraph->addOutput("floor_vector2_output", "vector2");
      floorVector2Output->setConnectedNode(floorVector2_1);
      docs.push_back(floorVector2Doc);
      outputList.push_back(floorVector2Output);

      // floor (vector3)
      mx::DocumentPtr floorVector3Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, floorVector3Doc);
      mx::NodeGraphPtr floorVector3NodeGraph = floorVector3Doc->addNodeGraph("floor_vector3_nodegraph");
      mx::NodePtr floorVector3Constant1 = floorVector3NodeGraph->addNode("constant", "floorVector3Constant1", "vector3");
      mx::Vector3 floorVector3Vector(0.5, 1.5, 0.25);
      floorVector3Constant1->setParameterValue("value", floorVector3Vector);
      mx::NodePtr floorVector3_1 = floorVector3NodeGraph->addNode("floor", "floorVector3_1", "vector3");
      floorVector3_1->setConnectedNode("in", floorVector3Constant1);
      mx::OutputPtr floorVector3Output = floorVector3NodeGraph->addOutput("floor_vector3_output", "vector3");
      floorVector3Output->setConnectedNode(floorVector3_1);
      docs.push_back(floorVector3Doc);
      outputList.push_back(floorVector3Output);

      // floor (vector4)
      mx::DocumentPtr floorVector4Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, floorVector4Doc);
      mx::NodeGraphPtr floorVector4NodeGraph = floorVector4Doc->addNodeGraph("floor_vector4_nodegraph");
      mx::NodePtr floorVector4Constant1 = floorVector4NodeGraph->addNode("constant", "floorVector4Constant1", "vector4");
      mx::Vector4 floorVector4Vector(0.5, 1.5, 0.25, 1.25);
      floorVector4Constant1->setParameterValue("value", floorVector4Vector);
      mx::NodePtr floorVector4_1 = floorVector4NodeGraph->addNode("floor", "floorVector4_1", "vector4");
      floorVector4_1->setConnectedNode("in", floorVector4Constant1);
      mx::OutputPtr floorVector4Output = floorVector4NodeGraph->addOutput("floor_vector4_output", "vector4");
      floorVector4Output->setConnectedNode(floorVector4_1);
      docs.push_back(floorVector4Doc);
      outputList.push_back(floorVector4Output);

      // ceil (float)
      mx::DocumentPtr ceilFloatDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, ceilFloatDoc);
      mx::NodeGraphPtr ceilFloatNodeGraph = ceilFloatDoc->addNodeGraph("ceil_float_nodegraph");
      mx::NodePtr ceilFloatConstant1 = ceilFloatNodeGraph->addNode("constant", "ceilFloatConstant1", "float");
      ceilFloatConstant1->setParameterValue("value", -0.5);
      mx::NodePtr ceilFloat1 = ceilFloatNodeGraph->addNode("ceil", "ceilFloat1", "float");
      ceilFloat1->setConnectedNode("in", ceilFloatConstant1);
      mx::OutputPtr ceilFloatOutput = ceilFloatNodeGraph->addOutput("ceil_float_output", "float");
      ceilFloatOutput->setConnectedNode(ceilFloat1);
      docs.push_back(ceilFloatDoc);
      outputList.push_back(ceilFloatOutput);

      // ceil (color2)
      mx::DocumentPtr ceilColor2Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, ceilColor2Doc);
      mx::NodeGraphPtr ceilColor2NodeGraph = ceilColor2Doc->addNodeGraph("ceil_color2_nodegraph");
      mx::NodePtr ceilColor2Constant1 = ceilColor2NodeGraph->addNode("constant", "ceilColor2Constant1", "color2");
      mx::Color2 ceilColor2Color(0.0, 0.5);
      ceilColor2Constant1->setParameterValue("value", ceilColor2Color);
      mx::NodePtr ceilColor2_1 = ceilColor2NodeGraph->addNode("ceil", "ceilColor2_1", "color2");
      ceilColor2_1->setConnectedNode("in", ceilColor2Constant1);
      mx::OutputPtr ceilColor2Output = ceilColor2NodeGraph->addOutput("ceil_color2_output", "color2");
      ceilColor2Output->setConnectedNode(ceilColor2_1);
      docs.push_back(ceilColor2Doc);
      outputList.push_back(ceilColor2Output);

      // ceil (color3)
      mx::DocumentPtr ceilColor3Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, ceilColor3Doc);
      mx::NodeGraphPtr ceilColor3NodeGraph = ceilColor3Doc->addNodeGraph("ceil_color3_nodegraph");
      mx::NodePtr ceilColor3Constant1 = ceilColor3NodeGraph->addNode("constant", "ceilColor3Constant1", "color3");
      mx::Color3 ceilColor3Color(0.0, 0.5, 0.75);
      ceilColor3Constant1->setParameterValue("value", ceilColor3Color);
      mx::NodePtr ceilColor3_1 = ceilColor3NodeGraph->addNode("ceil", "ceilColor3_1", "color3");
      ceilColor3_1->setConnectedNode("in", ceilColor3Constant1);
      mx::OutputPtr ceilColor3Output = ceilColor3NodeGraph->addOutput("ceil_color3_output", "color3");
      ceilColor3Output->setConnectedNode(ceilColor3_1);
      docs.push_back(ceilColor3Doc);
      outputList.push_back(ceilColor3Output);

      // ceil (color4)
      mx::DocumentPtr ceilColor4Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, ceilColor4Doc);
      mx::NodeGraphPtr ceilColor4NodeGraph = ceilColor4Doc->addNodeGraph("ceil_color4_nodegraph");
      mx::NodePtr ceilColor4Constant1 = ceilColor4NodeGraph->addNode("constant", "ceilColor4Constant1", "color4");
      mx::Color4 ceilColor4Color(0.0, 0.5, 0.75, 1.0);
      ceilColor4Constant1->setParameterValue("value", ceilColor4Color);
      mx::NodePtr ceilColor4_1 = ceilColor4NodeGraph->addNode("ceil", "ceilColor4_1", "color4");
      ceilColor4_1->setConnectedNode("in", ceilColor4Constant1);
      mx::OutputPtr ceilColor4Output = ceilColor4NodeGraph->addOutput("ceil_color4_output", "color4");
      ceilColor4Output->setConnectedNode(ceilColor4_1);
      docs.push_back(ceilColor4Doc);
      outputList.push_back(ceilColor4Output);

      // ceil (vector2)
      mx::DocumentPtr ceilVector2Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, ceilVector2Doc);
      mx::NodeGraphPtr ceilVector2NodeGraph = ceilVector2Doc->addNodeGraph("ceil_vector2_nodegraph");
      mx::NodePtr ceilVector2Constant1 = ceilVector2NodeGraph->addNode("constant", "ceilVector2Constant1", "vector2");
      mx::Vector2 ceilVector2Vector(0.0, 0.5);
      ceilVector2Constant1->setParameterValue("value", ceilVector2Vector);
      mx::NodePtr ceilVector2_1 = ceilVector2NodeGraph->addNode("ceil", "ceilVector2_1", "vector2");
      ceilVector2_1->setConnectedNode("in", ceilVector2Constant1);
      mx::OutputPtr ceilVector2Output = ceilVector2NodeGraph->addOutput("ceil_vector2_output", "vector2");
      ceilVector2Output->setConnectedNode(ceilVector2_1);
      docs.push_back(ceilVector2Doc);
      outputList.push_back(ceilVector2Output);

      // ceil (vector3)
      mx::DocumentPtr ceilVector3Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, ceilVector3Doc);
      mx::NodeGraphPtr ceilVector3NodeGraph = ceilVector3Doc->addNodeGraph("ceil_vector3_nodegraph");
      mx::NodePtr ceilVector3Constant1 = ceilVector3NodeGraph->addNode("constant", "ceilVector3Constant1", "vector3");
      mx::Vector3 ceilVector3Vector(0.0, 0.5, 0.75);
      ceilVector3Constant1->setParameterValue("value", ceilVector3Vector);
      mx::NodePtr ceilVector3_1 = ceilVector3NodeGraph->addNode("ceil", "ceilVector3_1", "vector3");
      ceilVector3_1->setConnectedNode("in", ceilVector3Constant1);
      mx::OutputPtr ceilVector3Output = ceilVector3NodeGraph->addOutput("ceil_vector3_output", "vector3");
      ceilVector3Output->setConnectedNode(ceilVector3_1);
      docs.push_back(ceilVector3Doc);
      outputList.push_back(ceilVector3Output);

      // ceil (vector4)
      mx::DocumentPtr ceilVector4Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, ceilVector4Doc);
      mx::NodeGraphPtr ceilVector4NodeGraph = ceilVector4Doc->addNodeGraph("ceil_vector4_nodegraph");
      mx::NodePtr ceilVector4Constant1 = ceilVector4NodeGraph->addNode("constant", "ceilVector4Constant1", "vector4");
      mx::Vector4 ceilVector4Vector(0.0, 0.5, 0.75, 1.0);
      ceilVector4Constant1->setParameterValue("value", ceilVector4Vector);
      mx::NodePtr ceilVector4_1 = ceilVector4NodeGraph->addNode("ceil", "ceilVector4_1", "vector4");
      ceilVector4_1->setConnectedNode("in", ceilVector4Constant1);
      mx::OutputPtr ceilVector4Output = ceilVector4NodeGraph->addOutput("ceil_vector4_output", "vector4");
      ceilVector4Output->setConnectedNode(ceilVector4_1);
      docs.push_back(ceilVector4Doc);
      outputList.push_back(ceilVector4Output);

      // sin (float)
      mx::DocumentPtr sinDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, sinDoc);
      mx::NodeGraphPtr sinNodeGraph = sinDoc->addNodeGraph("sin_nodegraph");
      mx::NodePtr sinConstant1 = sinNodeGraph->addNode("constant", "sinConstant1", "float");
      sinConstant1->setParameterValue("value", M_PI);
      mx::NodePtr sin1 = sinNodeGraph->addNode("sin", "sin1", "float");
      sin1->setConnectedNode("in", sinConstant1);
      mx::OutputPtr sinOutput = sinNodeGraph->addOutput("sin_output", "float");
      sinOutput->setConnectedNode(sin1);
      docs.push_back(sinDoc);
      outputList.push_back(sinOutput);

      // cos (float)
      mx::DocumentPtr cosDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, cosDoc);
      mx::NodeGraphPtr cosNodeGraph = cosDoc->addNodeGraph("cos_nodegraph");
      mx::NodePtr cosConstant1 = cosNodeGraph->addNode("constant", "cosConstant1", "float");
      cosConstant1->setParameterValue("value", M_PI);
      mx::NodePtr cos1 = cosNodeGraph->addNode("cos", "cos1", "float");
      cos1->setConnectedNode("in", cosConstant1);
      mx::OutputPtr cosOutput = cosNodeGraph->addOutput("cos_output", "float");
      cosOutput->setConnectedNode(cos1);
      docs.push_back(cosDoc);
      outputList.push_back(cosOutput);

      // tan (float)
      mx::DocumentPtr tanDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, tanDoc);
      mx::NodeGraphPtr tanNodeGraph = tanDoc->addNodeGraph("tan_nodegraph");
      mx::NodePtr tanConstant1 = tanNodeGraph->addNode("constant", "tanConstant1", "float");
      tanConstant1->setParameterValue("value", M_PI);
      mx::NodePtr tan1 = tanNodeGraph->addNode("tan", "tan1", "float");
      tan1->setConnectedNode("in", tanConstant1);
      mx::OutputPtr tanOutput = tanNodeGraph->addOutput("tan_output", "float");
      tanOutput->setConnectedNode(tan1);
      docs.push_back(tanDoc);
      outputList.push_back(tanOutput);

      // asin (float)
      mx::DocumentPtr asinDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, asinDoc);
      mx::NodeGraphPtr asinNodeGraph = asinDoc->addNodeGraph("asin_nodegraph");
      mx::NodePtr asinConstant1 = asinNodeGraph->addNode("constant", "asinConstant1", "float");
      asinConstant1->setParameterValue("value", M_PI);
      mx::NodePtr asin1 = asinNodeGraph->addNode("asin", "asin1", "float");
      asin1->setConnectedNode("in", asinConstant1);
      mx::OutputPtr asinOutput = asinNodeGraph->addOutput("asin_output", "float");
      asinOutput->setConnectedNode(asin1);
      docs.push_back(asinDoc);
      outputList.push_back(asinOutput);

      // acos (float)
      mx::DocumentPtr acosDoc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, acosDoc);
      mx::NodeGraphPtr acosNodeGraph = acosDoc->addNodeGraph("acos_nodegraph");
      mx::NodePtr acosConstant1 = acosNodeGraph->addNode("constant", "acosConstant1", "float");
      acosConstant1->setParameterValue("value", M_PI);
      mx::NodePtr acos1 = acosNodeGraph->addNode("acos", "acos1", "float");
      acos1->setConnectedNode("in", acosConstant1);
      mx::OutputPtr acosOutput = acosNodeGraph->addOutput("acos_output", "float");
      acosOutput->setConnectedNode(acos1);
      docs.push_back(acosDoc);
      outputList.push_back(acosOutput);

      // atan2 (float)
      mx::DocumentPtr atan2Doc = mx::createDocument();
      loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, atan2Doc);
      mx::NodeGraphPtr atan2NodeGraph = acosDoc->addNodeGraph("atan2_nodegraph");
      mx::NodePtr atan2Constant1 = atan2NodeGraph->addNode("constant", "atan2Constant1", "float");
      atan2Constant1->setParameterValue("value", M_PI);
      mx::NodePtr atan2Constant2 = atan2NodeGraph->addNode("constant", "atan2Constant2", "float");
      atan2Constant2->setParameterValue("value", M_PI);
      mx::NodePtr atan2_1 = atan2NodeGraph->addNode("atan2", "aton2_1", "float");
      atan2_1->setConnectedNode("in1", atan2Constant1);
      atan2_1->setConnectedNode("in2", atan2Constant2);
      mx::OutputPtr atan2Output = atan2NodeGraph->addOutput("atan2_output", "float");
      atan2Output->setConnectedNode(atan2_1);
      docs.push_back(atan2Doc);
      outputList.push_back(atan2Output);

      // Create a validator
      bool orthographicView = true;
      mx::GlslValidatorPtr validator = createValidator(orthographicView, "shaderball.obj", log);

      // Set up shader generator
      mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
      shaderGenerator->registerSourceCodeSearchPath(searchPath);

      // Run validation
      runValidation("math_operators", outputList, validator, shaderGenerator, orthographicView, docs, log);
}


TEST_CASE("GLSL shading", "[shadervalid]")
{
#ifdef LOG_TO_FILE
    std::ofstream logfile("log_shadervalid_glsl_shading.txt");
    std::ostream& log(logfile);
#else
    std::ostream& log(std::cout);
#endif
    std::vector<mx::DocumentPtr> docs;
    mx::DocumentPtr doc = mx::createDocument();
    docs.push_back(doc);

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    // Create a validator
    bool orthographicView = true;
    mx::GlslValidatorPtr validator = createValidator(orthographicView, "shaderball.obj", log);

    // Create shader generator
    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Set up lighting
    mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
    mx::HwShaderGenerator& hwGenerator = static_cast<mx::HwShaderGenerator&>(*shaderGenerator);
    createLightRig(doc, *lightHandler, hwGenerator);
    // Pre-clamp the number of light sources to the number bound
    size_t lightSourceCount = lightHandler->getLightSources().size();
    hwGenerator.setMaxActiveLightSources(lightSourceCount);

    mx::SgOptions options;

    //
    // Lighting test
    //
    {
        const std::string lightDoc = " \
        <?xml version=\"1.0\"?> \
        <materialx version=\"1.36\" require=\"\"> \
          <nodegraph name=\"lighting1\"> \
            <surface name=\"surface1\" type=\"surfaceshader\"> \
              <input name=\"bsdf\" type=\"BSDF\" value=\"\" nodename=\"diffusebsdf1\" /> \
              <input name=\"edf\" type=\"EDF\" value=\"\" /> \
              <input name=\"opacity\" type=\"float\" value=\"1.0\" /> \
            </surface>  \
            <diffusebsdf name=\"diffusebsdf1\" type=\"BSDF\"> \
              <input name=\"reflectance\" type=\"color3\" value=\"1.0, 1.0, 1.0\" />  \
              <input name=\"roughness\" type=\"float\" value=\"0.8\" /> \
              <input name=\"normal\" type=\"vector3\" /> \
            </diffusebsdf>  \
            <output name=\"lighting_output\" type=\"surfaceshader\" nodename=\"surface1\" /> \
          </nodegraph> \
        </materialx>";

        MaterialX::readFromXmlBuffer(doc, lightDoc.c_str());
        mx::NodeGraphPtr nodeGraph = doc->getNodeGraph("lighting1");
        mx::OutputPtr output = nodeGraph->getOutput("lighting_output");

        // Run validation
        std::vector<mx::ElementPtr> outputList;
        outputList.push_back(output);
        runValidation(nodeGraph->getName(), outputList, validator, shaderGenerator, orthographicView, docs, log);
    }

    //
    // Materials test
    //
    {
        std::vector<mx::MaterialPtr> materials;
        createExampleMaterials(doc, materials);

        for (const mx::MaterialPtr& material : materials)
        {
            for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                const std::string name = material->getName() + "_" + shaderRef->getName();

                std::vector<mx::ElementPtr> shaderRefList;
                shaderRefList.push_back(shaderRef);
                runValidation(name, shaderRefList, validator, shaderGenerator, orthographicView, docs, log);
            }
        }
    }


}

#endif
#endif
