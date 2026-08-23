#include "renderer/modules/picking_module.hpp"
#include "SDL2/SDL.h"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/imgui_windows/imgui_base.hpp"
#include "gui/imgui_windows/viewport.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/geometry_module.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/vertex.hpp"
#include "utilities/tracy_utils/tracy_utils.hpp"

rendering::PickingModule::PickingModule( graphics::VulkanContext* vkCtx,
                                         FrameGraph* graph,
                                         gui::VulkanImguiRenderer* imguiRenderer,
                                         VkExtent2D extent,
                                         ModuleDescriptorData descriptorData )
    : m_graph{ graph }
    , m_vkCtx{ vkCtx }
    , m_extent{ extent }
    , m_mouseCoords{ -1, -1 }
    , m_frameCounter{ 0u }
    , m_readyToCopyImage{ false }
    , m_readyToRead{ false }
    , m_imguiRenderer{ imguiRenderer }
    , m_moduleDescriptorData{ descriptorData }
{
  createModuleResources();
  registerPasses();

  core::EventDispatcher* eventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
  eventDispatcher->addHandler<core::MouseClickedEvent, &PickingModule::onMouseClicked>( *this );
}

rendering::PickingModule::~PickingModule()
{
  PickingModuleData& pickingData = m_graph->getBlackboard()->get<PickingModuleData>();

  m_vkCtx->device->destroyBuffer( pickingData.pickingbuffer );
  m_vkCtx->device->destroyImageView( pickingData.color->vulkanImage.vkImageView );
  m_vkCtx->device->destroyImage( pickingData.color->vulkanImage.vkImage, pickingData.color->vulkanImage.vmaAllocation );
  m_vkCtx->device->destroyPipelineLayout( pickingData.pickingPipeline.getLayout() );
  m_vkCtx->device->destroyPipeline( pickingData.pickingPipeline.getPipeline() );
}

auto rendering::PickingModule::registerPasses() -> void
{
  registerPickingPass();
  registerPickingReadbackPass();
  registerPickingEntityReadPass();
}

auto rendering::PickingModule::setCoords( glm::ivec2 coords ) -> void
{
  m_mouseCoords = coords;
}

