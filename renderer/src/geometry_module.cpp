#include "core/ecs/components/cubemap_component.hpp"
#include "renderer/modules/geometry_module.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "graphics/vulkan_context.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/prepass_module.hpp"
#include "renderer/modules/shadowmap_module.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/vertex.hpp"
#include "tracy/TracyVulkan.hpp"
// components for view filtering in scene
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "graphics/vulkan_memory_allocator.hpp"

rendering::GeometryModule::GeometryModule( FrameGraph* graph,
                                           graphics::VulkanContext* ctx,
                                           VkExtent2D extent,
                                           graphics::FrameInFlightVulkanDescriptor* cameraDescriptor,
                                           glm::vec4 clearColor )
    : m_vkCtx{ ctx }
    , m_graph{ graph }
    , m_wireframe{ false }
    , m_wireframeInit{ false }
    , m_cameraDescriptor{ cameraDescriptor }
    , m_clearColor{ clearColor }
    , m_extent{ extent }
{
    registerPasses();
}

rendering::GeometryModule::GeometryModule( FrameGraph* graph,
                                           graphics::VulkanContext* ctx,
                                           VkExtent2D extent,
                                           graphics::FrameInFlightVulkanDescriptor* cameraDescriptor )
    : GeometryModule( graph, ctx, extent, cameraDescriptor, glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f } )
{
}

rendering::GeometryModule::~GeometryModule()
{
    destroyModuleResources();
}

auto rendering::GeometryModule::setClearColor( glm::vec4 clearColor ) -> void
{
    m_clearColor = clearColor;
}

void rendering::GeometryModule::registerPasses()
{
    createModuleResources( m_extent );

    registerBaseGeometryPass();
    if ( m_wireframe )
    {
        registerWireframePass();
    }
}

auto rendering::GeometryModule::recreate( VkExtent2D extent ) -> void
{
    m_extent = extent;

    destroyModuleResources();
    registerPasses();
}

auto rendering::GeometryModule::destroyModuleResources() -> void
{
    Blackboard* blackboard = m_graph->getBlackboard();
    GeometryModuleData& geometryModuleData = blackboard->get<GeometryModuleData>();

    graphics::VulkanMemoryAllocator* allocator = m_vkCtx->device->getMemAllocator();

    m_vkCtx->device->destroyPipelineLayout( geometryModuleData.basePipeline.getLayout() );
    m_vkCtx->device->destroyPipeline( geometryModuleData.basePipeline.getPipeline() );

    m_vkCtx->device->destroyPipelineLayout( geometryModuleData.wireframePipeline.getLayout() );
    m_vkCtx->device->destroyPipeline( geometryModuleData.wireframePipeline.getPipeline() );

    m_vkCtx->device->destroyImageView( geometryModuleData.color->vulkanImage.vkImageView );

    allocator->destroyImage( geometryModuleData.color->vulkanImage.vkImage,
                             geometryModuleData.color->vulkanImage.vmaAllocation );

    blackboard->removeFromStorage<GeometryModuleData>();
}

