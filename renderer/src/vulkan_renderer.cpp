#include "renderer/vulkan_renderer.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/text_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "graphics/vulkan_context.hpp"
#include "gui/imgui_windows/viewport.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "physics/jolt_physics.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/geometry_module.hpp"
#include "renderer/modules/imgui_module.hpp"
#include "renderer/modules/picking_module.hpp"
#include "resources/mesh_push_constant.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "utilities/tracy_utils/tracy_vulkan_utils.hpp"
#include "utilities/utils/utils.hpp"
#include <SDL2/SDL.h>

rendering::VulkanRenderer::VulkanRenderer( graphics::VulkanContext* pCtx, SDL_Window* window )
    : m_vkCtx{ pCtx }
    , m_wnd{ window }
    , m_mouseCoords{ -1, -1 }
{
  core::EventDispatcher* eventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
  core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
  utilities::TimeTracker* timeTracker = core::MainRegistry::getInstance().getTimeTracker();

  timeTracker->start( "resize" );
  eventDispatcher->addHandler<core::MouseClickedEvent, &VulkanRenderer::onMouseClicked>( *this );

  createCameraDescriptorSetLayout();
  createCameraBuffers();
  createCameraDescriptorSet();

  initImgui();

  m_frameGraph = std::make_unique<FrameGraph>( m_vkCtx->device.get() );

  VkExtent2D extent = m_vkCtx->swapchain->getSwapchainExtent();

  m_geometryModule = std::make_unique<GeometryModule>(
    m_frameGraph.get(),
    m_vkCtx,
    extent,
    ModuleDescriptorData{ .descriptorSetLayouts = { m_cameraDescriptor.layout,
                                                    assetManager->getBindlessDescriptorLayout(),
                                                    assetManager->getMaterialsDescriptorLayout() },
                          .descriptorSets = { m_cameraDescriptor.set,
                                              assetManager->getBindlessDescriptorSet(),
                                              assetManager->getMaterialsDescriptorSet() } } );

  m_pickingModule =
    std::make_unique<PickingModule>( m_vkCtx,
                                     m_frameGraph.get(),
                                     m_pImguiRenderer.get(),
                                     extent,
                                     ModuleDescriptorData{ .descriptorSetLayouts = { m_cameraDescriptor.layout },
                                                           .descriptorSets = { m_cameraDescriptor.set } } );

  m_imguiModule = std::make_unique<ImGuiModule>( m_vkCtx, m_frameGraph.get(), m_pImguiRenderer.get() );

  m_frameGraph->compile();
}

rendering::VulkanRenderer::~VulkanRenderer()
{
  m_vkCtx->device->destroyBuffer( m_cameraBuffers );
  m_vkCtx->device->destroyDescriptorSetLayout( m_cameraDescriptor.layout );
}

auto rendering::VulkanRenderer::render() -> void
{
  m_vkCtx->swapchain->waitForFences();
  m_vkCtx->swapchain->resetFences();

  updateCameraBuffer();

  m_vkCtx->swapchain->aquireNextImage();
  m_vkCtx->swapchain->beginCommandBuffer();

  // swapchain image should be transitioned by the graph
  m_vkCtx->swapchain->prepareAttachment();

  m_frameGraph->execute( m_vkCtx->swapchain->getCurrentCommandBuffer() );
}

auto rendering::VulkanRenderer::createCameraBuffers() -> void
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof( core::CameraUbo ) + 2;
  bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  m_vkCtx->device->createBuffer( m_cameraBuffers, bufferInfo, vmaAllocInfo, "cameraBuffer" );

  for ( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; ++i )
  {
    m_vkCtx->device->setName( std::string{ "cameraBuffer_" + std::to_string( i ) },
                              m_cameraBuffers.buffers.at( i ).vmaAllocation );
  }

  m_cameraBuffers.each( [this]( const graphics::VulkanBuffer& buffer, uint32_t index ) {
    std::string name = std::string{ "cameraBuffer_" + std::to_string( index ) };
    m_vkCtx->device->setDebugName(
      VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>( m_cameraBuffers.buffers.at( index ).vkBuffer ), name );
  } );
}

