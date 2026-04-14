#include "editor/editor.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <iostream>
#include <memory>
#include <rapidjson/istreamwrapper.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/systems/scene_rendering_system.hpp"
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "resources/texture.hpp"
#include "utilities/config_manager/config_manager.hpp"
#include "window/window.hpp"

namespace editor
{

static VkPipeline graphicsPipeline;
static VkPipelineLayout pipelineLayout;
static VkDescriptorSetLayout descriptorSetLayout;
static VkDescriptorPool descriptorPool;
static std::vector<VkDescriptorSet> descriptorSets;

static std::vector<VkBuffer> uniformBuffers;
static std::vector<VkDeviceMemory> uniformBuffersMemory;
static std::vector<void*> uniformBuffersMapped;

static VkBuffer vertexBuffer;
static VkDeviceMemory vertexBufferMemory;

static VkBuffer indicesBuffer;
static VkDeviceMemory indicesBufferMemory;

static VkBuffer stageBuffer;
static VkDeviceMemory stageBufferMemory;

struct Viewport
{
  VkImage image;
  VkImageView imageView;
  VkDeviceMemory memory;
};

static void createDescriptorPool( graphics::VulkanContext* ctx )
{
  std::array<VkDescriptorPoolSize, 2> poolSizes{
    VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
    VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * 2 } };

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT ) * 2;

  VK_CALL( vkCreateDescriptorPool( ctx->device->getLogicalDevice(), &poolInfo, nullptr, &descriptorPool ) );
}

static Viewport viewport;

void createViewport( graphics::VulkanContext* ctx )
{

  CreateImageInfo info{ .width = static_cast<uint32_t>( ctx->swapchain->getSwapchainExtent().width ),
                        .height = static_cast<uint32_t>( ctx->swapchain->getSwapchainExtent().height ),
                        .format = ctx->swapchain->getSwapchainImageFormat(),
                        .tiling = VK_IMAGE_TILING_OPTIMAL,
                        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                        .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        .image = &viewport.image,
                        .imageMemory = &viewport.memory };

  createImage( ctx, info );

  viewport.imageView =
    createImageView( ctx, viewport.image, ctx->swapchain->getSwapchainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT );
}

