#include "renderer/modules/prepass_module.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "renderer/blackboard.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/vertex.hpp"
#include "utilities/tracy_utils/tracy_utils.hpp"

rendering::PrepassModule::PrepassModule( FrameGraph* graph,
                                         graphics::VulkanContext* vkCtx,
                                         VkExtent2D extent,
                                         ModuleDescriptorData descriptorData )
    : m_graph{ graph }
    , m_vkCtx{ vkCtx }
    , m_moduleDescriptorData{ descriptorData }
    , m_extent{ extent }
{
  createModuleResources( extent );
  registerPasses();
}

rendering::PrepassModule::~PrepassModule()
{
  destroyModuleResources();
}

auto rendering::PrepassModule::registerPasses() -> void
{
  registerDepthPrepass();
}

auto rendering::PrepassModule::registerDepthPrepass() -> void
{
  PrepassModuleData& prepassData = m_graph->getBlackboard()->get<PrepassModuleData>();

  VkShaderModule vertex = m_vkCtx->device->createShaderModule( "depth", "vertexMain" );
  VkShaderModule fragment = m_vkCtx->device->createShaderModule( "depth", "fragmentMain" );

  graphics::VulkanPipelineSpec defaultPipelineSpec{
    .type = graphics::PipelineType::geometry,
    .options = { .cullMode = VK_CULL_MODE_BACK_BIT, .polyMode = VK_POLYGON_MODE_FILL },
    .descriptorLayout = m_moduleDescriptorData.descriptorSetLayouts,
    .vertexModule = vertex,
    .fragmentModule = fragment,
    .pushConstantSize = sizeof( resources::MeshPushConstant ),
    .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .colorAttachmentCount = 0u,
    .vertexBindingDescription = resources::Vertex::getBindingDescription(),
    .vertexAttributesDescription = resources::Vertex::getAttributeDescriptions() };

  prepassData.depthPrepassPipeline.create( defaultPipelineSpec, m_vkCtx );

  m_vkCtx->device->destroyShaderModule( vertex );
  m_vkCtx->device->destroyShaderModule( fragment );

  m_graph->addNode(
    std::string{ passId::DepthPrepass },
    []( NodeBuilder& b, Blackboard* blackboard ) {
      PrepassModuleData& prepassData = blackboard->get<PrepassModuleData>();
      b.write( prepassData.depth, FGResourceType::Depth );
    },
    [=]( VkCommandBuffer cmdBuffer ) {
      TracyVkZone( m_vkCtx->tracyContext->getCtx(), cmdBuffer, passId::DepthPrepass );
      if ( m_extent.width == 0 || m_extent.height == 0 )
        return;

      PrepassModuleData& prepassData = m_graph->getBlackboard()->get<PrepassModuleData>();

      prepassData.renderingInfo.depthAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = prepassData.depth->vulkanImage.vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { .depthStencil = { 1.f, 0 } } };

      prepassData.renderingInfo.vkRenderingInfo = VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { { 0, 0 }, m_extent },
        .layerCount = 1,
        .colorAttachmentCount = 0,
        .pColorAttachments = VK_NULL_HANDLE,
        .pDepthAttachment = &prepassData.renderingInfo.depthAttachmentInfo,
      };

      m_vkCtx->swapchain->beginRendering( prepassData.renderingInfo.vkRenderingInfo );
      m_vkCtx->swapchain->setupScissors( cmdBuffer );
      m_vkCtx->swapchain->setupViewport( cmdBuffer );

      prepassData.depthPrepassPipeline.bind( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

      vkCmdBindDescriptorSets( cmdBuffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               prepassData.depthPrepassPipeline.getLayout(),
                               0,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 0 ),
                               0,
                               nullptr );

      core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
      core::Scene* scene = sceneManager->getCurrentScene();

      auto view = scene->getEnttRegistry().view<core::MeshComponent, core::TransformComponent>();
      view.each(
        [&]( const entt::entity& entityId, core::MeshComponent& meshComponent, core::TransformComponent& transform ) {
          if ( !meshComponent.loaded )
            return;

          VkDeviceSize offsets[] = { 0 };

          vkCmdBindVertexBuffers( cmdBuffer, 0, 1, &meshComponent.pMesh->getVertexBufferObject().vkBuffer, offsets );
          vkCmdBindIndexBuffer(
            cmdBuffer, meshComponent.pMesh->getIndicesBufferObject().vkBuffer, 0, VK_INDEX_TYPE_UINT32 );
          for ( auto& submesh : meshComponent.pMesh->getSubmeshes() )
          {
            // this should be expensive, move it somewhere in the mesh or submesh
            auto push = resources::MeshPushConstant{ .modelMatrix = transform.getMatrix(),
                                                     .materialIndex = submesh.materialIndex };

            vkCmdPushConstants( cmdBuffer,
                                prepassData.depthPrepassPipeline.getLayout(),
                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                0,
                                sizeof( resources::MeshPushConstant ),
                                &push );

            vkCmdDrawIndexed( cmdBuffer, submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0 );
          }
        } );

      m_vkCtx->swapchain->endRendering();
    } );
}

auto rendering::PrepassModule::createModuleResources( VkExtent2D extent ) -> void
{
  Blackboard* blackboard = m_graph->getBlackboard();
  blackboard->addToStorage<PrepassModuleData>();

  PrepassModuleData& prepassData = blackboard->get<PrepassModuleData>();
  VkImageCreateInfo prepassDepthCreateInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = VK_FORMAT_D32_SFLOAT,
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
    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo prepassDepthAllocInfo{ .usage = VMA_MEMORY_USAGE_AUTO };

  prepassData.depth = m_graph->createResource( "prepassDepth", prepassDepthCreateInfo, prepassDepthAllocInfo );
}

auto rendering::PrepassModule::destroyModuleResources() -> void
{
  PrepassModuleData& prepassData = m_graph->getBlackboard()->get<PrepassModuleData>();

  m_vkCtx->device->destroyImageView( prepassData.depth->vulkanImage.vkImageView );
  m_vkCtx->device->destroyImage( prepassData.depth->vulkanImage.vkImage, prepassData.depth->vulkanImage.vmaAllocation );

  m_vkCtx->device->destroyPipelineLayout( prepassData.depthPrepassPipeline.getLayout() );
  m_vkCtx->device->destroyPipeline( prepassData.depthPrepassPipeline.getPipeline() );
}

auto rendering::PrepassModule::setExtent( VkExtent2D extent ) -> void
{
  m_extent = extent;
}

auto rendering::PrepassModule::recreate( VkExtent2D extent ) -> void
{
  destroyModuleResources();
  createModuleResources( extent );
  registerPasses();
}