auto rendering::VulkanRenderer::updateCameraBuffer() -> void
{
  core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
  core::Scene* scene = sceneManager->getCurrentScene();
  auto view = scene->getEnttRegistry().view<core::PerspectiveCameraComponent>();
  view.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComp ) {
    if ( cameraComp.isUsed )
    {
      if ( cameraComp.props.changed )
      {
        cameraComp.updateUbo();
      }

      m_vkCtx->device->copyDataToBuffer( cameraComp.ubo,
                                         m_cameraBuffers.buffers.at( m_vkCtx->swapchain->getCurrentFrameNumber() ),
                                         sizeof( core::CameraUbo ) );

      // vmaCopyMemoryToAllocation(
      //   m_vkCtx->device->getAllocator(),
      //   &cameraComp.ubo,
      //   m_cameraBuffers.buffers.at( m_vkCtx->swapchain->getCurrentFrameNumber() ).vmaAllocation,
      //   0,
      //   sizeof( core::CameraUbo ) );
    }
  } );
}

auto rendering::VulkanRenderer::createCameraDescriptorSet() -> void
{
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_vkCtx->globalDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_cameraDescriptor.layout;
  allocInfo.pNext = nullptr;

  m_vkCtx->device->allocateDescriptorSet( m_cameraDescriptor.set, allocInfo );

  std::vector<VkDescriptorBufferInfo> bufferInfo{};
  bufferInfo.resize( MAX_FRAMES_IN_FLIGHT );
  for ( auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
  {
    VkDescriptorBufferInfo info{};
    bufferInfo.at( i ).buffer = m_cameraBuffers.buffers.at( i ).vkBuffer;
    bufferInfo.at( i ).offset = 0;
    bufferInfo.at( i ).range = sizeof( core::CameraUbo );
  }

  VkWriteDescriptorSet uniformBufferDescriptor{
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = m_cameraDescriptor.set,
    .dstBinding = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .pBufferInfo = bufferInfo.data(),
  };

  VkWriteDescriptorSet descriptorWrites = uniformBufferDescriptor;
  vkUpdateDescriptorSets( m_vkCtx->device->getLogicalDevice(), 1, &descriptorWrites, 0, nullptr );
}

auto rendering::VulkanRenderer::createCameraDescriptorSetLayout() -> void
{
  VkDescriptorSetLayoutBinding cameraBufferBinding{};
  cameraBufferBinding.binding = 0;
  cameraBufferBinding.descriptorCount = 1;

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

  m_vkCtx->device->createDescriptorSetLayout( m_cameraDescriptor.layout, layoutInfo );
}

auto rendering::VulkanRenderer::initImgui() -> void
{
  m_pImguiRenderer =
    std::make_shared<gui::VulkanImguiRenderer>( m_wnd, m_vkCtx->device.get(), m_vkCtx->swapchain.get() );
}

auto rendering::VulkanRenderer::onMouseClicked( const core::MouseClickedEvent& e ) -> void
{
  core::SceneEventHandler* sceneHandler = core::MainRegistry::getInstance().getSceneManager()->getEventHandler();
  entt::entity currentEntity = sceneHandler->getCurrentEntityId();

  int mouseX, mouseY;
  SDL_GetMouseState( &mouseX, &mouseY );

  gui::Viewport* viewport =
    dynamic_cast<gui::Viewport*>( m_pImguiRenderer->getImGuiWindows().at( gui::ImGuiWindowName::Viewport ).get() );

  gui::ImGuiProps* props = viewport->getProps();

  VkExtent2D extent = m_vkCtx->swapchain->getSwapchainExtent();

  float localX = ( mouseX - props->x ) / props->width;
  float localY = ( mouseY - props->y ) / props->height;

  if ( localX >= 0.0f && localX <= 1.0f && localY >= 0.0f && localY <= 1.0f )
  {
    m_mouseCoords.x = static_cast<int>( localX * extent.width );
    m_mouseCoords.y = static_cast<int>( localY * extent.height );
    m_pickingModule->setCoords( m_mouseCoords );
  }
}
