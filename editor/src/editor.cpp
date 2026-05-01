#include "editor/editor.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <rapidjson/istreamwrapper.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/app_event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "core/systems/scene_rendering_system.hpp"
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"
#include "utilities/config_manager/config_manager.hpp"
#include "utilities/utils/utils.hpp"
#include "window/window.hpp"

namespace editor
{

// clean this up
static VkPipeline graphicsPipeline;
static VkPipelineLayout pipelineLayout;

static VkDescriptorSetLayout descriptorSetLayout;
static VkDescriptorPool descriptorPool;
static std::vector<VkDescriptorSet> globalDescriptorSets;

static VkDescriptorSet cameraDescriptor;
static VkDescriptorSetLayout cameraDescriptorLayout;

static std::vector<graphics::VulkanBuffer> uniformBuffers;

static VkBuffer stageBuffer;
static VkDeviceMemory stageBufferMemory;

static VkImage depthImage;
static VkImageView depthView;
static VmaAllocation depthAllocation;

// the "game" is drawn onto this
struct VulkanViewport
{
  VkImage image;
  VkImageView imageView;
  VmaAllocation allocation;
};

static VulkanViewport viewport;

void createViewport( graphics::VulkanContext* ctx )
{
}

struct CameraBuffer
{
  glm::mat4 view;
  glm::mat4 proj;
} ubo;

void createUniformBuffers( graphics::VulkanContext* ctx )
{
  uniformBuffers.resize( MAX_FRAMES_IN_FLIGHT );

  for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
  {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof( CameraBuffer );
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    ctx->memoryAllocator->createBuffer( uniformBuffers.at( i ), bufferInfo, vmaAllocInfo );
    vmaMapMemory(
      ctx->memoryAllocator->getAllocator(), uniformBuffers.at( i ).allocation, &uniformBuffers.at( i ).mappedData );

    uniformBuffers.at( i ).persistent = true;
  }
}

static void createGraphicsPipeline( graphics::VulkanContext* ctx )
{
  auto vert = std::filesystem::absolute( "." ) / "engine_resources\\shaders\\vulkan_vertex.spv";
  auto frag = std::filesystem::absolute( "." ) / "engine_resources\\shaders\\vulkan_fragment.spv";

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

  auto bindingDesc = resources::Vertex::getBindingDescription();
  auto attribDesc = resources::Vertex::getAttributeDescriptions();

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

  VkPipelineRasterizationStateCreateInfo rasterizer{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE /*VK_CULL_MODE_BACK_BIT*/,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.0f,
  };

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

  // create the mesh constants here
  VkPushConstantRange meshPushConstant{};
  meshPushConstant.offset = 0;
  meshPushConstant.size = sizeof( resources::MeshPushConstant );
  meshPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  auto& assetManager = core::AssetManager::getInstance();
  auto& bindlessDescriptorLayout = assetManager.getBindlessDescriptorLayout();
  auto& materialsDescriptorLayout = assetManager.getMaterialsDescriptorLayout();
  VkDescriptorSetLayout layouts[] = {
    descriptorSetLayout /* this is the camera set, should be on index 1*/,
    bindlessDescriptorLayout,
    materialsDescriptorLayout,
  };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                 .setLayoutCount = std::size( layouts ),
                                                 .pSetLayouts = layouts,
                                                 .pushConstantRangeCount = 1,
                                                 .pPushConstantRanges = &meshPushConstant };

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

static void createDescriptorPool( graphics::VulkanContext* ctx )
{
  std::vector<VkDescriptorPoolSize> poolSizes{
    VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
    VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT },
    VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 } };

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = 0;
  poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 3000;

  VK_CALL( vkCreateDescriptorPool( ctx->device->getLogicalDevice(), &poolInfo, nullptr, &descriptorPool ) );
}

