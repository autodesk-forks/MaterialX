//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderVk/VkRenderer.h>
#include <MaterialXRenderHw/SimpleWindow.h>
#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXRender/CgltfLoader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>

#ifdef _USE_VULKAN_CPP
// Vulkan layer
#include <MaterialXRenderVk/VkDevice.h>
// Render module
#include <MaterialXRenderVk/VkBuffer.h>
#include <MaterialXRenderVk/VkUniformBuffer.h>
#include <MaterialXRenderVk/VkFrameBuffer.h>
#include "shaders.hpp"
#endif



MATERIALX_NAMESPACE_BEGIN

const float PI = std::acos(-1.0f);

// View information
const float FOV_PERSP = 45.0f; // degrees
const float NEAR_PLANE_PERSP = 0.05f;
const float FAR_PLANE_PERSP = 100.0f;

// GLOBALS for now

#ifdef _USE_VULKAN_CPP
    VkFrameBufferPtr _frameBuffer;
#endif
    vk::Instance _instance;
    vk::SurfaceKHR _surface;
    vk::Format _format;
    vk::ImageView _imageView;
 
    //
// VkRenderer methods
//


static std::vector<uint8_t> generateCheckeredPattern8x8()
{
    const uint32_t width = 64;  // Make texture larger for less pixelation
    const uint32_t height = 64;
    std::vector<uint8_t> pixels(width * height * 4); // RGBA8

    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            bool isWhite = (((x / 8) + (y / 8)) % 2) == 0; 

            uint8_t color = isWhite ? 255 : 0;
            size_t idx = (y * width + x) * 4;
            pixels[idx + 0] = color; // R
            pixels[idx + 1] = color; // G
            pixels[idx + 2] = color; // B
            pixels[idx + 3] = 255;   // A
        }
    }
    return pixels;
}

    VkRendererPtr VkRenderer::create(unsigned int width, unsigned int height, Image::BaseType baseType)
{
    return VkRendererPtr(new VkRenderer(width, height, baseType));
}

VkRenderer::VkRenderer(unsigned int width, unsigned int height, Image::BaseType baseType) :
    ShaderRenderer(width, height, baseType),
    _initialized(false),
    _eye(0.0f, 0.0f, 3.0f),
    _center(0.0f, 0.0f, 0.0f),
    _up(0.0f, 1.0f, 0.0f),
    _objectScale(1.0f),
    _screenColor(DEFAULT_SCREEN_COLOR_LIN_REC709)
{
    //_program = VkProgram::create();

    _geometryHandler = GeometryHandler::create();
    _geometryHandler->addLoader(TinyObjLoader::create());
    _geometryHandler->addLoader(CgltfLoader::create());
    _camera = Camera::create();
}

void VkRenderer::initialize(RenderContextHandle)
{
    if (!_initialized)
    {
        // Create window
        _window = SimpleWindow::create();

        if (!_window->initialize("Renderer Window", _width, _height, nullptr))
        {
            throw ExceptionRenderError("Failed to initialize renderer window");
        }
        
        _surfaceExtent = vk::Extent2D(_width, _height);
     
        // Create and initialize Device. 
        // Create a presentable swapchain for testing 
        #ifdef _USE_VULKAN_CPP
        _vkDevice = VulkanDevice::create();



        // Create Surface
        HINSTANCE hInstance = GetModuleHandle(NULL);
        auto const createInfo = vk::Win32SurfaceCreateInfoKHR()
                                    .setHinstance(hInstance)
                                    .setHwnd(_window->getWindowWrapper()->externalHandle());
        _surface = vk::SurfaceKHR();
        _instance = _vkDevice->GetInstance();
        _format = vk::Format();

        vk::Result sresult = _instance.createWin32SurfaceKHR(&createInfo, nullptr, &_surface);
        assert(sresult == vk::Result::eSuccess);

        _vkDevice->InitializeDevice(_surface, _width, _height);
        _vkDevice->CreateCommandPool();
        #endif
        
        createFrameBuffer(true);

        /// Create DepthBuffer
        
         //Create a default program
         //createProgram(nullptr);
        _geometryHandler->loadGeometry("F:/source/MaterialX/resources/Geometry/teapot.obj");

        _initialized = true;
        auto checkerPixels = generateCheckeredPattern8x8();
        VkTexture::ImageDesc desc = {};

        desc.width = 64;
        desc.height = 64;
        desc.channelCount = 4; // RGBA
        desc.mipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(desc.width, desc.height)))) + 1; ///Fixed!
        desc.rowStride = desc.width * desc.channelCount; 
        desc.elementSize = 1;                          
        desc.isFloat = false;
        desc.isHalf = false;

       _checkeredTexture = VkTexture::create(_vkDevice);

        bool success = _checkeredTexture->initialize(
            desc,               
            checkerPixels.data(),
            false,                
            false                 
        );

        
        if (!success)
        {
            throw std::runtime_error("Failed to create checkerboard texture");
        }

    }
 }

