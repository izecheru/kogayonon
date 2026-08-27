#include "renderer/modules/geometry_module.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "graphics/vulkan_context.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/frame_graph.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/vertex.hpp"
#include "utilities/tracy_utils/tracy_utils.hpp"
// components for view filtering in scene
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"

rendering::GeometryModule::GeometryModule( FrameGraph* graph,
                                           graphics::VulkanContext* ctx,
                                           VkExtent2D extent,
                                           ModuleDescriptorData descriptorData,
                                           glm::vec4 clearColor )
    : m_vkCtx{ ctx }
    , m_graph{ graph }
    , m_wireframe{ false }
    , m_wireframeInit{ false }
    , m_moduleDescriptorData{ descriptorData }
    , m_clearColor{ clearColor }
    , m_extent{ extent }
{
  createModuleResources( extent );
  registerPasses();
}

rendering::GeometryModule::GeometryModule( FrameGraph* graph,
                                           graphics::VulkanContext* ctx,
                                           VkExtent2D extent,
                                           ModuleDescriptorData descriptorData )
    : GeometryModule( graph, ctx, extent, descriptorData, glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f } )
{
}

rendering::GeometryModule::~GeometryModule()
{
  Blackboard* blackboard = m_graph->getBlackboard();
  GeometryModuleData& geometryModuleData = blackboard->get<GeometryModuleData>();

  m_vkCtx->device->destroyPipelineLayout( geometryModuleData.basePipeline.getLayout() );
  m_vkCtx->device->destroyPipeline( geometryModuleData.basePipeline.getPipeline() );

  m_vkCtx->device->destroyPipelineLayout( geometryModuleData.depthPipeline.getLayout() );
  m_vkCtx->device->destroyPipeline( geometryModuleData.depthPipeline.getPipeline() );

  m_vkCtx->device->destroyPipelineLayout( geometryModuleData.wireframePipeline.getLayout() );
  m_vkCtx->device->destroyPipeline( geometryModuleData.wireframePipeline.getPipeline() );

  m_vkCtx->device->destroyImages(
    { { geometryModuleData.color->vulkanImage.vkImage, geometryModuleData.color->vulkanImage.vmaAllocation },
      { geometryModuleData.depth->vulkanImage.vkImage, geometryModuleData.depth->vulkanImage.vmaAllocation } } );

  m_vkCtx->device->destroyImageViews(
    { geometryModuleData.color->vulkanImage.vkImageView, geometryModuleData.depth->vulkanImage.vkImageView } );
}

auto rendering::GeometryModule::setClearColor( glm::vec4 clearColor ) -> void
{
  m_clearColor = clearColor;
}

void rendering::GeometryModule::registerPasses()
{
  registerBaseGeometryPass();
  registerDepthPrepass();
  if ( m_wireframe )
  {
    registerWireframePass();
  }
}

