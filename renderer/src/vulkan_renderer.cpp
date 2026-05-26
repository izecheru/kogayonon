#include "renderer/vulkan_renderer.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "graphics/vulkan_context.hpp"
#include "gui/imgui_windows/viewport.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "physics/jolt_physics.hpp"
#include "resources/mesh_push_constant.hpp"
#include "utilities/utils/utils.hpp"
#include <SDL2/SDL.h>

rendering::VulkanRenderer::VulkanRenderer( graphics::VulkanContext* pCtx, SDL_Window* window )
    : m_pVkContext{ pCtx }
    , m_wnd{ window }
    , m_shaderCompiler{ m_pVkContext->device->getLogicalDevice() }
    , m_mouseCoord{ -1, -1 }
    , m_selectedEntity{ entt::null }
{
  auto& pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<core::MouseClickedEvent, &VulkanRenderer::onMouseClicked>( *this );
  pEventDispatcher->addHandler<core::SelectEntityEvent, &VulkanRenderer::onEntitySelect>( *this );

  createCameraDescriptorSetLayout();
  createCameraBuffers();
#ifdef PICKING_ENABLED
  createPickingBuffers();
#endif
  createCameraDescriptorSet();
  initViewports();
  initImgui();

  auto& assetManager = core::AssetManager::getInstance();
  auto shadersPath = std::filesystem::current_path() / "engine_resources" / " shaders ";

  auto basicGeometryVertex = m_shaderCompiler.createShaderModule( "basic_shader", "vertexMain" );
  auto basicGeometryFragment = m_shaderCompiler.createShaderModule( "basic_shader", "fragmentMain" );

  assert( basicGeometryVertex != VK_NULL_HANDLE );
  assert( basicGeometryFragment != VK_NULL_HANDLE );

  auto defaultPipelineSpec =
    graphics::VulkanPipelineSpec{ .type = graphics::PipelineType::GEOMETRY_BASIC,
                                  .options = { .cullMode = VK_CULL_MODE_BACK_BIT, .polyMode = VK_POLYGON_MODE_FILL },
                                  .descriptorLayout = { m_cameraDescriptor.layout,
                                                        assetManager.getBindlessDescriptorLayout(),
                                                        assetManager.getMaterialsDescriptorLayout() },
                                  .colorAttachmentFormat = m_pVkContext->swapchain->getSwapchainImageFormat(),
                                  .vertexModule = basicGeometryVertex,
                                  .fragmentModule = basicGeometryFragment,
                                  .pushConstantSize = sizeof( resources::MeshPushConstant ),
                                  .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                  .vertexBindingDescription = resources::Vertex::getBindingDescription(),
                                  .vertexAttributesDescription = resources::Vertex::getAttributeDescriptions() };

  createPipeline( defaultPipelineSpec );

#ifdef PICKING_ENABLED
  auto pickingPipelineSpec = defaultPipelineSpec;

  auto pickingVertexModule = m_shaderCompiler.createShaderModule( "picking_vertex" );
  auto pickingFragmentModule = m_shaderCompiler.createShaderModule( "picking_fragment" );

  pickingPipelineSpec.vertexModule = pickingVertexModule;
  pickingPipelineSpec.fragmentModule = pickingFragmentModule;
  pickingPipelineSpec.pushConstantSize = sizeof( resources::EntityPickingPushConstant );
  pickingPipelineSpec.type = graphics::PipelineType::PICKING;
  pickingPipelineSpec.descriptorLayout = { m_cameraDescriptor.layout };
  pickingPipelineSpec.colorAttachmentFormat = VK_FORMAT_R32_SINT;
  pickingPipelineSpec.options.depthTestEnable = VK_FALSE;
  pickingPipelineSpec.options.depthWriteEnable = VK_FALSE;

  createPipeline( pickingPipelineSpec );
  vkDestroyShaderModule( m_pVkContext->device->getLogicalDevice(), pickingVertexModule, nullptr );
  vkDestroyShaderModule( m_pVkContext->device->getLogicalDevice(), pickingFragmentModule, nullptr );
#endif

  // Destroy the shader modules after pipeline creation
  vkDestroyShaderModule( m_pVkContext->device->getLogicalDevice(), basicGeometryVertex, nullptr );
  vkDestroyShaderModule( m_pVkContext->device->getLogicalDevice(), basicGeometryFragment, nullptr );
}

