#include "renderer/modules/shadowmap_module.hpp"
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "renderer/blackboard.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/vertex.hpp"

rendering::ShadowmapModule::ShadowmapModule( FrameGraph* graph, graphics::VulkanContext* vkCtx, VkExtent2D extent )
    : m_graph{ graph }
    , m_vkCtx{ vkCtx }
    , m_extent{ extent }
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

auto rendering::ShadowmapModule::getShadowmapDescriptor() -> graphics::VulkanDescriptor&
{
  graphics::VulkanDescriptor& descriptor =
    m_graph->getBlackboard()->get<ShadowmapModuleData>().directionalLightDescriptor;
  return descriptor;
}

auto rendering::ShadowmapModule::registerShadowmapPass() -> void
{
  ShadowmapModuleData& shadowmapData = m_graph->getBlackboard()->get<ShadowmapModuleData>();

  VkShaderModule vertex = m_vkCtx->device->createShaderModule( "shadowmap", "vertexMain" );
  VkShaderModule fragment = m_vkCtx->device->createShaderModule( "shadowmap", "fragmentMain" );

  std::vector<VkDescriptorSetLayout> descriptorLayout{ shadowmapData.directionalLightDescriptor.layout };

  graphics::VulkanPipelineSpec defaultPipelineSpec{
    .options = { .cullMode = VK_CULL_MODE_BACK_BIT, .polyMode = VK_POLYGON_MODE_FILL },
    .descriptorLayout = descriptorLayout,
    .colorAttachmentCount = 0u,
    .vertexModule = vertex,
    .fragmentModule = fragment,
    .pushConstantSize = sizeof( resources::MeshPushConstant ),
    .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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

      core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
      core::Scene* scene = sceneManager->getCurrentScene();

      entt::entity directionalLightId{ entt::null };
      auto directionalView = scene->getEnttRegistry().view<core::DirectionalLightComponent>();

      if ( directionalView.size() == 0 )
        return;

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

      // light pov
      vkCmdBindDescriptorSets( cmdBuffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               shadowmapData.shadowmapPipeline.getLayout(),
                               0,
                               1,
                               &shadowmapData.directionalLightDescriptor.set,
                               0,
                               nullptr );

      // texture that gets written to
      vkCmdBindDescriptorSets( cmdBuffer,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               shadowmapData.shadowmapPipeline.getLayout(),
                               1,
                               1,
                               &shadowmapData.directionalLightPovDescriptor.set,
                               0,
                               nullptr );

      directionalView.each( [&]( const entt::entity& entityId, const core::DirectionalLightComponent& dirLight ) {
        directionalLightId = entityId;
      } );

      const core::DirectionalLightComponent& directionalLightComp =
        scene->getRegistry()->getComponent<core::DirectionalLightComponent>( directionalLightId );

      glm::mat4 lightProjection =
        glm::ortho( -10.0f, 10.0f, -10.0f, 10.0f, directionalLightComp.nearPlane, directionalLightComp.farPlane );

      glm::mat4 lightView =
        glm::lookAt( glm::vec3( -2.0f, 4.0f, -1.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

      DirectionalLightUBO ubo{ .projection = lightProjection, .view = lightView };

      uint32_t index = m_vkCtx->swapchain->getCurrentFrameNumber();
      m_vkCtx->device->copyToBuffer( ubo, shadowmapData.directionalLightBuffer.buffers.at( index ) );

      auto view = scene->getEnttRegistry().view<core::MeshComponent, core::TransformComponent>();
      view.each(
        [&]( const entt::entity& entityId, core::MeshComponent& meshComponent, core::TransformComponent& transform ) {
          resources::Mesh* pMesh = meshComponent.pMesh;
          if ( !pMesh )
            return;

          if ( !pMesh->isLoaded() )
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
  // m_graph->getBlackboard()->addToStorage<ShadowmapModuleData>();

  // ShadowmapModuleData& shadowmapData = m_graph->getBlackboard()->get<ShadowmapModuleData>();

  // VkBufferCreateInfo bufferInfo{};
  // bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  // bufferInfo.size = sizeof( DirectionalLightUBO );
  // bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  // VmaAllocationCreateInfo vmaAllocInfo{};
  // vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  // vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  // m_vkCtx->device->createBuffer( shadowmapData.directionalLightBuffer, bufferInfo, vmaAllocInfo, "lightBuffer" );

  //// create the ubo for the light mvp matrix
  // VkDescriptorSetLayoutBinding cameraBufferBinding{};
  // cameraBufferBinding.binding = 0;
  // cameraBufferBinding.descriptorCount = 1;

  // cameraBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // cameraBufferBinding.pImmutableSamplers = nullptr;
  // cameraBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  // VkDescriptorBindingFlags flags = 0;

  // VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
  // bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
  // bindingFlags.bindingCount = 1;
  // bindingFlags.pBindingFlags = &flags;

  // VkDescriptorSetLayoutCreateInfo layoutInfo{};
  // layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  // layoutInfo.bindingCount = 1;
  // layoutInfo.pBindings = &cameraBufferBinding;
  // layoutInfo.pNext = nullptr;
  // layoutInfo.flags = 0;

  // m_vkCtx->device->createDescriptorSetLayout( shadowmapData.directionalLightPovDescriptor.layout, layoutInfo );

  ///////

  // VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  // samplerLayoutBinding.binding = 0;
  // samplerLayoutBinding.descriptorCount = 1;
  // samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  // samplerLayoutBinding.pImmutableSamplers = nullptr;
  // samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  // VkDescriptorSetLayoutCreateInfo layoutInfo{};
  // layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  // layoutInfo.bindingCount = 1;
  // layoutInfo.pBindings = &samplerLayoutBinding;
  // layoutInfo.pNext = VK_NULL_HANDLE;

  // m_vkCtx->device->createDescriptorSetLayout( shadowmapData.directionalLightDescriptor.layout, layoutInfo );

  // VkDescriptorSetAllocateInfo allocInfo{};
  // allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  // allocInfo.descriptorPool = m_vkCtx->globalDescriptorPool;
  // allocInfo.descriptorSetCount = 1;
  // allocInfo.pSetLayouts = &shadowmapData.directionalLightDescriptor.layout;
  // allocInfo.pNext = VK_NULL_HANDLE;

  // m_vkCtx->device->allocateDescriptorSet( shadowmapData.directionalLightDescriptor.set, allocInfo );

  // VkImageCreateInfo shadowmapDepthCreateInfo{
  //     .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
  //     .imageType = VK_IMAGE_TYPE_2D,
  //     .format = VK_FORMAT_D32_SFLOAT,
  //     .extent =
  //         {
  //             .width = extent.width,
  //             .height = extent.height,
  //             .depth = 1,
  //         },
  //     .mipLevels = 1,
  //     .arrayLayers = 1,
  //     .samples = VK_SAMPLE_COUNT_1_BIT,
  //     .tiling = VK_IMAGE_TILING_OPTIMAL,
  //     .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
  //     .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  //     .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
  // };

  // VmaAllocationCreateInfo prepassDepthAllocInfo{ .usage = VMA_MEMORY_USAGE_AUTO };

  // shadowmapData.depth = m_graph->createResource( "shadowmapDepth", shadowmapDepthCreateInfo, prepassDepthAllocInfo );
}

auto rendering::ShadowmapModule::destroyModuleResources() -> void
{
  ShadowmapModuleData& shadowmapData = m_graph->getBlackboard()->get<ShadowmapModuleData>();

  m_vkCtx->device->destroyDescriptorSetLayout( shadowmapData.directionalLightDescriptor.layout );

  m_vkCtx->device->destroyPipelineLayout( shadowmapData.shadowmapPipeline.getLayout() );
  m_vkCtx->device->destroyPipeline( shadowmapData.shadowmapPipeline.getPipeline() );

  m_vkCtx->device->destroyBuffer( shadowmapData.directionalLightBuffer );

  m_vkCtx->device->destroyImageView( shadowmapData.depth->vulkanImage.vkImageView );
  m_vkCtx->device->destroyImage( shadowmapData.depth->vulkanImage.vkImage,
                                 shadowmapData.depth->vulkanImage.vmaAllocation );
}