void VkRenderer::createProgram(ShaderPtr shader)
{

    /*_program = VkProgram::create();
    _program->setStages(shader);
     _program->build(_vkDevice, _framebuffer);*/

}

void VkRenderer::createProgram(const StageMap& stages)
{
    //TODO: VULKAN
    /* 
    for (const auto& it : stages)
    {
        _program->addStage(it.first, it.second);
    }
    _program->build(_device, _framebuffer);
    */
}


void VkRenderer::validateInputs()
{
   #if 0 
    // Check that the generated uniforms and attributes are valid
    _program->getUniformsList();
    _program->getAttributesList();
   #endif
}

void VkRenderer::createFrameBuffer(bool /*encodeSrgb*/)
{
    /// Add two image types, colorBuffer and depthBuffer
    _frameBuffer = VkFrameBuffer::create(_vkDevice, _width, _height, 4,
                                         _baseType);
}



void VkRenderer::setSize(unsigned int width, unsigned int height)
{

    if (_frameBuffer)
    {
        _frameBuffer->resize(width, height);
    }
    else
    {
        _width = width;
        _height = height;
        createFrameBuffer(true);
    }
}

void VkRenderer::updateViewInformation()
{
    float fH = std::tan(FOV_PERSP / 360.0f * PI) * NEAR_PLANE_PERSP;
    float fW = fH * static_cast<float>(_width) / static_cast<float>(_height);

    _camera->setViewMatrix(Camera::createViewMatrix(_eye, _center, _up));
    _camera->setProjectionMatrix(Camera::createPerspectiveMatrixZP(-fW, fW, -fH, fH, NEAR_PLANE_PERSP, FAR_PLANE_PERSP));
}

void VkRenderer::updateWorldInformation()
{
    _camera->setWorldMatrix(Matrix44::createScale(Vector3(_objectScale)));
}

VkRenderer::FrameData VkRenderer::acquireFrame(const vk::RenderPass& renderPass,
                                               const std::vector<vk::Framebuffer>& framebuffers)
{
    FrameData frame;
    VkRenderContext vkContext(_vkDevice);

    frame.imageAcquiredSemaphore = vkContext.device.createSemaphore({});
    frame.drawFence = vkContext.device.createFence({});

    auto acquired = vkContext.device.acquireNextImageKHR(vkContext.swapchain._swapChain,
                                                   100000000, frame.imageAcquiredSemaphore, nullptr);

    if (acquired.result != vk::Result::eSuccess || acquired.value >= framebuffers.size())
    {
        throw std::runtime_error("Failed to acquire image for rendering.");
    }

    frame.imageIndex = acquired.value;
    frame.commandBuffer = _vkDevice->getCommandBuffer();

    return frame;
}

void VkRenderer::submitAndPresent(FrameData& frame, vk::RenderPass renderPass)
{
    VkRenderContext vkContext(_vkDevice);
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo(frame.imageAcquiredSemaphore, waitStage, frame.commandBuffer);

    vkContext.graphicsQueue.submit(submitInfo, frame.drawFence);


    while (vkContext.device.waitForFences(frame.drawFence, VK_TRUE, 100000000) == vk::Result::eTimeout)
        ;

    vk::Result result = vkContext.presentQueue.presentKHR(vk::PresentInfoKHR({}, vkContext.swapchain._swapChain, frame.imageIndex));
    if (result == vk::Result::eSuboptimalKHR)
    {
        std::cout << "Warning: Suboptimal presentation." << std::endl;
    }
    else
    {
        assert(result == vk::Result::eSuccess);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    vkContext.device.waitIdle();
    vkContext.device.destroyFence(frame.drawFence);
    vkContext.device.destroySemaphore(frame.imageAcquiredSemaphore);
    vkContext.device.destroyRenderPass(renderPass);
}

void VkRenderer::recordDrawCommands(vk::CommandBuffer& cmd,
                                    const vk::RenderPass& renderPass,
                                    const vk::Framebuffer& framebuffer)
{
    cmd.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

    std::array<vk::ClearValue, 2> clear{
        vk::ClearColorValue{ 0.6f, 0.6f, 0.6f, 0.0f },
        vk::ClearDepthStencilValue{ 1.0f, 0 }
    };

    cmd.beginRenderPass({ renderPass, framebuffer, vk::Rect2D{ { 0, 0 }, _surfaceExtent }, clear }, vk::SubpassContents::eInline);

    cmd.setViewport(0, vk::Viewport{ 0, static_cast<float>(_surfaceExtent.height),
                                     static_cast<float>(_surfaceExtent.width), -static_cast<float>(_surfaceExtent.height), 0, 1 });
    cmd.setScissor(0, vk::Rect2D{ { 0, 0 }, _surfaceExtent });

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, _program->getPipeline());

    for (const auto& meshBuffer : _meshBuffers)
    {
        _program->bindViewInformation(_camera);       

        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               _program->getPipelineLayout(), 0,
                               { _program->getDescriptorSet() }, {});

        vk::DeviceSize offset = 0;
        cmd.bindVertexBuffers(0, meshBuffer.vertexBuffer->getBuffer(), offset);
        cmd.bindIndexBuffer(meshBuffer.indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
        cmd.drawIndexed(meshBuffer.indexCount, 1, 0, 0, 0);
    }

    cmd.endRenderPass();
    cmd.end();
}