void rendering::VulkanRenderer::onEntitySelect( core::SelectEntityEvent& e )
{
  if ( e.getEntityId() == m_selectedEntity )
    return;

  if ( e.getEventSource() == core::SelectEntityEventSource::None && e.getEntityId() == entt::null )
  {
    m_selectedEntity = entt::null;
    return;
  }

  m_selectedEntity = e.getEntityId();
}

rendering::VulkanRenderer::~VulkanRenderer()
{
}

void rendering::VulkanRenderer::render()
{
  auto scene = core::SceneManager::getCurrentScene().lock();

  if ( !m_pVkContext->swapchain->isRendering() )
    return;

  m_pVkContext->swapchain->waitForFences();
  m_pVkContext->swapchain->resetFences();

#ifdef PICKING_ENABLED
  static bool copyImage{ false };

  if ( copyImage )
  {
    void* data{ nullptr };
    vmaMapMemory( m_pVkContext->memoryAllocator->getAllocator(),
                  m_pickingBuffer.buffers.at( m_pVkContext->swapchain->getCurrentFrameIndex() ).allocation,
                  &data );

    int32_t id{ -1 };
    std::memcpy( &id, data, sizeof( int32_t ) );
    auto entity = static_cast<entt::entity>( id );
    if ( scene->getEnttRegistry().valid( entity ) )
    {
      auto& pEventDispathcer = core::MainRegistry::getInstance().getEventDispatcher();
      m_selectedEntity = entity;
      pEventDispathcer->dispatchEvent<core::SelectEntityEvent>(
        core::SelectEntityEvent{ entity, core::SelectEntityEventSource::Viewport_Window } );
    }
    else
    {
      KOGAYONON_WARN( "Got {} from picking", id );
    }

    vmaUnmapMemory( m_pVkContext->memoryAllocator->getAllocator(),
                    m_pickingBuffer.buffers.at( m_pVkContext->swapchain->getCurrentFrameIndex() ).allocation );
    m_mouseCoord = { -1, -1 };
    copyImage = false;
  }
#endif

  updateCameraBuffer();

  m_pVkContext->swapchain->aquireNextImage();
  auto& cmd = m_pVkContext->swapchain->getCurrentCommandBuffer();
  m_pVkContext->swapchain->beginCommandBuffer();

  geometryPass( cmd );

  VkImageMemoryBarrier swapchainToColor{
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcAccessMask = 0,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .image = m_pVkContext->swapchain->getCurrentFrame().image,
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

#ifdef PICKING_ENABLED
  if ( m_mouseCoord.x >= 0 && m_mouseCoord.y >= 0 && m_selectedEntity == entt::null )
  {
    pickingPass( cmd );
    copyImage = true;
  }
#endif

  imguiPass( cmd );
}

void rendering::VulkanRenderer::createPipeline( const graphics::VulkanPipelineSpec& spec )
{
  m_pipelines.emplace( spec.type, graphics::VulkanPipeline{ spec, m_pVkContext } );
}

auto rendering::VulkanRenderer::getViewport() -> VulkanViewport&
{
  return m_viewport;
}

void rendering::VulkanRenderer::createPickingViewport( uint32_t width, uint32_t height )
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R32_SINT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo imageAllocInfo{};
  imageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;

  m_pVkContext->memoryAllocator->createImage(
    m_pickingViewport.image, imageInfo, imageAllocInfo, m_pickingViewport.allocation );

  m_pickingViewport.imageView =
    createImageView( m_pVkContext, m_pickingViewport.image, VK_FORMAT_R32_SINT, VK_IMAGE_ASPECT_COLOR_BIT );

  auto depthFormat = findDepthFormat( &m_pVkContext->device->getPhysicalDevice() );

  VkImageCreateInfo depthImageInfo{};
  depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
  depthImageInfo.extent.width = m_pVkContext->swapchain->getSwapchainExtent().width;
  depthImageInfo.extent.height = m_pVkContext->swapchain->getSwapchainExtent().height;
  depthImageInfo.extent.depth = 1;
  depthImageInfo.mipLevels = 1;
  depthImageInfo.arrayLayers = 1;
  depthImageInfo.format = depthFormat;
  depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo depthAllocInfo{};
  depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  m_pVkContext->memoryAllocator->createImage(
    m_pickingViewport.depthImage, depthImageInfo, depthAllocInfo, m_pickingViewport.depthAllocation );

  m_pickingViewport.depthView =
    createImageView( m_pVkContext, m_pickingViewport.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT );
}