void createCameraDescriptorSets( graphics::VulkanContext* ctx )
{
  uint32_t descriptorCount[]{ 1 };
  std::vector<VkDescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT, descriptorSetLayout );

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = std::size( layouts );
  allocInfo.pSetLayouts = layouts.data();
  allocInfo.pNext = nullptr;

  globalDescriptorSets.resize( MAX_FRAMES_IN_FLIGHT );
  VK_CALL( vkAllocateDescriptorSets( ctx->device->getLogicalDevice(), &allocInfo, globalDescriptorSets.data() ) );
  for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
  {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers[i].vkBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof( CameraBuffer );

    auto uniformBufferDescriptor = VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = globalDescriptorSets[i],
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = &bufferInfo,
    };

    VkWriteDescriptorSet descriptorWrites = uniformBufferDescriptor;
    vkUpdateDescriptorSets( ctx->device->getLogicalDevice(), 1, &descriptorWrites, 0, nullptr );
  }
}

static void createCameraDescriptorSetLayout( graphics::VulkanContext* ctx )
{
  VkDescriptorSetLayoutBinding cameraBufferBinding{};
  cameraBufferBinding.binding = 0;
  cameraBufferBinding.descriptorCount = 1;

  // this is a uniform buffer
  cameraBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  cameraBufferBinding.pImmutableSamplers = nullptr;
  cameraBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorBindingFlags flags = 0;

  VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
  bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
  bindingFlags.bindingCount = 1;
  bindingFlags.pBindingFlags = &flags;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &cameraBufferBinding;
  layoutInfo.pNext = nullptr;
  layoutInfo.flags = 0;

  VK_CALL( vkCreateDescriptorSetLayout( ctx->device->getLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout ) );
}