void VkRenderer::prepareMeshBuffers()
{
    _meshBuffers.clear();

    for (const auto& mesh : _geometryHandler->getMeshes())
    {
        MeshStreamPtr posStream = mesh->getStream(HW::IN_POSITION);
        MeshStreamPtr normStream = mesh->getStream(HW::IN_NORMAL);

        const auto& vertexData = posStream->getData();
        const auto& normalData = normStream->getData();
        size_t vertexCount = vertexData.size() / 3;

        MeshStreamPtr texCoordStream = mesh->getStream(HW::IN_TEXCOORD + "_0");
        std::vector<float> texCoordData;
        
        if (texCoordStream) 
        {
            texCoordData = texCoordStream->getData();
            std::cout << "Mesh " << mesh->getName() << " has texture coordinates, count: " << texCoordData.size() << std::endl;
            
            // Fix invalid UV coordinates that are outside 0-1 range, this is to solve the problem for the cube which is not in 0-1 range. Still not working correctly
            for (size_t i = 0; i < texCoordData.size(); i += 2) {
                texCoordData[i] = std::fmod(texCoordData[i], 1.0f);         // U coordinate
                texCoordData[i + 1] = std::fmod(texCoordData[i + 1], 1.0f); // V coordinate
                
                if (texCoordData[i] < 0.0f) texCoordData[i] += 1.0f;
                if (texCoordData[i + 1] < 0.0f) texCoordData[i + 1] += 1.0f;
            }
            std::cout << "Fixed UV coordinates to 0-1 range" << std::endl;
        }
        else 
        {
            texCoordData.resize(vertexCount * 2);
            
            float minX = vertexData[0], maxX = vertexData[0];
            float minY = vertexData[1], maxY = vertexData[1]; 
            float minZ = vertexData[2], maxZ = vertexData[2];
            
            for (size_t i = 1; i < vertexCount; ++i) 
            {
                minX = std::min(minX, vertexData[i * 3]);
                maxX = std::max(maxX, vertexData[i * 3]);
                minY = std::min(minY, vertexData[i * 3 + 1]);
                maxY = std::max(maxY, vertexData[i * 3 + 1]);
                minZ = std::min(minZ, vertexData[i * 3 + 2]);
                maxZ = std::max(maxZ, vertexData[i * 3 + 2]);
            }
            
            Vector3 center((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f);
            
            for (size_t i = 0; i < vertexCount; ++i) 
            {
                Vector3 pos(vertexData[i * 3], vertexData[i * 3 + 1], vertexData[i * 3 + 2]);
                Vector3 dir = (pos - center).getNormalized();
                
               
                float u = 0.5f + std::atan2(dir[2], dir[0]) / (2.0f * PI);
                float v = 0.5f + std::asin(dir[1]) / PI;
                
             
                u = std::max(0.0f, std::min(1.0f, u));
                v = std::max(0.0f, std::min(1.0f, v));
                
                texCoordData[i * 2] = u;
                texCoordData[i * 2 + 1] = v;
            }
            std::cout << "Mesh " << mesh->getName() << " missing texture coordinates, generated spherical mapping" << std::endl;
        }

        std::vector<float> interleavedData;
        interleavedData.reserve(vertexCount * 8); // 3 pos + 3 normal + 2 texcoord
        for (size_t i = 0; i < vertexCount; ++i)
        {
            interleavedData.insert(interleavedData.end(), { vertexData[i * 3], vertexData[i * 3 + 1], vertexData[i * 3 + 2],
                                                            normalData[i * 3], normalData[i * 3 + 1], normalData[i * 3 + 2],
                                                            texCoordData[i * 2], texCoordData[i * 2 + 1] });
        }
        
        // Debug UV's
        if (vertexCount > 0) {
            std::cout << "First 3 UV coordinates for mesh " << mesh->getName() << " (corrected):" << std::endl;
            for (size_t i = 0; i < std::min(vertexCount, (size_t)3); ++i) {
                std::cout << "  UV[" << i << "]: (" << texCoordData[i * 2] << ", " << texCoordData[i * 2 + 1] << ")" << std::endl;
            }
        }

        auto vertexBuffer = VkBuffer::create(_vkDevice);
        vertexBuffer->CreateVertexBuffer(interleavedData.data(),
                                         interleavedData.size() * sizeof(float),
                                         sizeof(float) * 8);

        std::vector<uint32_t> indices;
        for (size_t i = 0; i < mesh->getPartitionCount(); ++i)
        {
            const auto& partition = mesh->getPartition(i);
            const auto& indexBuffer = partition->getIndices();
            indices.insert(indices.end(), indexBuffer.begin(), indexBuffer.end());
        }

        auto indexBuffer = VkBuffer::create(_vkDevice);
        indexBuffer->CreateIndexBuffer(indices.data(),
                                       indices.size() * sizeof(uint32_t),
                                       sizeof(uint32_t));


        _meshBuffers.push_back({ vertexBuffer, indexBuffer, static_cast<uint32_t>(indices.size()), mesh, nullptr, {} });
    }
}


void VkRenderer::drawMesh()
{
    const auto& swapData = _vkDevice->getSwapChainData();

    _program = VkProgram::create(_vkDevice);
    auto renderPass = _vkDevice->createRenderPass(swapData._colorFormat, vk::Format::eD16Unorm);

    _program->build(vertexShaderText_PC_M, fragmentShaderText_C_M, renderPass, true, vk::FrontFace::eCounterClockwise);
    _program->updateTextureDescriptor(_checkeredTexture->getImageView(), _checkeredTexture->getSampler());

    prepareMeshBuffers();

    CameraPtr camera = _camera;
    if (!camera)
    {
        throw ExceptionRenderError("Camera is not initialized");
    }

    // Update camera matrices
    updateViewInformation();
    updateWorldInformation();

    auto framebuffers = _vkDevice->createFramebuffers(
        renderPass, swapData._imageViews, _frameBuffer->getDepthBufferImageView(), _surfaceExtent);

    auto frame = acquireFrame(renderPass, framebuffers);
    recordDrawCommands(frame.commandBuffer, renderPass, framebuffers[frame.imageIndex]);
    submitAndPresent(frame, renderPass);
}



void VkRenderer::render()
{
    /// Simple clear screen

    //drawCube();
    drawMesh();

}
//
// void VkRenderer::drawCube()
//{
//    const VulkanDevice::SwapChainData& swapData = _vkDevice->getSwapChainData();
//    _swapImageLayouts.resize(swapData._images.size(), vk::ImageLayout::ePresentSrcKHR);
//
//    _program = VkProgram::create(_vkDevice);
//    vk::RenderPass renderPass = _vkDevice->createRenderPass(swapData._colorFormat, vk::Format::eD16Unorm);
//
//    _cubeVtxBuffer = VkBuffer::create(_vkDevice);
//    _cubeVtxBuffer->CreateVertexBuffer(coloredCubeData, coloredCubeSizeInBytes, sizeof(VertexPC));
//
//    _program->build(vertexShaderText_PC_C,
//                    fragmentShaderText_C_C,
//                    renderPass,
//                    true, // enableDepthTest
//                    vk::FrontFace::eClockwise);
//
//
//    std::vector<vk::Framebuffer> framebuffers =
//        _vkDevice->createFramebuffers(renderPass, swapData._imageViews,
//                                                  _frameBuffer->getDepthBufferImageView(),
//                                                  _surfaceExtent);
//
//
//    FrameData frame = acquireFrame(renderPass, framebuffers);
//    recordDrawCommands(frame.commandBuffer, renderPass, framebuffers[frame.imageIndex]);
//    submitAndPresent(frame, renderPass);
//}

ImagePtr VkRenderer::captureImage(ImagePtr /*image*/)
{
    // return _framebuffer->getColorImage(_cmdQueue, image);
    return nullptr;
}

MATERIALX_NAMESPACE_END