void rendering::VulkanRenderer::createViewport( uint32_t width, uint32_t height )
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = m_pVkContext->swapchain->getSwapchainImageFormat();
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo imageAllocInfo{};
  imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  m_pVkContext->memoryAllocator->createImage( m_viewport.image, imageInfo, imageAllocInfo, m_viewport.allocation );

  m_viewport.imageView = createImageView(
    m_pVkContext, m_viewport.image, m_pVkContext->swapchain->getSwapchainImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT );

  auto depthFormat = findDepthFormat( &m_pVkContext->device->getPhysicalDevice() );

  VkImageCreateInfo depthImageInfo{};
  depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
  depthImageInfo.extent.width = m_pVkContext->swapchain->getSwapchainExtent().width;
  depthImageInfo.extent.height = m_pVkContext->swapchain->getSwapchainExtent().height;
  depthImageInfo.extent.depth = 1;
  depthImageInfo.mipLevels = 1;
  depthImageInfo.arrayLayers = 1;
  depthImageInfo.format = depthFormat;
  depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo depthAllocInfo{};
  depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  m_pVkContext->memoryAllocator->createImage(
    m_viewport.depthImage, depthImageInfo, depthAllocInfo, m_viewport.depthAllocation );

  m_viewport.depthView = createImageView( m_pVkContext, m_viewport.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT );
}

void rendering::VulkanRenderer::createCameraBuffers()
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof( core::CameraUbo );
  bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  m_pVkContext->memoryAllocator->createBuffers( m_cameraBuffers, bufferInfo, vmaAllocInfo );
}

void rendering::VulkanRenderer::updateCameraBuffer()
{
  auto scene = core::SceneManager::getCurrentScene().lock();
  auto view = scene->getEnttRegistry().view<core::PerspectiveCameraComponent>();
  view.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComp ) {
    if ( cameraComp.isUsed )
    {

      if ( cameraComp.props.changed )
      {
        cameraComp.updateUbo();
      }

      vmaCopyMemoryToAllocation(
        m_pVkContext->memoryAllocator->getAllocator(),
        &cameraComp.ubo,
        m_cameraBuffers.buffers.at( m_pVkContext->swapchain->getCurrentFrameIndex() ).allocation,
        0,
        sizeof( core::CameraUbo ) );
    }
  } );
}

void rendering::VulkanRenderer::createCameraDescriptorSet()
{
  std::vector<VkDescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT, m_cameraDescriptor.layout );

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = *m_pVkContext->globalDescriptorPool;
  allocInfo.descriptorSetCount = std::size( layouts );
  allocInfo.pSetLayouts = layouts.data();
  allocInfo.pNext = nullptr;

  VK_CALL(
    vkAllocateDescriptorSets( m_pVkContext->device->getLogicalDevice(), &allocInfo, m_cameraDescriptor.set.data() ) );

  for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
  {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_cameraBuffers.buffers.at( i ).vkBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof( core::CameraUbo );

    auto uniformBufferDescriptor = VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = m_cameraDescriptor.set.at( i ),
      .dstBinding = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = &bufferInfo,
    };

    VkWriteDescriptorSet descriptorWrites = uniformBufferDescriptor;
    vkUpdateDescriptorSets( m_pVkContext->device->getLogicalDevice(), 1, &descriptorWrites, 0, nullptr );
  }
}