auto rendering::GeometryModule::registerWireframePass() -> void
{
  Blackboard* blackboard = m_graph->getBlackboard();

  GeometryModuleData& geometryInfo{ blackboard->get<GeometryModuleData>() };

  VkShaderModule vertex = m_vkCtx->device->createShaderModule( "basic_shader", "vertexMain" );
  VkShaderModule fragment = m_vkCtx->device->createShaderModule( "basic_shader", "fragmentMain" );

  graphics::VulkanPipelineSpec wireframeSpec{
    .type = graphics::PipelineType::geometry,
    .options = { .cullMode = VK_CULL_MODE_NONE, .polyMode = VK_POLYGON_MODE_LINE, .lineWidth = 0.1f },
    .descriptorLayout = m_moduleDescriptorData.descriptorSetLayouts,
    .colorAttachmentFormat = { m_vkCtx->swapchain->getSwapchainImageFormat() },
    .vertexModule = vertex,
    .fragmentModule = fragment,
    .pushConstantSize = sizeof( resources::MeshPushConstant ),
    .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .colorAttachmentCount = 1,
    .vertexBindingDescription = resources::Vertex::getBindingDescription(),
    .vertexAttributesDescription = resources::Vertex::getAttributeDescriptions() };

  geometryInfo.wireframePipeline.create( wireframeSpec, m_vkCtx );

  m_vkCtx->device->destroyShaderModule( vertex );
  m_vkCtx->device->destroyShaderModule( fragment );

  m_graph->addNode(
    std::string{ passId::Wireframe },
    []( NodeBuilder& b, Blackboard* blackboard ) {
      GeometryModuleData& geometryData = blackboard->get<GeometryModuleData>();

      b.write( geometryData.color, rendering::FGResourceType::Color );
      b.read( geometryData.depth, rendering::FGResourceType::Depth );
    },
    [=]( VkCommandBuffer buffer ) {
      Blackboard* blackboard = m_graph->getBlackboard();
      GeometryModuleData& geometryModule = blackboard->get<GeometryModuleData>();

      graphics::VulkanPipeline& wireframePipeline = geometryModule.wireframePipeline;

      geometryModule.renderingInfo.colorAttachmentInfo = VkRenderingAttachmentInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = geometryModule.color->vulkanImage.vkImageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { { m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w } } };

      geometryModule.renderingInfo.depthAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = geometryModule.depth->vulkanImage.vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { .depthStencil = { 1.f, 0 } } };

      geometryModule.renderingInfo.vkRenderingInfo = VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { { 0, 0 }, m_extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &geometryModule.renderingInfo.colorAttachmentInfo,
        .pDepthAttachment = &geometryModule.renderingInfo.depthAttachmentInfo,
      };

      m_vkCtx->swapchain->beginRendering( geometryModule.renderingInfo.vkRenderingInfo );
      m_vkCtx->swapchain->setupScissors( buffer );
      m_vkCtx->swapchain->setupViewport( buffer );

      wireframePipeline.bind( buffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

      // camera descriptor, this is tripple buffered to ensure operations are not overwritten by cpu/ gpu
      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               wireframePipeline.getLayout(),
                               0,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 0 ),
                               0,
                               nullptr );

      // this is the bindless texture set
      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               wireframePipeline.getLayout(),
                               1,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 1 ),
                               0,
                               nullptr );

      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               wireframePipeline.getLayout(),
                               2,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 2 ),
                               0,
                               nullptr );

      core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
      core::Scene* scene = sceneManager->getCurrentScene();

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
          // this should be expensive, move it somewhere in the mesh or submesh
          auto push =
            resources::MeshPushConstant{ .modelMatrix = transform.getMatrix(), .materialIndex = submesh.materialIndex };

          vkCmdPushConstants( buffer,
                              wireframePipeline.getLayout(),
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                              0,
                              sizeof( resources::MeshPushConstant ),
                              &push );

          vkCmdDrawIndexed( buffer, submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0 );
        }
      } );

      m_vkCtx->swapchain->endRendering();
    } );
}