auto rendering::PickingModule::registerPickingPass() -> void
{
  Blackboard* blackboard = m_graph->getBlackboard();

  PickingModuleData& pickingData = blackboard->get<PickingModuleData>();

  VkShaderModule vertex = m_vkCtx->device->createShaderModule( "picking", "vertexMain" );
  VkShaderModule fragment = m_vkCtx->device->createShaderModule( "picking", "fragmentMain" );

  graphics::VulkanPipelineSpec defaultPipelineSpec{
    .type = graphics::PipelineType::geometry,
    .options = { .cullMode = VK_CULL_MODE_BACK_BIT, .polyMode = VK_POLYGON_MODE_FILL },
    .descriptorLayout = m_moduleDescriptorData.descriptorSetLayouts,
    .colorAttachmentFormat = { VK_FORMAT_R32_SINT },
    .vertexModule = vertex,
    .fragmentModule = fragment,
    .pushConstantSize = sizeof( resources::EntityPickingPushConstant ),
    .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .colorAttachmentCount = 1,
    .vertexBindingDescription = resources::Vertex::getBindingDescription(),
    .vertexAttributesDescription = resources::Vertex::getAttributeDescriptions() };

  pickingData.pickingPipeline.create( defaultPipelineSpec, m_vkCtx );

  m_vkCtx->device->destroyShaderModule( vertex );
  m_vkCtx->device->destroyShaderModule( fragment );

  m_graph->addNode(
    std::string{ passId::Picking },
    []( NodeBuilder& b, Blackboard* blackboard ) {
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();
      GeometryModuleData& geometryData = blackboard->get<GeometryModuleData>();

      b.write( pickingData.color, FGResourceType::Color );
      b.read( geometryData.depth, FGResourceType::Depth );
    },
    [=, coords = &m_mouseCoords, readyToCopyImage = &m_readyToCopyImage, frameCounter = &m_frameCounter](
      VkCommandBuffer buffer ) {
      if ( ( coords->x == -1 && coords->y == -1 ) || *readyToCopyImage == true )
        return;

      Blackboard* blackboard = m_graph->getBlackboard();
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();
      GeometryModuleData& geometryData = blackboard->get<GeometryModuleData>();

      pickingData.renderingInfo.colorAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = pickingData.color->vulkanImage.vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { { -1.0f, -1.0f, -1.0f, -1.0f } } };

      pickingData.renderingInfo.depthAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = geometryData.depth->vulkanImage.vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { .depthStencil = { 1.f, 0 } } };

      pickingData.renderingInfo.vkRenderingInfo = VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { { 0, 0 }, m_extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &pickingData.renderingInfo.colorAttachmentInfo,
        .pDepthAttachment = &pickingData.renderingInfo.depthAttachmentInfo,
      };

      m_vkCtx->swapchain->beginRendering( pickingData.renderingInfo.vkRenderingInfo );
      m_vkCtx->swapchain->setupScissors( buffer );
      m_vkCtx->swapchain->setupViewport( buffer );

      graphics::VulkanPipeline& pickingPipeline = pickingData.pickingPipeline;
      pickingPipeline.bind( buffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               pickingPipeline.getLayout(),
                               0,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 0 ),
                               0,
                               nullptr );

      core::Scene* scene = core::MainRegistry::getInstance().getSceneManager()->getCurrentScene();

      auto view = scene->getEnttRegistry().view<core::MeshComponent, core::TransformComponent>();
      view.each( [&]( const entt::entity& entityId,
                      core::MeshComponent& meshComponent,
                      core::TransformComponent& transform ) {
        if ( !meshComponent.loaded )
          return;

        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers( buffer, 0, 1, &meshComponent.pMesh->getVertexBufferObject().vkBuffer, offsets );
        vkCmdBindIndexBuffer( buffer, meshComponent.pMesh->getIndicesBufferObject().vkBuffer, 0, VK_INDEX_TYPE_UINT32 );
        for ( auto& submesh : meshComponent.pMesh->getSubmeshes() )
        {
          auto push = resources::EntityPickingPushConstant{ .modelMatrix = transform.getMatrix(),
                                                            .entityId = static_cast<int>( entityId ) };

          vkCmdPushConstants( buffer,
                              pickingPipeline.getLayout(),
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                              0,
                              sizeof( resources::EntityPickingPushConstant ),
                              &push );

          vkCmdDrawIndexed( buffer, submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0 );
        }
      } );

      m_vkCtx->swapchain->endRendering();
    } );
}

auto rendering::PickingModule::registerPickingReadbackPass() -> void
{

  m_graph->addNode(
    std::string{ passId::PickingReadback },
    []( NodeBuilder& b, Blackboard* blackboard ) {
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();
      b.read( pickingData.color, FGResourceType::ColorTransfer );
    },
    [=,
     coords = &m_mouseCoords,
     readyToCopyImage = &m_readyToCopyImage,
     readyToCopyData = &m_readyToRead,
     frameCounter = &m_frameCounter]( VkCommandBuffer buffer ) {
      Blackboard* blackboard = m_graph->getBlackboard();
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();

      if ( coords->x == -1 && coords->y == -1 )
        return;

      if ( *frameCounter < 1 )
      {
        ++( *frameCounter );
        return;
      }

      VkBufferImageCopy region{};
      region.bufferOffset = 0;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;
      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = 0;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount = 1;
      region.imageOffset = { coords->x, coords->y, 0 };
      region.imageExtent = { 1, 1, 1 };

      vkCmdCopyImageToBuffer( buffer,
                              pickingData.color->vulkanImage.vkImage,
                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              pickingData.pickingbuffer.vkBuffer,
                              1,
                              &region );
      coords->x = -1;
      coords->y = -1;
      *frameCounter = 0;
      *readyToCopyData = true;
    } );
}