void rendering::VulkanRenderer::createCameraDescriptorSetLayout()
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

  VK_CALL( vkCreateDescriptorSetLayout(
    m_pVkContext->device->getLogicalDevice(), &layoutInfo, nullptr, &m_cameraDescriptor.layout ) );
}

void rendering::VulkanRenderer::initImgui()
{
  m_pImguiRenderer =
    std::make_shared<gui::VulkanImguiRenderer>( m_wnd, m_pVkContext->device.get(), m_pVkContext->swapchain.get() );
  m_pImguiRenderer->setViewport( m_viewport.imageView );
}

void rendering::VulkanRenderer::geometryPass( VkCommandBuffer& cmd )
{
  auto scene = core::SceneManager::getCurrentScene().lock();
  auto& assetManager = core::AssetManager::getInstance();

  const auto& view = scene->getEnttRegistry().view<core::MeshComponent, core::TransformComponent>();
  const auto& rigidView =
    scene->getEnttRegistry().view<core::MeshComponent, core::TransformComponent, core::RigidbodyComponent>();

  rigidView.each( [&]( const entt::entity& entityId,
                       core::MeshComponent& meshComponent,
                       core::TransformComponent& transform,
                       core::RigidbodyComponent& rigidBody ) {
    auto& jolt = core::MainRegistry::getInstance().getJoltPhysics();
    auto& bodyInterface = jolt->getPhysicsSystem().GetBodyInterface();

    if ( jolt->isRunning() )
    {
      auto pos = bodyInterface.GetCenterOfMassPosition( rigidBody.body );
      auto rotation = bodyInterface.GetRotation( rigidBody.body );

      transform.translation = { pos.GetX(), pos.GetY(), pos.GetZ() };
      auto euler = rotation.GetEulerAngles();
      transform.rotation = { glm::degrees( euler.GetX() ), glm::degrees( euler.GetY() ), glm::degrees( euler.GetZ() ) };
      transform.computeMatrix();
    }
  } );

  VkImageMemoryBarrier textureToColor{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                       .srcAccessMask = 0,
                                       .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                       .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                       .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                       .image = m_viewport.image,
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
                                     .image = m_viewport.depthImage,
                                     .subresourceRange = {
                                       .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                       .levelCount = 1,
                                       .layerCount = 1,
                                     } };

  VkRenderingAttachmentInfo depthAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                             .imageView = m_viewport.depthView,
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

  VkRenderingAttachmentInfo colorAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                             .imageView = m_viewport.imageView,
                                             .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                             .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                             .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                             .clearValue = { { 0.2471f, 0.2471f, 0.2471f, 1.0f } } };

  VkRenderingInfo renderingInfo{
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = { { 0, 0 }, m_pVkContext->swapchain->getSwapchainExtent() },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &colorAttachment,
    .pDepthAttachment = &depthAttachment,
  };

  m_pVkContext->swapchain->beginRendering( renderingInfo );
  m_pVkContext->swapchain->setupScissors( cmd );
  m_pVkContext->swapchain->setupViewport( cmd );

  VkDeviceSize offsets[] = { 0 };

  auto& pipeline = m_pipelines.at( graphics::PipelineType ::GEOMETRY_BASIC );
  pipeline.bind( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS );

  // camera descriptor, this is tripple buffered to ensure operations are not overwritten by cpu/ gpu
  vkCmdBindDescriptorSets( cmd,
                           VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipeline.getLayout(),
                           DescriptorSetNum::SET_0,
                           1,
                           &m_cameraDescriptor.set.at( m_pVkContext->swapchain->getCurrentFrameIndex() ),
                           0,
                           nullptr );

  // this is the bindless texture set
  vkCmdBindDescriptorSets( cmd,
                           VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipeline.getLayout(),
                           DescriptorSetNum::SET_1,
                           1,
                           &assetManager.getBindlessDescriptorSet(),
                           0,
                           nullptr );

  // Since view.each calls a lambda for each entity in the view and does not act like a traditional for
  // we just return if the mesh is not loaded for obvious reasons
  view.each(
    [&]( const entt::entity& entityId, core::MeshComponent& meshComponent, core::TransformComponent& transform ) {
      if ( !meshComponent.loaded )
        return;

      vkCmdBindDescriptorSets( cmd,
                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                               pipeline.getLayout(),
                               DescriptorSetNum::SET_2,
                               1,
                               &assetManager.getMaterialsDescriptorSet(),
                               0,
                               nullptr );

      vkCmdBindVertexBuffers( cmd, 0, 1, &meshComponent.pMesh->getVertexBufferObject().vkBuffer, offsets );
      vkCmdBindIndexBuffer( cmd, meshComponent.pMesh->getIndicesBufferObject().vkBuffer, 0, VK_INDEX_TYPE_UINT32 );
      for ( auto& submesh : meshComponent.pMesh->getSubmeshes() )
      {
        // this should be expensive, move it somewhere in the mesh or submesh
        auto push =
          resources::MeshPushConstant{ .modelMatrix = transform.getMatrix(), .materialIndex = submesh.materialIndex };

        vkCmdPushConstants( cmd,
                            pipeline.getLayout(),
                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                            0,
                            sizeof( resources::MeshPushConstant ),
                            &push );

        vkCmdDrawIndexed( cmd, submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0 );
      }
    } );

  m_pVkContext->swapchain->endRendering();

  VkImageMemoryBarrier textureToShader{
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
    .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    .image = m_viewport.image,
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
}