auto rendering::GeometryModule::registerBaseGeometryPass() -> void
{
  Blackboard* blackboard = m_graph->getBlackboard();

  GeometryModuleData& geometryInfo = blackboard->get<GeometryModuleData>();

  VkShaderModule vertex = m_vkCtx->device->createShaderModule( "basic_shader", "vertexMain" );
  VkShaderModule fragment = m_vkCtx->device->createShaderModule( "basic_shader", "fragmentMain" );

  graphics::VulkanPipelineSpec defaultPipelineSpec{
    .type = graphics::PipelineType::geometry,
    .options = { .cullMode = VK_CULL_MODE_BACK_BIT, .polyMode = VK_POLYGON_MODE_FILL },
    .descriptorLayout = m_moduleDescriptorData.descriptorSetLayouts,
    .colorAttachmentFormat = { m_vkCtx->swapchain->getSwapchainImageFormat() },
    .vertexModule = vertex,
    .fragmentModule = fragment,
    .pushConstantSize = sizeof( resources::MeshPushConstant ),
    .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    .colorAttachmentCount = 1,
    .vertexBindingDescription = resources::Vertex::getBindingDescription(),
    .vertexAttributesDescription = resources::Vertex::getAttributeDescriptions(),
  };

  geometryInfo.basePipeline.create( defaultPipelineSpec, m_vkCtx );

  m_vkCtx->device->destroyShaderModule( vertex );
  m_vkCtx->device->destroyShaderModule( fragment );

  m_graph->addNode(
    std::string{ passId::Geometry },
    []( NodeBuilder& b, Blackboard* blackboard ) {
      GeometryModuleData& geometryModule{ blackboard->get<GeometryModuleData>() };
      b.write( geometryModule.color, rendering::FGResourceType::Color );
      b.read( geometryModule.depth, rendering::FGResourceType::Depth );
    },
    [=]( VkCommandBuffer buffer ) {
      // TracyVkZone( m_vkCtx->tracyContext->getCtx(), buffer, passId::Geometry );
      Blackboard* blackboard{ m_graph->getBlackboard() };
      GeometryModuleData& geometryModule{ blackboard->get<GeometryModuleData>() };

      graphics::VulkanPipeline& geometryPipeline = geometryModule.basePipeline;

      geometryModule.renderingInfo.colorAttachmentInfo = VkRenderingAttachmentInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = geometryModule.color->vulkanImage.vkImageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { { m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w } } };

      // TODO set flags according to acces type, for example the loadOp should be Clear if this node
      // writes to this attachment, and Load if it just reads
      geometryModule.renderingInfo.depthAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = geometryModule.depth->vulkanImage.vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { .depthStencil = { 1.f, 0 } } };

      geometryModule.renderingInfo.vkRenderingInfo = VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { { 0, 0 }, m_extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &geometryModule.renderingInfo.colorAttachmentInfo,
        .pDepthAttachment = &geometryModule.renderingInfo.depthAttachmentInfo,
      };

      m_vkCtx->swapchain->beginRendering( geometryModule.renderingInfo.vkRenderingInfo );
      m_vkCtx->swapchain->setupScissors( buffer );
      m_vkCtx->swapchain->setupViewport( buffer );

      geometryPipeline.bind( buffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               geometryPipeline.getLayout(),
                               0,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 0 ),
                               0,
                               nullptr );

      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               geometryPipeline.getLayout(),
                               1,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 1 ),
                               0,
                               nullptr );

      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               geometryPipeline.getLayout(),
                               2,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 2 ),
                               0,
                               nullptr );

      core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
      core::Scene* scene = sceneManager->getCurrentScene();

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
          // this should be expensive, move it somewhere in the mesh or submesh
          auto push =
            resources::MeshPushConstant{ .modelMatrix = transform.getMatrix(), .materialIndex = submesh.materialIndex };

          vkCmdPushConstants( buffer,
                              geometryPipeline.getLayout(),
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                              0,
                              sizeof( resources::MeshPushConstant ),
                              &push );

          vkCmdDrawIndexed( buffer, submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0 );
        }
      } );

      m_vkCtx->swapchain->endRendering();
    } );
}