static void updateUniformBuffer( graphics::VulkanContext* ctx )
{
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>( currentTime - startTime ).count();

  ubo.view =
    glm::lookAt( glm::vec3( 300.0f, 300.0f, 3.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
  ubo.proj =
    glm::perspective( glm::radians( 50.0f ),
                      ctx->swapchain->getSwapchainExtent().width / (float)ctx->swapchain->getSwapchainExtent().height,
                      0.1f,
                      1000.0f );

  ubo.proj[1][1] *= -1;

  memcpy( uniformBuffers.at( ctx->swapchain->getCurrentFrameIndex() ).mappedData, &ubo, sizeof( CameraBuffer ) );
  // vmaCopyMemoryToAllocation( ctx->memoryAllocator->getAllocator(),
  //                            &ubo,
  //                            uniformBuffers.at( ctx->swapchain->getCurrentFrameIndex() ).allocation,
  //                            0,
  //                            sizeof( CameraBuffer ) );
}

Editor::Editor()
{
  auto consoleSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
  consoleSink->set_level( spdlog::level::debug );

  // file sink (only error and above)
  auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_st>( "logs/log.txt", 23, 59 );
  fileSink->set_level( spdlog::level::err );
  std::vector<spdlog::sink_ptr> sinks{ consoleSink, fileSink };

  auto logger = std::make_shared<spdlog::logger>( "app_logger", sinks.begin(), sinks.end() );

  logger->set_level( spdlog::level::debug );
  logger->set_pattern( "[%H:%M:%S] [%^%L%$] %v" );

  spdlog::set_default_logger( logger );

  utilities::EditorConfigManager::initConfig();

  KeyboardState::initState();

  init();
}

Editor::~Editor()
{
}

void Editor::cleanup() const
{
  auto& vkCtx = core::MainRegistry::getInstance().getVulkanContext();
  vkCtx->memoryAllocator->deallocate();
}

void Editor::pollEvents()
{
  const auto& pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();

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
        core::WindowResizeEvent windowResizeEvent{ newWidth, newHeight };
        pEventDispatcher->dispatchEvent( windowResizeEvent );
      }
      break;
    }
    case SDL_QUIT: {
      pEventDispatcher->dispatchEvent( core::WindowCloseEvent{} );
      m_running = false;
      break;
    }
    case SDL_KEYDOWN: {
      KeyboardState::updateState();
      auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );

      core::KeyPressedEvent keyPressEvent{ scanCode, KeyScanCode::None, 0 };
      if ( KeyboardState::getKeyState( KeyScanCode::LeftControl ) )
      {
        keyPressEvent.setKeyModifier( KeyScanCode::LeftControl );
      }

      if ( KeyboardState::getKeyState( KeyScanCode::LeftShift ) )
      {
        keyPressEvent.setKeyModifier( KeyScanCode::LeftShift );
      }

      pEventDispatcher->dispatchEvent( keyPressEvent );
      break;
    }
    case SDL_KEYUP: {
      KeyboardState::updateState();
      auto scanCode = static_cast<KeyScanCode>( e.key.keysym.scancode );
      core::KeyReleasedEvent keyReleaseEvent{ scanCode, KeyScanCode::None };
      pEventDispatcher->dispatchEvent( keyReleaseEvent );
      break;
    }
    case SDL_MOUSEMOTION: {
      double x = e.motion.x;
      double y = e.motion.y;
      double xRel = e.motion.xrel;
      double yRel = e.motion.yrel;
      core::MouseMovedEvent mouseMovedEvent{ x, y, xRel, yRel };
      pEventDispatcher->dispatchEvent( mouseMovedEvent );
      break;
    }
    case SDL_MOUSEWHEEL: {
      double xOff = e.wheel.x;
      double yOff = e.wheel.y;
      core::MouseScrolledEvent mouseScrolled{ xOff, yOff };
      pEventDispatcher->dispatchEvent( mouseScrolled );
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      UINT32 buttonState = SDL_GetMouseState( NULL, NULL );
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_MIDDLE ) )
      {
        core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_MIDDLE ),
                                              static_cast<int>( MouseAction::Press ),
                                              static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->dispatchEvent( mouseClicked );
      }
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_LEFT ) )
      {
        core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_LEFT ),
                                              static_cast<int>( MouseAction::Press ),
                                              static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->dispatchEvent( mouseClicked );
      }
      if ( buttonState & SDL_BUTTON( SDL_BUTTON_RIGHT ) )
      {
        core::MouseClickedEvent mouseClicked{ static_cast<int>( MouseCode::BUTTON_RIGHT ),
                                              static_cast<int>( MouseAction::Press ),
                                              static_cast<int>( MouseModifier::None ) };
        pEventDispatcher->dispatchEvent( mouseClicked );
      }
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

  auto& assetManager = core::AssetManager::getInstance();
  assetManager.initSampler();

  createViewport( vkContext.get() );
  createUniformBuffers( vkContext.get() );
  createCameraDescriptorSetLayout( vkContext.get() );
  createDescriptorPool( vkContext.get() );
  createCameraDescriptorSets( vkContext.get() );

  assetManager.setDescriptorPool( &descriptorPool );
  assetManager.initDescriptors();

  // create pipeline last since we need all descriptor layouts for pipeline creation
  createGraphicsPipeline( vkContext.get() );

  while ( m_running )
  {
    pollEvents();
    auto scene = core::SceneManager::getCurrentScene().lock();

    if ( !swapchain->isRendering() )
      continue;

    swapchain->waitForFences();
    swapchain->resetFences();
    updateUniformBuffer( vkContext.get() );

    const auto& view = scene->getEnttRegistry().view<core::MeshComponent, core::TransformComponent>();

    // update transforms if they are marked as update needed
    view.each(
      [&]( const entt::entity& entityId, core::MeshComponent& meshComponent, core::TransformComponent& transform ) {
        if ( transform.update )
        {
          transform.computeMatrix();
        }
      } );

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

    VkImageMemoryBarrier depthBarrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                       .srcAccessMask = 0,
                                       .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                       .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                       .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                       .image = depthImage,
                                       .subresourceRange = {
                                         .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                         .levelCount = 1,
                                         .layerCount = 1,
                                       } };

    VkRenderingAttachmentInfo depthAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                               .imageView = depthView,
                                               .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                               .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                               .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                               .clearValue = { .depthStencil = { 1.f, 0 } } };

    vkCmdPipelineBarrier( cmd,
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                          0,
                          0,
                          nullptr,
                          0,
                          nullptr,
                          1,
                          &depthBarrier );

    // this should include the viewport cause this is the whole engine UI
    VkRenderingAttachmentInfo colorAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                               .imageView = viewport.imageView,
                                               .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                               .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                               .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                               .clearValue = { { 0.350f, 0.350f, 0.350f, 0.8f } } };

    VkRenderingInfo renderingInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = { { 0, 0 }, swapchain->getSwapchainExtent() },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = &depthAttachment,
    };

    swapchain->beginRendering( renderingInfo );
    swapchain->setupScissors( cmd );
    swapchain->setupViewport( cmd );

    VkDeviceSize offsets[] = { 0 };

    vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );

    vkCmdBindDescriptorSets( cmd,
                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                             pipelineLayout,
                             0, // set = 0
                             1,
                             &globalDescriptorSets[swapchain->getCurrentFrameIndex()],
                             0,
                             nullptr );

    // this is the bindless texture set
    vkCmdBindDescriptorSets( cmd,
                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                             pipelineLayout,
                             1, // set = 1
                             1,
                             &assetManager.getBindlessDescriptorSet(),
                             0,
                             nullptr );

    view.each(
      [&]( const entt::entity& entityId, core::MeshComponent& meshComponent, core::TransformComponent& transform ) {
        vkCmdBindDescriptorSets( cmd,
                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 pipelineLayout,
                                 2, // set = 2
                                 1,
                                 &assetManager.getMaterialsDescriptorSet(),
                                 0,
                                 nullptr );

        vkCmdBindVertexBuffers( cmd, 0, 1, &meshComponent.pMesh->getVertexBufferObject().vkBuffer, offsets );
        vkCmdBindIndexBuffer( cmd, meshComponent.pMesh->getIndicesBufferObject().vkBuffer, 0, VK_INDEX_TYPE_UINT32 );
        for ( auto& submesh : meshComponent.pMesh->getSubmeshes() )
        {
          auto push =
            resources::MeshPushConstant{ .modelMatrix = transform.getMatrix(), .materialIndex = submesh.materialIndex };

          vkCmdPushConstants( cmd,
                              pipelineLayout,
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                              0,
                              sizeof( resources::MeshPushConstant ),
                              &push );

          vkCmdDrawIndexed( cmd, submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0 );
        }
      } );

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
  auto& assetManager = core::AssetManager::getInstance();
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