void rendering::VulkanRenderer::pickingPass( VkCommandBuffer& cmd )
{
  auto& viewport = m_pImguiRenderer->getImGuiWindows().at( gui::ImGuiWindowName::Viewport );
  auto props = viewport->getProps();

  if ( m_mouseCoord.x < 0 || m_mouseCoord.y < 0 || m_mouseCoord.x > props->width || m_mouseCoord.y > props->height )
    return;

  auto scene = core::SceneManager::getCurrentScene().lock();
  auto& assetManager = core::AssetManager::getInstance();

  const auto& view = scene->getEnttRegistry().view<core::MeshComponent, core::TransformComponent>();

  VkImageMemoryBarrier textureToColor{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                       .srcAccessMask = 0,
                                       .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                       .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                       .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                       .image = m_pickingViewport.image,
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
                                     .image = m_pickingViewport.depthImage,
                                     .subresourceRange = {
                                       .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                       .levelCount = 1,
                                       .layerCount = 1,
                                     } };

  VkRenderingAttachmentInfo depthAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                             .imageView = m_pickingViewport.depthView,
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

  VkRenderingAttachmentInfo colorAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                             .imageView = m_pickingViewport.imageView,
                                             .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                             .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                             .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                             .clearValue = { .color = { .int32 = { -1, -1, -1, -1 } } } };

  VkRenderingInfo renderingInfo{
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = { { 0, 0 }, m_pVkContext->swapchain->getSwapchainExtent() },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &colorAttachment,
    .pDepthAttachment = &depthAttachment,
  };

  m_pVkContext->swapchain->beginRendering( renderingInfo );
  m_pVkContext->swapchain->setupScissors( cmd );
  m_pVkContext->swapchain->setupViewport( cmd );

  VkDeviceSize offsets[] = { 0 };

  auto& pipeline = m_pipelines.at( graphics::PipelineType::PICKING );
  pipeline.bind( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS );

  // camera descriptor, this is tripple buffered to ensure operations are not overwritten by cpu/ gpu
  vkCmdBindDescriptorSets( cmd,
                           VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipeline.getLayout(),
                           0, // set = 0 camera
                           1,
                           &m_cameraDescriptor.set.at( m_pVkContext->swapchain->getCurrentFrameIndex() ),
                           0,
                           nullptr );

  // Since view.each calls a lambda for each entity in the view and does not act like a traditional for
  // we just return if the mesh is not loaded for obvious reasons
  view.each(
    [&]( const entt::entity& entityId, core::MeshComponent& meshComponent, core::TransformComponent& transform ) {
      if ( !meshComponent.loaded )
        return;

      vkCmdBindVertexBuffers( cmd, 0, 1, &meshComponent.pMesh->getVertexBufferObject().vkBuffer, offsets );
      vkCmdBindIndexBuffer( cmd, meshComponent.pMesh->getIndicesBufferObject().vkBuffer, 0, VK_INDEX_TYPE_UINT32 );
      for ( auto& submesh : meshComponent.pMesh->getSubmeshes() )
      {
        // this should be expensive, move it somewhere in the mesh or submesh
        auto push = resources::EntityPickingPushConstant{ .modelMatrix = transform.getMatrix(),
                                                          .entityId = static_cast<int>( entityId ) };

        vkCmdPushConstants( cmd,
                            pipeline.getLayout(),
                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                            0,
                            sizeof( resources::EntityPickingPushConstant ),
                            &push );

        vkCmdDrawIndexed( cmd, submesh.indexCount, 1, submesh.indexOffset, submesh.vertexOffset, 0 );
      }
    } );

  m_pVkContext->swapchain->endRendering();

  VkImageMemoryBarrier textureToTransfer{
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
    .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    .image = m_pickingViewport.image,
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
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0,
                        0,
                        nullptr,
                        0,
                        nullptr,
                        1,
                        &textureToTransfer );

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = { m_mouseCoord.x, m_mouseCoord.y, 0 };
  region.imageExtent = { 1, 1, 1 };

  vkCmdCopyImageToBuffer( cmd,
                          m_pickingViewport.image,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          m_pickingBuffer.buffers.at( m_pVkContext->swapchain->getCurrentFrameIndex() ).vkBuffer,
                          1,
                          &region );
}