struct UniformBufferObject
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct Vertex
{
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription()
  {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof( Vertex );
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
  {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof( Vertex, pos );

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof( Vertex, color );

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof( Vertex, texCoord );

    return attributeDescriptions;
  }
};

const std::vector<Vertex> vertices = { { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
                                       { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
                                       { { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
                                       { { -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },

                                       { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
                                       { { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
                                       { { 0.5f, 0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
                                       { { -0.5f, 0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } } };

const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4 };

void createUniformBuffers( graphics::VulkanContext* ctx )
{
  uniformBuffers.resize( MAX_FRAMES_IN_FLIGHT );
  uniformBuffersMemory.resize( MAX_FRAMES_IN_FLIGHT );
  uniformBuffersMapped.resize( MAX_FRAMES_IN_FLIGHT );

  for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
  {
    createBuffer( ctx,
                  static_cast<VkDeviceSize>( sizeof( UniformBufferObject ) ),
                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  uniformBuffers[i],
                  uniformBuffersMemory[i] );

    vkMapMemory( ctx->device->getLogicalDevice(),
                 uniformBuffersMemory[i],
                 0,
                 static_cast<VkDeviceSize>( sizeof( UniformBufferObject ) ),
                 0,
                 &uniformBuffersMapped[i] );
  }
}

static void createGraphicsPipeline( graphics::VulkanContext* ctx )
{
  auto vert = std::filesystem::absolute( "." ) / "engine_resources\\shaders\\vert.spv";
  auto frag = std::filesystem::absolute( "." ) / "engine_resources\\shaders\\frag.spv";
  auto vertShaderCode = readFile( vert.string() );
  auto fragShaderCode = readFile( frag.string() );

  auto vertModule = createShaderModule( vertShaderCode, ctx->device->getLogicalDevice() );
  auto fragModule = createShaderModule( fragShaderCode, ctx->device->getLogicalDevice() );

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

  auto bindingDesc = Vertex::getBindingDescription();
  auto attribDesc = Vertex::getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &bindingDesc,
    .vertexAttributeDescriptionCount = static_cast<uint32_t>( attribDesc.size() ),
    .pVertexAttributeDescriptions = attribDesc.data(),
  };

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = &descriptorSetLayout };

  VK_CALL( vkCreatePipelineLayout( ctx->device->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout ) );

  VkPipelineRenderingCreateInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachmentFormats = &ctx->swapchain->getSwapchainImageFormat();
  renderingInfo.depthAttachmentFormat = findDepthFormat( &ctx->device->getPhysicalDevice() );
  renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

  VkPipelineDepthStencilStateCreateInfo depthStencil{ .sType =
                                                        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                                                      .depthTestEnable = VK_TRUE,
                                                      .depthWriteEnable = VK_TRUE,
                                                      .depthCompareOp = VK_COMPARE_OP_LESS,
                                                      .depthBoundsTestEnable = VK_FALSE,
                                                      .stencilTestEnable = VK_FALSE };

  VkGraphicsPipelineCreateInfo pipelineInfo{
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &renderingInfo,
    .stageCount = 2,
    .pStages = shaderStages,
    .pVertexInputState = &vertexInputInfo,
    .pInputAssemblyState = &inputAssembly,
    .pViewportState = &viewportState,
    .pRasterizationState = &rasterizer,
    .pMultisampleState = &multisampling,
    .pDepthStencilState = &depthStencil,
    .pColorBlendState = &colorBlending,
    .pDynamicState = &dynamicState,
    .layout = pipelineLayout,
    .renderPass = VK_NULL_HANDLE,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
  };

  VK_CALL( vkCreateGraphicsPipelines(
    ctx->device->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline ) );

  vkDestroyShaderModule( ctx->device->getLogicalDevice(), vertModule, nullptr );
  vkDestroyShaderModule( ctx->device->getLogicalDevice(), fragModule, nullptr );
}

void createDescriptorSets( graphics::VulkanContext* ctx )
{
  std::vector<VkDescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT, descriptorSetLayout );
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT );
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize( MAX_FRAMES_IN_FLIGHT );
  VK_CALL( vkAllocateDescriptorSets( ctx->device->getLogicalDevice(), &allocInfo, descriptorSets.data() ) );
  auto stopPath = std::filesystem::absolute( "." ) / "engine_resources\\textures\\logo.png";
  auto tex = core::AssetManager::get().addTexture( "logo.png", stopPath.string() );

  for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
  {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof( UniformBufferObject );

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = tex->getView();
    imageInfo.sampler = core::AssetManager::get().getTextureSampler();

    auto uniformBufferDescriptor = VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = descriptorSets[i],
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = &bufferInfo,
    };

    auto samplerDescriptor = VkWriteDescriptorSet{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                   .dstSet = descriptorSets[i],
                                                   .dstBinding = 1,
                                                   .dstArrayElement = 0,
                                                   .descriptorCount = 1,
                                                   .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                   .pImageInfo = &imageInfo };

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{ uniformBufferDescriptor, samplerDescriptor };

    vkUpdateDescriptorSets( ctx->device->getLogicalDevice(),
                            static_cast<uint32_t>( descriptorWrites.size() ),
                            descriptorWrites.data(),
                            0,
                            nullptr );
  }
}

static void createDescriptorSetLayout( graphics::VulkanContext* ctx )
{
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>( bindings.size() );
  layoutInfo.pBindings = bindings.data();

  VK_CALL( vkCreateDescriptorSetLayout( ctx->device->getLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout ) );
}

static void updateUniformBuffer( graphics::VulkanContext* ctx )
{
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>( currentTime - startTime ).count();

  UniformBufferObject ubo{};
  ubo.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
  ubo.view = glm::lookAt( glm::vec3( 3.0f, 3.0f, 3.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
  ubo.proj =
    glm::perspective( glm::radians( 65.0f ),
                      ctx->swapchain->getSwapchainExtent().width / (float)ctx->swapchain->getSwapchainExtent().height,
                      0.1f,
                      10.0f );
  ubo.proj[1][1] *= -1;

  memcpy( uniformBuffersMapped[ctx->swapchain->getCurrentFrameIndex()], &ubo, sizeof( ubo ) );
}

static void createIndexBuffer( graphics::VulkanContext* ctx )
{
  VkDeviceSize bufferSize = sizeof( indices[0] ) * indices.size();

  createBuffer( ctx,
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stageBuffer,
                stageBufferMemory );

  void* data;
  vkMapMemory( ctx->device->getLogicalDevice(), stageBufferMemory, 0, bufferSize, 0, &data );
  memcpy( data, indices.data(), (size_t)bufferSize );
  vkUnmapMemory( ctx->device->getLogicalDevice(), stageBufferMemory );

  createBuffer( ctx,
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                indicesBuffer,
                indicesBufferMemory );

  copyBuffer( ctx->swapchain->getCommandPool(),
              ctx->device->getLogicalDevice(),
              ctx->device->getGraphicsQueue().handle,
              stageBuffer,
              indicesBuffer,
              bufferSize );

  vkDestroyBuffer( ctx->device->getLogicalDevice(), stageBuffer, nullptr );
  vkFreeMemory( ctx->device->getLogicalDevice(), stageBufferMemory, nullptr );
}

static void createVertexBuffer( graphics::VulkanContext* ctx )
{
  VkDeviceSize bufferSize = sizeof( vertices[0] ) * vertices.size();
  createBuffer( ctx,
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stageBuffer,
                stageBufferMemory );

  void* data;
  vkMapMemory( ctx->device->getLogicalDevice(), stageBufferMemory, 0, bufferSize, 0, &data );
  memcpy( data, vertices.data(), (size_t)bufferSize );
  vkUnmapMemory( ctx->device->getLogicalDevice(), stageBufferMemory );

  createBuffer( ctx,
                bufferSize,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                vertexBuffer,
                vertexBufferMemory );

  copyBuffer( ctx->swapchain->getCommandPool(),
              ctx->device->getLogicalDevice(),
              ctx->device->getGraphicsQueue().handle,
              stageBuffer,
              vertexBuffer,
              bufferSize );

  vkDestroyBuffer( ctx->device->getLogicalDevice(), stageBuffer, nullptr );
  vkFreeMemory( ctx->device->getLogicalDevice(), stageBufferMemory, nullptr );
}

Editor::Editor()
{
  auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
  consoleSink->set_level( spdlog::level::debug );

  // File sink (only error and above)
  auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_st>( "logs/log.txt", 23, 59 );
  fileSink->set_level( spdlog::level::err );
  std::vector<spdlog::sink_ptr> sinks{ consoleSink, fileSink };

  auto logger = std::make_shared<spdlog::logger>( "app_logger", sinks.begin(), sinks.end() );

  logger->set_level( spdlog::level::debug );
  logger->set_pattern( "[%H:%M:%S] [%^%L%$] %v" );

  spdlog::set_default_logger( logger );

  utilities::EditorConfigManager::initConfig();

  init();
}

Editor::~Editor()
{
  cleanup();
}

void Editor::cleanup() const
{
  spdlog::info( "Closing editor and cleaning up" );
}

void Editor::pollEvents()
{
  SDL_Event e;
  while ( SDL_PollEvent( &e ) )
  {
    ImGui_ImplSDL2_ProcessEvent( &e );
    switch ( e.type )
    {
    case SDL_WINDOWEVENT: {
      if ( e.window.event == SDL_WINDOWEVENT_RESIZED )
      {
        int newWidth = e.window.data1;
        int newHeight = e.window.data2;
        // WindowResizeEvent windowResizeEvent{ newWidth, newHeight };
        // pEventDispatcher->dispatchEvent( windowResizeEvent );
      }
      break;
    }
    case SDL_QUIT: {
      // pEventDispatcher->dispatchEvent( WindowCloseEvent{} );
      m_running = false;
      break;
    }
    case SDL_KEYDOWN: {
      // KeyboardState::updateState();
      // auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );

      // KeyPressedEvent keyPressEvent{ scanCode, KeyScanCode::None, 0 };
      // if ( KeyboardState::getKeyState( KeyScanCode::LeftControl ) )
      //{
      //   keyPressEvent.setKeyModifier( KeyScanCode::LeftControl );
      // }

      // if ( KeyboardState::getKeyState( KeyScanCode::LeftShift ) )
      //{
      //   keyPressEvent.setKeyModifier( KeyScanCode::LeftShift );
      // }

      // pEventDispatcher->dispatchEvent( keyPressEvent );
      break;
    }
    case SDL_KEYUP: {
      // KeyboardState::updateState();
      // auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );
      // KeyReleasedEvent keyReleaseEvent{ scanCode, KeyScanCode::None };
      // pEventDispatcher->dispatchEvent( keyReleaseEvent );
      // break;
    }
    case SDL_MOUSEMOTION: {
      double x = e.motion.x;
      double y = e.motion.y;
      double xRel = e.motion.xrel;
      double yRel = e.motion.yrel;
      // MouseMovedEvent mouseMovedEvent{ x, y, xRel, yRel };
      // pEventDispatcher->dispatchEvent( mouseMovedEvent );
      break;
    }
    case SDL_MOUSEWHEEL: {
      double xOff = e.wheel.x;
      double yOff = e.wheel.y;
      // MouseScrolledEvent mouseScrolled{ xOff, yOff };
      // pEventDispatcher->dispatchEvent( mouseScrolled );
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      // UINT32 buttonState = SDL_GetMouseState( NULL, NULL );
      // if ( buttonState & SDL_BUTTON( SDL_BUTTON_MIDDLE ) )
      //{
      //   MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_MIDDLE ),
      //                                   static_cast<int>( MouseAction::Press ),
      //                                   static_cast<int>( MouseModifier::None ) };
      //   pEventDispatcher->dispatchEvent( mouseClicked );
      // }
      // if ( buttonState & SDL_BUTTON( SDL_BUTTON_LEFT ) )
      //{
      //   MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_LEFT ),
      //                                   static_cast<int>( MouseAction::Press ),
      //                                   static_cast<int>( MouseModifier::None ) };
      //   pEventDispatcher->dispatchEvent( mouseClicked );
      // }
      // if ( buttonState & SDL_BUTTON( SDL_BUTTON_RIGHT ) )
      //{
      //   MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_RIGHT ),
      //                                   static_cast<int>( MouseAction::Press ),
      //                                   static_cast<int>( MouseModifier::None ) };
      //   pEventDispatcher->dispatchEvent( mouseClicked );
      // }
      break;
    }
    default:
      break;
    }
  }
}

void Editor::run()
{
  auto& vkContext = core::MainRegistry::getInstance().getVulkanContext();
  auto& swapchain = vkContext->swapchain;

  auto& assetManager = core::AssetManager::get();
  assetManager.initSampler();

  createViewport( vkContext.get() );
  createUniformBuffers( vkContext.get() );
  createDescriptorSetLayout( vkContext.get() );
  createDescriptorPool( vkContext.get() );
  createDescriptorSets( vkContext.get() );
  createGraphicsPipeline( vkContext.get() );
  createVertexBuffer( vkContext.get() );
  createIndexBuffer( vkContext.get() );

  // VkImageMemoryBarrier depthBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
  //                                    .srcAccessMask = 0,
  //                                    .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  //                                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  //                                    .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
  //                                    .image = m_depthImage,
  //                                    .subresourceRange = {
  //                                      .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
  //                                      .levelCount = 1,
  //                                      .layerCount = 1,
  //                                    } };

  // VkRenderingAttachmentInfo depthAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
  //                                            .imageView = m_depthView,
  //                                            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
  //                                            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
  //                                            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
  //                                            .clearValue = { .depthStencil = { 1.f, 0 } } };

  while ( m_running )
  {
    pollEvents();
    swapchain->waitForFences();
    swapchain->resetFences();
    updateUniformBuffer( vkContext.get() );
    swapchain->aquireNextImage();

    auto& cmd = swapchain->getCurrentCommandBuffer();

    // set the current command buffer into begin state
    swapchain->beginCommandBuffer();
    VkImageMemoryBarrier textureToColor{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                         .srcAccessMask = 0,
                                         .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                         .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                         .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                         .image = viewport.image,
                                         .subresourceRange = {
                                           .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                           .levelCount = 1,
                                           .layerCount = 1,
                                         } };

    vkCmdPipelineBarrier( cmd,
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          0,
                          0,
                          nullptr,
                          0,
                          nullptr,
                          1,
                          &textureToColor );

    // vkCmdPipelineBarrier( cmd,
    //                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    //                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    //                       0,
    //                       0,
    //                       nullptr,
    //                       0,
    //                       nullptr,
    //                       1,
    //                       &depthBarrier );

    // this should include the viewport cause this is the whole engine UI
    VkRenderingAttachmentInfo colorAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                               .imageView = viewport.imageView,
                                               .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                               .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                               .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                               .clearValue = { { 0.0f, 0.0f, 0.0f, 0.8f } } };

    VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = { { 0, 0 }, swapchain->getSwapchainExtent() },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = nullptr,
    };

    swapchain->beginRendering( renderingInfo );
    swapchain->setupScissors( cmd );
    swapchain->setupViewport( cmd );
    // render the scene
    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );
    vkCmdBindVertexBuffers( cmd, 0, 1, vertexBuffers, offsets );
    vkCmdBindIndexBuffer( cmd, indicesBuffer, 0, VK_INDEX_TYPE_UINT16 );

    vkCmdBindDescriptorSets( cmd,
                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                             pipelineLayout,
                             0,
                             1,
                             &descriptorSets[swapchain->getCurrentFrameIndex()],
                             0,
                             nullptr );

    vkCmdDrawIndexed( cmd, static_cast<uint32_t>( indices.size() ), 1, 0, 0, 0 );

    swapchain->endRendering();

    VkImageMemoryBarrier textureToShader{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .image = viewport.image,
      .subresourceRange =
        {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
        },
    };

    vkCmdPipelineBarrier( cmd,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          0,
                          0,
                          nullptr,
                          0,
                          nullptr,
                          1,
                          &textureToShader );

    VkImageMemoryBarrier swapchainToColor{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image = swapchain->getCurrentFrame().image,
      .subresourceRange =
        {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1,
        },
    };

    vkCmdPipelineBarrier( cmd,
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          0,
                          0,
                          nullptr,
                          0,
                          nullptr,
                          1,
                          &swapchainToColor );

    VkRenderingAttachmentInfo imguiColorAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                                    .imageView = swapchain->getCurrentFrame().imageView,
                                                    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                    .clearValue = { { 0.f, 0.f, 0.f, 1.f } } };

    VkRenderingInfo imguiRenderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = { { 0, 0 }, swapchain->getSwapchainExtent() },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &imguiColorAttachment,
      .pDepthAttachment = nullptr,
    };

    // render imgui
    swapchain->beginRendering( imguiRenderingInfo );
    m_pImguiRenderer->setViewport( viewport.imageView );
    m_pImguiRenderer->render();
    m_pImguiRenderer->present( cmd );
    swapchain->endRendering();

    // present the rendering result to the screen
    swapchain->presentFrame();
  }
}

bool Editor::initSDL()
{
  if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ) != 0 )
  {
    spdlog::error( "SDL_Init Error: {}", SDL_GetError() );
    throw std::runtime_error( "SDL_Init failed" );
  }

  if ( SDL_Vulkan_LoadLibrary( nullptr ) != 0 )
  {
    spdlog::error( "SDL Vulkan load failed: {}", SDL_GetError() );
    throw std::runtime_error( "could not load lib vulkan" );
  }

  return true;
}