auto rendering::GeometryModule::registerDepthPrepass() -> void
{
  Blackboard* blackboard = m_graph->getBlackboard();

  GeometryModuleData& geometryInfo = blackboard->get<GeometryModuleData>();

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

  geometryInfo.depthPipeline.create( defaultPipelineSpec, m_vkCtx );

  m_vkCtx->device->destroyShaderModule( vertex );
  m_vkCtx->device->destroyShaderModule( fragment );

  m_graph->addNode(
    std::string{ passId::DepthPrepass },
    []( NodeBuilder& b, Blackboard* blackboard ) {
      GeometryModuleData& geometryModule{ blackboard->get<GeometryModuleData>() };
      b.write( geometryModule.depth, rendering::FGResourceType::Depth );
    },
    [=]( VkCommandBuffer buffer ) {
      TracyVkZone( m_vkCtx->tracyContext->getCtx(), buffer, passId::DepthPrepass );
      Blackboard* blackboard{ m_graph->getBlackboard() };
      GeometryModuleData& geometryModule{ blackboard->get<GeometryModuleData>() };

      graphics::VulkanPipeline& depthPipeline = geometryModule.depthPipeline;

      geometryModule.renderingInfo.depthAttachmentInfo =
        VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                   .imageView = geometryModule.depth->vulkanImage.vkImageView,
                                   .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                   .clearValue = { .depthStencil = { 1.f, 0 } } };

      geometryModule.renderingInfo.vkRenderingInfo = VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { { 0, 0 }, m_extent },
        .layerCount = 1,
        .colorAttachmentCount = 0,
        .pColorAttachments = VK_NULL_HANDLE,
        .pDepthAttachment = &geometryModule.renderingInfo.depthAttachmentInfo,
      };

      m_vkCtx->swapchain->beginRendering( geometryModule.renderingInfo.vkRenderingInfo );
      m_vkCtx->swapchain->setupScissors( buffer );
      m_vkCtx->swapchain->setupViewport( buffer );

      depthPipeline.bind( buffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

      // camera descriptor, this is tripple buffered to ensure operations are not overwritten by cpu/ gpu
      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               depthPipeline.getLayout(),
                               0,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 0 ),
                               0,
                               nullptr );

      // this is the bindless texture set
      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               depthPipeline.getLayout(),
                               1,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 1 ),
                               0,
                               nullptr );

      vkCmdBindDescriptorSets( buffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               depthPipeline.getLayout(),
                               2,
                               1,
                               &m_moduleDescriptorData.descriptorSets.at( 2 ),
                               0,
                               nullptr );

      core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
      core::Scene* scene = sceneManager->getCurrentScene();

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
          // this should be expensive, move it somewhere in the mesh or submesh
          auto push =
            resources::MeshPushConstant{ .modelMatrix = transform.getMatrix(), .materialIndex = submesh.materialIndex };

          vkCmdPushConstants( buffer,
                              depthPipeline.getLayout(),
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                              0,
                              sizeof( resources::MeshPushConstant ),
                              &push );

          vkCmdDrawIndexed( buffer, submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0 );
        }
      } );

      m_vkCtx->swapchain->endRendering();
    } );
}

auto rendering::GeometryModule::createModuleResources( VkExtent2D extent ) -> void
{
  Blackboard* blackboard = m_graph->getBlackboard();

  blackboard->addToStorage<GeometryModuleData>();
  GeometryModuleData& geometryModuleData = blackboard->get<GeometryModuleData>();

  VkImageCreateInfo geometryColorInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = m_vkCtx->swapchain->getSwapchainImageFormat(),
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
    .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo geometryColorAllocInfo{ .usage = VMA_MEMORY_USAGE_AUTO };

  geometryModuleData.color = m_graph->createResource( "geometryColor", geometryColorInfo, geometryColorAllocInfo );

  VkImageCreateInfo geometryDepthInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = VK_FORMAT_D32_SFLOAT,
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
    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  };

  VmaAllocationCreateInfo geometryDepthAllocInfo{ .usage = VMA_MEMORY_USAGE_AUTO };

  geometryModuleData.depth = m_graph->createResource( "geometryDepth", geometryDepthInfo, geometryDepthAllocInfo );
}

auto rendering::GeometryModule::enableWireframe() -> void
{
  m_wireframe = true;
  if ( !m_wireframeInit )
  {
    registerWireframePass();
    m_wireframeInit = true;
  }
  // cull base pass
  for ( std::unique_ptr<Node>& node : m_graph->getContainer().nodes )
  {
    if ( node->name == passId::Wireframe )
    {
      node->culled = false;
    }
    if ( node->name == passId::Geometry )
    {
      node->culled = true;
    }
  }
  m_graph->recompile();
}

auto rendering::GeometryModule::disableWireframe() -> void
{
  m_wireframe = false;
  // cull base pass
  for ( std::unique_ptr<Node>& node : m_graph->getContainer().nodes )
  {
    if ( node->name == passId::Wireframe )
    {
      node->culled = true;
    }
    if ( node->name == passId::Geometry )
    {
      node->culled = false;
    }
  }
  m_graph->recompile();
}