void rendering::VulkanRenderer::imguiPass( VkCommandBuffer& cmd )
{
  VkRenderingAttachmentInfo imguiColorAttachment{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                                  .imageView = m_pVkContext->swapchain->getCurrentFrame().imageView,
                                                  .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                  .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                  .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                  .clearValue = { { 0.f, 0.f, 0.f, 1.f } } };

  VkRenderingInfo imguiRenderingInfo{
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea = { { 0, 0 }, m_pVkContext->swapchain->getSwapchainExtent() },
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &imguiColorAttachment,
    .pDepthAttachment = nullptr,
  };

  m_pVkContext->swapchain->beginRendering( imguiRenderingInfo );
  m_pImguiRenderer->render();
  m_pImguiRenderer->present( cmd );
  m_pVkContext->swapchain->endRendering();
}

void rendering::VulkanRenderer::onMouseClicked( core::MouseClickedEvent& e )
{
#ifdef PICKING_ENABLED
  if ( m_selectedEntity != entt::null )
    return;

  int mouseX, mouseY;
  SDL_GetMouseState( &mouseX, &mouseY );

  auto& viewport = m_pImguiRenderer->getImGuiWindows().at( gui::ImGuiWindowName::Viewport );

  auto props = viewport->getProps();

  auto extent = m_pVkContext->swapchain->getSwapchainExtent();

  float localX = ( mouseX - props->x ) / props->width;
  float localY = ( mouseY - props->y ) / props->height;

  if ( localX >= 0.0f && localX <= 1.0f && localY >= 0.0f && localY <= 1.0f )
  {
    m_mouseCoord.x = static_cast<int>( localX * extent.width );
    m_mouseCoord.y = static_cast<int>( localY * extent.height );
    KOGAYONON_INFO( "mouse coords for picking {} {}", m_mouseCoord.x, m_mouseCoord.y );
  }
#endif
}

void rendering::VulkanRenderer::createPickingBuffers()
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof( uint32_t );
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

  m_pVkContext->memoryAllocator->createBuffers( m_pickingBuffer, bufferInfo, vmaAllocInfo );
}

void rendering::VulkanRenderer::initViewports()
{
  auto& extent = m_pVkContext->swapchain->getSwapchainExtent();
  createViewport( extent.width, extent.height );
#ifdef PICKING_ENABLED
  createPickingViewport( extent.width, extent.height );
#endif
}