void Editor::onWindowClose( const core::WindowCloseEvent& e )
{
}

bool Editor::initMainRegistry()
{
  auto device = std::make_shared<graphics::VulkanDevice>( m_pWindow->getWindow() );
  auto swapchain = std::make_shared<graphics::VulkanSwapchain>( device.get(), m_pWindow->getWindow() );

  auto vma = std::make_shared<graphics::VulkanMemoryAllocator>(
    device->getLogicalDevice(), device->getInstance(), device->getPhysicalDevice() );

  auto vkCtx = std::make_shared<graphics::VulkanContext>( graphics::VulkanContext{
    .device = std::move( device ), .swapchain = std::move( swapchain ), .memoryAllocator = std::move( vma ) } );

  auto& mainRegistry = core::MainRegistry::getInstance();

  assert( vkCtx.get() && "could not create vulkan context" );
  mainRegistry.addToContext<std::shared_ptr<graphics::VulkanContext>>( std::move( vkCtx ) );

  auto eventDispatcher = std::make_shared<core::EventDispatcher>();
  eventDispatcher->addHandler<core::WindowCloseEvent, &Editor::onWindowClose>( *this );
  assert( eventDispatcher && "could not init event dispathcer" );
  mainRegistry.addToContext<std::shared_ptr<core::EventDispatcher>>( std::move( eventDispatcher ) );

  // TODO(kogayonon) remove this scene code from here
  auto scene = std::make_shared<core::Scene>( "Default" );
  core::SceneManager::addScene( scene );
  core::SceneManager::setCurrentScene( scene->getName() );
  //

  return true;
}

} // namespace editor
