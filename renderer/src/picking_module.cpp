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
#include "renderer/modules/prepass_module.hpp"
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
    , m_pickRequested{ false }
    , m_readyToCopy{ false }
    , m_mouseCoords{ -1, -1 }
    , m_imguiRenderer{ imguiRenderer }
    , m_moduleDescriptorData{ descriptorData }
    , m_lastFrameIndex{ -1 }
{
  createModuleResources( extent );
  registerPasses();
}

rendering::PickingModule::~PickingModule()
{
  destroyModuleResources();
}

auto rendering::PickingModule::registerPasses() -> void
{
  registerPickingPass();
  registerPickingReadbackPass();
  registerPickingEntityReadPass();
}

auto rendering::PickingModule::setCoords( glm::ivec2 coords ) -> void
{
  if ( m_pickRequested == true || m_readyToCopy == true )
    return;

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
      PrepassModuleData& prepassData = blackboard->get<PrepassModuleData>();

      b.write( pickingData.color, FGResourceType::Color );
      b.read( prepassData.depth, FGResourceType::Depth );
    },
    [=, coords = &m_mouseCoords, readyToCopy = &m_readyToCopy, pickRequested = &m_pickRequested](
      VkCommandBuffer buffer ) {
      if ( ( coords->x == -1 && coords->y == -1 ) || *readyToCopy == true )
      {
        return;
      }

      *pickRequested = true;

      Blackboard* blackboard = m_graph->getBlackboard();
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();
      PrepassModuleData& prepassData = blackboard->get<PrepassModuleData>();

      pickingData.renderingInfo.colorAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = pickingData.color->vulkanImage.vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { { -1.0f, -1.0f, -1.0f, -1.0f } } };

      pickingData.renderingInfo.depthAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = prepassData.depth->vulkanImage.vkImageView,
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
          resources::EntityPickingPushConstant push{ .modelMatrix = transform.getMatrix(),
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
     lastFrameIndex = &m_lastFrameIndex,
     coords = &m_mouseCoords,
     pickRequested = &m_pickRequested,
     readyToCopy = &m_readyToCopy]( VkCommandBuffer buffer ) {
      Blackboard* blackboard = m_graph->getBlackboard();
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();

      if ( !*pickRequested || *readyToCopy )
        return;

      uint32_t frameNumber = m_vkCtx->swapchain->getCurrentFrameNumber();
      m_vkCtx->device->copyImageToBuffer( pickingData.color->vulkanImage.vkImage,
                                          buffer,
                                          pickingData.pickingBuffer.buffers.at( frameNumber ).vkBuffer,
                                          { coords->x, coords->y, 0 },
                                          { 1, 1, 1 },
                                          false );

      *lastFrameIndex = frameNumber;
      *readyToCopy = true;
      *pickRequested = false;
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
    [=,
     lastFrameIndex = &m_lastFrameIndex,
     coords = &m_mouseCoords,
     pickRequested = &m_pickRequested,
     readyToCopy = &m_readyToCopy]( VkCommandBuffer buffer ) {
      if ( *lastFrameIndex == -1 )
      {
        *lastFrameIndex = m_vkCtx->swapchain->getCurrentFrameNumber();
        return;
      }

      if ( !*readyToCopy )
        return;

      if ( *lastFrameIndex == m_vkCtx->swapchain->getCurrentFrameNumber() )
        return;

      Blackboard* blackboard = m_graph->getBlackboard();
      PickingModuleData& pickingData = blackboard->get<PickingModuleData>();
      core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
      core::Scene* scene = sceneManager->getCurrentScene();

      int32_t id{ -1 };
      m_vkCtx->device->copyBufferData( id, pickingData.pickingBuffer.buffers.at( *lastFrameIndex ), sizeof( int32_t ) );
      K_INFO( "id {}", id );

      if ( id != -1 )
      {
        entt::entity entity = static_cast<entt::entity>( id );
        entt::entity currentEntity =
          core::MainRegistry::getInstance().getSceneManager()->getEventHandler()->getCurrentEntityId();
        if ( scene->getRegistry()->isValid( entity ) && currentEntity == entt::null )
        {
          core::EventDispatcher* eventDispathcer = core::MainRegistry::getInstance().getEventDispatcher();
          eventDispathcer->dispatchEvent<core::SelectEntityEvent>(
            core::SelectEntityEvent{ entity, core::SelectEntityEventSource::Viewport_Window } );
        }
      }

      int32_t clearValue{ -1 };
      m_vkCtx->device->copyDataToBuffer(
        clearValue, pickingData.pickingBuffer.buffers.at( *lastFrameIndex ), sizeof( int32_t ) );

      *lastFrameIndex = m_vkCtx->swapchain->getCurrentFrameNumber();
      coords->x = -1;
      coords->y = -1;
      *readyToCopy = false;
    } );
}

auto rendering::PickingModule::createModuleResources( VkExtent2D extent ) -> void
{
  m_graph->getBlackboard()->addToStorage<PickingModuleData>();

  PickingModuleData& pickingData = m_graph->getBlackboard()->get<PickingModuleData>();

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof( uint32_t );
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  m_vkCtx->device->createBuffer( pickingData.pickingBuffer, bufferInfo, vmaAllocInfo, "pickingBuffer" );

  VkImageCreateInfo pickingColorInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = VK_FORMAT_R32_SINT,
    .extent =
      {
        .width = extent.width,
        .height = extent.height,
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

auto rendering::PickingModule::destroyModuleResources() -> void
{
  PickingModuleData& pickingData = m_graph->getBlackboard()->get<PickingModuleData>();

  m_vkCtx->device->destroyBuffer( pickingData.pickingBuffer );
  m_vkCtx->device->destroyImageView( pickingData.color->vulkanImage.vkImageView );
  m_vkCtx->device->destroyImage( pickingData.color->vulkanImage.vkImage, pickingData.color->vulkanImage.vmaAllocation );
  m_vkCtx->device->destroyPipelineLayout( pickingData.pickingPipeline.getLayout() );
  m_vkCtx->device->destroyPipeline( pickingData.pickingPipeline.getPipeline() );
}