auto rendering::GeometryModule::registerWireframePass() -> void
{
    Blackboard* blackboard = m_graph->getBlackboard();

    GeometryModuleData& geometryInfo = blackboard->get<GeometryModuleData>();

    VkShaderModule vertex = m_vkCtx->device->createShaderModule( "basic_shader", "vertexMain" );
    VkShaderModule fragment = m_vkCtx->device->createShaderModule( "basic_shader", "fragmentMain" );

    core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
    std::vector<VkDescriptorSetLayout> descriptorLayout{ m_cameraDescriptor->layout,
                                                         assetManager->getBindlessTexturesDescriptorLayout() };

    graphics::VulkanPipelineSpec wireframeSpec{
        .options = { .cullMode = VK_CULL_MODE_NONE, .polyMode = VK_POLYGON_MODE_LINE, .lineWidth = 0.1f },
        .descriptorLayout = descriptorLayout,
        .colorAttachmentCount = 1,
        .colorAttachmentFormat = { m_vkCtx->swapchain->getSwapchainImageFormat() },
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .vertexModule = vertex,
        .fragmentModule = fragment,
        .pushConstantSize = sizeof( resources::MeshPushConstant ),
        .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };

    geometryInfo.wireframePipeline.create( wireframeSpec, m_vkCtx );

    m_vkCtx->device->destroyShaderModule( vertex );
    m_vkCtx->device->destroyShaderModule( fragment );

    m_graph->addNode(
        std::string{ passId::Wireframe },
        []( NodeBuilder& b, Blackboard* blackboard ) {
            GeometryModuleData& geometryData = blackboard->get<GeometryModuleData>();
            PrepassModuleData& prepassData = blackboard->get<PrepassModuleData>();

            b.write( geometryData.color, rendering::FGResourceType::Color );
            b.read( prepassData.depth, rendering::FGResourceType::Depth );
        },
        [=]( VkCommandBuffer cmdBuffer ) {
            TracyVkZone( m_vkCtx->tracyContext->getCtx(), cmdBuffer, passId::Wireframe );

            core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
            core::Scene* scene = sceneManager->getCurrentScene();

            if ( !scene )
            {
                return;
            }

            GeometryModuleData& geometryData = m_graph->getBlackboard()->get<GeometryModuleData>();
            PrepassModuleData& prepassData = m_graph->getBlackboard()->get<PrepassModuleData>();

            graphics::VulkanPipeline& wireframePipeline = geometryData.wireframePipeline;

            geometryData.renderingInfo.colorAttachmentInfo = VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = geometryData.color->vulkanImage.vkImageView,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            };

            geometryData.renderingInfo.depthAttachmentInfo = VkRenderingAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = prepassData.depth->vulkanImage.vkImageView, // reuse from prepass depth
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = { .depthStencil = { 1.f, 0 } } };

            geometryData.renderingInfo.vkRenderingInfo = VkRenderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = { { 0, 0 }, m_extent },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &geometryData.renderingInfo.colorAttachmentInfo,
                .pDepthAttachment = &geometryData.renderingInfo.depthAttachmentInfo,
            };

            m_vkCtx->swapchain->beginRendering( geometryData.renderingInfo.vkRenderingInfo );
            m_vkCtx->swapchain->setupScissors( cmdBuffer );
            m_vkCtx->swapchain->setupViewport( cmdBuffer );

            wireframePipeline.bind( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

            core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
            uint32_t currentFrame = m_vkCtx->swapchain->getCurrentFrameNumber();

            vkCmdBindDescriptorSets( cmdBuffer,
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     wireframePipeline.getLayout(),
                                     0,
                                     1,
                                     &m_cameraDescriptor->set[currentFrame],
                                     0,
                                     nullptr );

            vkCmdBindDescriptorSets( cmdBuffer,
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     wireframePipeline.getLayout(),
                                     1,
                                     1,
                                     &assetManager->getBindlessDescriptorSet(),
                                     0,
                                     nullptr );

            VkBuffer commandsBuffer = prepassData.indirectCommandsBuffer.buffers.at( currentFrame ).vkBuffer;
            graphics::VulkanMemoryAllocator* allocator = m_vkCtx->device->getMemAllocator();

            graphics::VulkanBuffer& entityParams = prepassData.entitiesParamsBuffer.buffers.at( currentFrame );

            resources::FramePushConstant framePushConst{ .vertexBufferAddress =
                                                             allocator->getGlobalVertexBuffer().deviceAdress,
                                                         .entityParamsBufferAddress = entityParams.deviceAdress };

            vkCmdPushConstants( cmdBuffer,
                                geometryData.basePipeline.getLayout(),
                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                0,
                                sizeof( resources::FramePushConstant ),
                                &framePushConst );

            vkCmdBindIndexBuffer( cmdBuffer, allocator->getGlobalIndexBuffer().vkBuffer, 0, VK_INDEX_TYPE_UINT32 );
            vkCmdDrawIndexedIndirect( cmdBuffer,
                                      commandsBuffer,
                                      0,
                                      prepassData.drawCount[currentFrame],
                                      sizeof( VkDrawIndexedIndirectCommand ) );

            m_vkCtx->swapchain->endRendering();
        } );
}