auto rendering::PickingModule::registerPickingEntityReadPass() -> void
{

  m_graph->addNode(
    std::string{ passId::PickingEntityRead },
    []( NodeBuilder& b, Blackboard* blackboard ) {
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();
      b.read( pickingData.color, FGResourceType::ColorTransfer );
    },
    [=, coords = &m_mouseCoords, readyToCopyData = &m_readyToRead, frameCounter = &m_frameCounter](
      VkCommandBuffer buffer ) {
      if ( *readyToCopyData == false )
        return;

      Blackboard* blackboard = m_graph->getBlackboard();
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();

      core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
      core::Scene* scene = sceneManager->getCurrentScene();

      void* data{ nullptr };
      vmaMapMemory( m_vkCtx->device->getAllocator(), pickingData.pickingbuffer.vmaAllocation, &data );

      int32_t id{ -1 };
      std::memcpy( &id, data, sizeof( int32_t ) );
      auto entity = static_cast<entt::entity>( id );
      K_INFO( "ENTITY ID {}", static_cast<uint32_t>( entity ) );
      if ( scene->getEnttRegistry().valid( entity ) && id > -1 )
      {
        core::EventDispatcher* eventDispathcer = core::MainRegistry::getInstance().getEventDispatcher();
        eventDispathcer->dispatchEvent<core::SelectEntityEvent>(
          core::SelectEntityEvent{ entity, core::SelectEntityEventSource::Viewport_Window } );
      }

      vmaUnmapMemory( m_vkCtx->device->getAllocator(), pickingData.pickingbuffer.vmaAllocation );
      *readyToCopyData = false;
    } );
}

auto rendering::PickingModule::createModuleResources() -> void
{
  m_graph->getBlackboard()->addToStorage<PickingModuleData>();

  PickingModuleData& pickingData = m_graph->getBlackboard()->get<PickingModuleData>();

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof( uint32_t );
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  m_vkCtx->device->createBuffer( pickingData.pickingbuffer, bufferInfo, vmaAllocInfo );
  m_vkCtx->device->setName( std::string{ "pickingBuffer" }, pickingData.pickingbuffer.vmaAllocation );

  VkImageCreateInfo pickingColorInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = VK_FORMAT_R32_SINT,
    .extent =
      {
        .width = m_extent.width,
        .height = m_extent.height,
        .depth = 1,
      },
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo pickingColorAllocInfo{ .usage = VMA_MEMORY_USAGE_AUTO };

  pickingData.color = m_graph->createResource( "pickingColor", pickingColorInfo, pickingColorAllocInfo );
}

auto rendering::PickingModule::onMouseClicked( const core::MouseClickedEvent& e ) -> void
{
  core::SceneEventHandler* sceneHandler = core::MainRegistry::getInstance().getSceneManager()->getEventHandler();
  entt::entity currentEntity = sceneHandler->getCurrentEntityId();

  if ( currentEntity != entt::null )
    return;

  int mouseX, mouseY;
  SDL_GetMouseState( &mouseX, &mouseY );

  gui::Viewport* viewport =
    dynamic_cast<gui::Viewport*>( m_imguiRenderer->getImGuiWindows().at( gui::ImGuiWindowName::Viewport ).get() );

  gui::ImGuiProps* props = viewport->getProps();

  VkExtent2D extent = m_vkCtx->swapchain->getSwapchainExtent();

  float localX = ( mouseX - props->x ) / props->width;
  float localY = ( mouseY - props->y ) / props->height;

  if ( localX >= 0.0f && localX <= 1.0f && localY >= 0.0f && localY <= 1.0f )
  {
    m_mouseCoords.x = static_cast<int>( localX * extent.width );
    m_mouseCoords.y = static_cast<int>( localY * extent.height );
    K_INFO( "Mouse coords {} {}", m_mouseCoords.x, m_mouseCoords.y );
  }
}