bool Editor::initVulkan()
{
  auto vkCtx = core::MainRegistry::getInstance().getVulkanContext();
  // TODO(kogayonon) this does not look clean, make the asset manager ctor better or smth
  auto& assetManager = core::AssetManager::get();
  assetManager.setContext( vkCtx.get() );

  return true;
}

bool Editor::initImgui()
{
  auto vkCtx = core::MainRegistry::getInstance().getVulkanContext();

  m_pImguiRenderer =
    std::make_shared<gui::VulkanImguiRenderer>( m_pWindow->getWindow(), vkCtx->device.get(), vkCtx->swapchain.get() );

  return true;
}

bool Editor::initMainWindow()
{
  auto& cfg = utilities::EditorConfigManager::getConfig();

  m_pWindow = std::make_shared<window::Window>( "kogayonon engine", cfg.width, cfg.height, false, cfg.maximized );
  m_pWindow->setBordered( true );
  m_pWindow->setResizable( true );
  return true;
}

bool Editor::init()
{

  if ( !initSDL() )
  {
    throw std::runtime_error( "sdl could not be initialized" );
  }

  if ( !initMainWindow() )
  {
    throw std::runtime_error( "could not initialize main window" );
  }

  if ( !initMainRegistry() )
  {
    throw std::runtime_error( "main registry could not be initialized" );
  }

  if ( !initVulkan() )
  {
    throw std::runtime_error( "vulkan could not be initialized" );
  }

  if ( !initImgui() )
  {
    throw std::runtime_error( "vulkan could not be initialized" );
  }

  m_running = true;
  return true;
}

bool Editor::initMainRegistry()
{
  auto device = std::make_shared<graphics::VulkanDevice>( m_pWindow->getWindow() );
  auto swapchain = std::make_shared<graphics::VulkanSwapchain>( device.get(), m_pWindow->getWindow() );
  auto vkCtx = std::make_shared<graphics::VulkanContext>( device, swapchain );
  auto& mainRegistry = core::MainRegistry::getInstance();
  assert( vkCtx.get() && "could not create vulkan context" );
  mainRegistry.addToContext<std::shared_ptr<graphics::VulkanContext>>( std::move( vkCtx ) );
  return true;
}

} // namespace editor