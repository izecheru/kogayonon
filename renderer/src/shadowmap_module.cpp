#include "renderer/modules/shadowmap_module.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "renderer/blackboard.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/vertex.hpp"

rendering::ShadowmapModule::ShadowmapModule( FrameGraph* graph,
                                             graphics::VulkanContext* vkCtx,
                                             ModuleDescriptorData descriptorData,
                                             VkExtent2D extent )
    : m_graph{ graph }
    , m_vkCtx{ vkCtx }
    , m_extent{ extent }
    , m_moduleDescriptorData{ descriptorData }
{
  createModuleResources( extent );
  registerPasses();
}

rendering::ShadowmapModule::~ShadowmapModule()
{
  destroyModuleResources();
}

auto rendering::ShadowmapModule::registerPasses() -> void
{
  registerShadowmapPass();
}

auto rendering::ShadowmapModule::registerShadowmapPass() -> void
{
  ShadowmapModuleData& shadowmapData = m_graph->getBlackboard()->get<ShadowmapModuleData>();

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

  shadowmapData.shadowmapPipeline.create( defaultPipelineSpec, m_vkCtx );

  m_vkCtx->device->destroyShaderModule( vertex );
  m_vkCtx->device->destroyShaderModule( fragment );

  m_graph->addNode(
    std::string{ passId::ShadowmapPass },
    []( NodeBuilder& b, Blackboard* blackboard ) {
      ShadowmapModuleData& shadowmapData = blackboard->get<ShadowmapModuleData>();
      b.write( shadowmapData.depth, FGResourceType::Depth );
    },
    [=]( VkCommandBuffer cmdBuffer ) {
      ShadowmapModuleData& shadowmapData = m_graph->getBlackboard()->get<ShadowmapModuleData>();

      shadowmapData.renderingInfo.depthAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = shadowmapData.depth->vulkanImage.vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { .depthStencil = { 1.f, 0 } } };

      shadowmapData.renderingInfo.vkRenderingInfo = VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { { 0, 0 }, m_extent },
        .layerCount = 1,
        .colorAttachmentCount = 0,
        .pColorAttachments = VK_NULL_HANDLE,
        .pDepthAttachment = &shadowmapData.renderingInfo.depthAttachmentInfo,
      };

      m_vkCtx->swapchain->beginRendering( shadowmapData.renderingInfo.vkRenderingInfo );
      m_vkCtx->swapchain->setupScissors( cmdBuffer );
      m_vkCtx->swapchain->setupViewport( cmdBuffer );

      shadowmapData.shadowmapPipeline.bind( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

      vkCmdBindDescriptorSets( cmdBuffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               shadowmapData.shadowmapPipeline.getLayout(),
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

            resources::MeshPushConstant push{ .modelMatrix = transform.getMatrix() };

            vkCmdPushConstants( cmdBuffer,
                                shadowmapData.shadowmapPipeline.getLayout(),
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

auto rendering::ShadowmapModule::setExtent( VkExtent2D extent ) -> void
{
  m_extent = extent;
}

auto rendering::ShadowmapModule::recreate( VkExtent2D extent ) -> void
{
  destroyModuleResources();
  createModuleResources( extent );
}

auto rendering::ShadowmapModule::createModuleResources( VkExtent2D extent ) -> void
{
  ShadowmapModuleData& shadowmapData = m_graph->getBlackboard()->get<ShadowmapModuleData>();

  VkImageCreateInfo shadowmapDepthCreateInfo{
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

  shadowmapData.depth = m_graph->createResource( "shadowmapDepth", shadowmapDepthCreateInfo, prepassDepthAllocInfo );
}

auto rendering::ShadowmapModule::destroyModuleResources() -> void
{
  ShadowmapModuleData& shadowmapData = m_graph->getBlackboard()->get<ShadowmapModuleData>();
  m_vkCtx->device->destroyDescriptorSetLayout( shadowmapData.directionalLightDescriptor.layout );

  m_vkCtx->device->destroyPipelineLayout( shadowmapData.shadowmapPipeline.getLayout() );
  m_vkCtx->device->destroyPipeline( shadowmapData.shadowmapPipeline.getPipeline() );

  m_vkCtx->device->destroyImageView( shadowmapData.depth->vulkanImage.vkImageView );
  m_vkCtx->device->destroyImage( shadowmapData.depth->vulkanImage.vkImage,
                                 shadowmapData.depth->vulkanImage.vmaAllocation );
}