auto rendering::GeometryModule::registerBaseGeometryPass() -> void
{
    Blackboard* blackboard = m_graph->getBlackboard();
    GeometryModuleData& geometryInfo = blackboard->get<GeometryModuleData>();

    VkShaderModule vertex = m_vkCtx->device->createShaderModule( "basic_shader", "vertexMain" );
    VkShaderModule fragment = m_vkCtx->device->createShaderModule( "basic_shader", "fragmentMain" );

    core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
    std::vector<VkDescriptorSetLayout> descriptorLayout{ m_cameraDescriptor->layout,
                                                         assetManager->getBindlessTexturesDescriptorLayout() };

    graphics::VulkanPipelineSpec defaultPipelineSpec{
        .options = { .cullMode = VK_CULL_MODE_BACK_BIT, .polyMode = VK_POLYGON_MODE_FILL },
        .descriptorLayout = descriptorLayout,
        .colorAttachmentCount = 1,
        .colorAttachmentFormat = { m_vkCtx->swapchain->getSwapchainImageFormat() },
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .vertexModule = vertex,
        .fragmentModule = fragment,
        .pushConstantSize = sizeof( resources::FramePushConstant ),
        .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    geometryInfo.basePipeline.create( defaultPipelineSpec, m_vkCtx );
    graphics::VulkanMemoryAllocator* allocator = m_vkCtx->device->getMemAllocator();
    allocator->setVulkanDebugName( VK_OBJECT_TYPE_PIPELINE,
                                   reinterpret_cast<uint64_t>( geometryInfo.basePipeline.getPipeline() ),
                                   "geometryBasePipeline" );

    m_vkCtx->device->destroyShaderModule( vertex );
    m_vkCtx->device->destroyShaderModule( fragment );

    m_graph->addNode(
        std::string{ passId::Geometry },
        []( NodeBuilder& b, Blackboard* blackboard ) {
            GeometryModuleData& geometryData = blackboard->get<GeometryModuleData>();
            PrepassModuleData& prepassData = blackboard->get<PrepassModuleData>();

            b.write( geometryData.color, rendering::FGResourceType::Color );
            b.read( prepassData.depth, rendering::FGResourceType::Depth );
        },
        [=]( VkCommandBuffer cmdBuffer ) {
            TracyVkZone( m_vkCtx->tracyContext->getCtx(), cmdBuffer, passId::Geometry );

            VkDebugUtilsLabelEXT markerInfo{};
            markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            markerInfo.pLabelName = passId::Geometry;

            GeometryModuleData& geometryData = m_graph->getBlackboard()->get<GeometryModuleData>();
            PrepassModuleData& prepassData = m_graph->getBlackboard()->get<PrepassModuleData>();

            if ( m_extent.width == 0 || m_extent.height == 0 )
                return;

            uint32_t currentFrame = m_vkCtx->swapchain->getCurrentFrameNumber();

            if ( prepassData.drawCount[currentFrame] == 0 )
            {
                return;
            }

            graphics::VulkanPipeline& geometryPipeline = geometryData.basePipeline;

            geometryData.renderingInfo.colorAttachmentInfo =
                VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                           .imageView = geometryData.color->vulkanImage.vkImageView,
                                           .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                           .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           .storeOp = VK_ATTACHMENT_STORE_OP_STORE };

            geometryData.renderingInfo.depthAttachmentInfo =
                VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                           .imageView = prepassData.depth->vulkanImage.vkImageView, // reuse
                                           .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                           .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                           .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                           .clearValue = { .depthStencil = { 1.f, 0 } } };

            geometryData.renderingInfo.vkRenderingInfo = VkRenderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = { { 0, 0 }, m_extent },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &geometryData.renderingInfo.colorAttachmentInfo,
                .pDepthAttachment = &geometryData.renderingInfo.depthAttachmentInfo,
            };

            m_vkCtx->swapchain->beginRendering( geometryData.renderingInfo.vkRenderingInfo );
            m_vkCtx->swapchain->setupScissors( cmdBuffer );
            m_vkCtx->swapchain->setupViewport( cmdBuffer );

            geometryPipeline.bind( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

            core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

            vkCmdBindDescriptorSets( cmdBuffer,
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     geometryPipeline.getLayout(),
                                     0,
                                     1,
                                     &m_cameraDescriptor->set[currentFrame],
                                     0,
                                     nullptr );

            vkCmdBindDescriptorSets( cmdBuffer,
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     geometryPipeline.getLayout(),
                                     1,
                                     1,
                                     &assetManager->getBindlessDescriptorSet(),
                                     0,
                                     nullptr );

            graphics::VulkanMemoryAllocator* allocator = m_vkCtx->device->getMemAllocator();

            graphics::VulkanBuffer& entityParams = prepassData.entitiesParamsBuffer.buffers.at( currentFrame );

            resources::FramePushConstant framePushConst{
                .vertexBufferAddress = allocator->getGlobalVertexBuffer().deviceAdress,
                .materialBufferAddress = assetManager->getMaterialBufferAddress(),
                .entityParamsBufferAddress = entityParams.deviceAdress };

            vkCmdPushConstants( cmdBuffer,
                                geometryData.basePipeline.getLayout(),
                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                0,
                                sizeof( resources::FramePushConstant ),
                                &framePushConst );

            graphics::VulkanBuffer& commandsBuffer = prepassData.indirectCommandsBuffer.buffers.at( currentFrame );

            vkCmdBindIndexBuffer( cmdBuffer, allocator->getGlobalIndexBuffer().vkBuffer, 0, VK_INDEX_TYPE_UINT32 );

            vkCmdDrawIndexedIndirect( cmdBuffer,
                                      commandsBuffer.vkBuffer,
                                      0,
                                      prepassData.drawCount[currentFrame],
                                      sizeof( VkDrawIndexedIndirectCommand ) );

            m_vkCtx->swapchain->endRendering();
        } );
}

auto rendering::GeometryModule::createModuleResources( VkExtent2D extent ) -> void
{
    Blackboard* blackboard = m_graph->getBlackboard();
    blackboard->addToStorage<GeometryModuleData>();

    GeometryModuleData& geometryData = blackboard->get<GeometryModuleData>();

    VkImageCreateInfo skyboxColorInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = m_vkCtx->swapchain->getSwapchainImageFormat(),
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
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo skyboxColorAllocInfo{ .usage = VMA_MEMORY_USAGE_AUTO };

    geometryData.color = m_graph->createResource( "geometryColor", skyboxColorInfo, skyboxColorAllocInfo );
}

auto rendering::GeometryModule::enableWireframe() -> void
{
    m_wireframe = true;

    if ( !m_wireframeInit )
    {
        registerWireframePass();
        m_wireframeInit = true;
    }

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

auto rendering::GeometryModule::setExtent( VkExtent2D extent ) -> void
{
    m_extent = extent;
}
