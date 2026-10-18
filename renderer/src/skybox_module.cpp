#include "renderer/modules/skybox_module.hpp"
#include "renderer/modules/geometry_module.hpp"
#include "core/ecs/components/cubemap_component.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "graphics/vulkan_context.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/frame_graph.hpp"
#include "renderer/modules/prepass_module.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/vertex.hpp"
#include "tracy/TracyVulkan.hpp"
// components for view filtering in scene
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "graphics/vulkan_memory_allocator.hpp"

rendering::SkyboxModule::SkyboxModule( FrameGraph* graph,
                                       graphics::VulkanContext* ctx,
                                       VkExtent2D extent,
                                       graphics::FrameInFlightVulkanDescriptor* cameraDescriptor,
                                       glm::vec4 clearColor )
    : m_graph{ graph }
    , m_vkCtx{ ctx }
    , m_extent{ extent }
    , m_cameraDescriptor{ cameraDescriptor }
{
    createModuleResources( extent );
    registerPasses();
}

rendering::SkyboxModule::SkyboxModule( FrameGraph* graph,
                                       graphics::VulkanContext* ctx,
                                       VkExtent2D extent,
                                       graphics::FrameInFlightVulkanDescriptor* cameraDescriptor )
    : SkyboxModule{ graph, ctx, extent, cameraDescriptor, glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f } }
{
}

rendering::SkyboxModule::~SkyboxModule()
{
    destroyModuleResources();
}

auto rendering::SkyboxModule::setExtent( VkExtent2D extent ) -> void
{
    m_extent = extent;
}

auto rendering::SkyboxModule::registerPasses() -> void
{
    registerSkyboxPass();
}

auto rendering::SkyboxModule::createModuleResources( VkExtent2D extent ) -> void
{
    Blackboard* blackboard = m_graph->getBlackboard();

    blackboard->addToStorage<SkyboxModuleData>();
    SkyboxModuleData& skyboxModuleData = blackboard->get<SkyboxModuleData>();
}

auto rendering::SkyboxModule::destroyModuleResources() -> void
{
    Blackboard* blackboard = m_graph->getBlackboard();
    SkyboxModuleData& skyboxModuleData = blackboard->get<SkyboxModuleData>();

    graphics::VulkanMemoryAllocator* allocator = m_vkCtx->device->getMemAllocator();

    m_vkCtx->device->destroyPipelineLayout( skyboxModuleData.skyboxPipeline.getLayout() );
    m_vkCtx->device->destroyPipeline( skyboxModuleData.skyboxPipeline.getPipeline() );
}

auto rendering::SkyboxModule::recreate( VkExtent2D extent ) -> void
{
    m_extent = extent;
    destroyModuleResources();
    createModuleResources( extent );
    registerPasses();
}

auto rendering::SkyboxModule::registerSkyboxPass() -> void
{
    Blackboard* blackboard = m_graph->getBlackboard();

    SkyboxModuleData& geometryInfo = blackboard->get<SkyboxModuleData>();

    VkShaderModule vertex = m_vkCtx->device->createShaderModule( "skybox", "vertexMain" );
    VkShaderModule fragment = m_vkCtx->device->createShaderModule( "skybox", "fragmentMain" );

    core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

    std::vector<VkDescriptorSetLayout> descriptorLayout{ m_cameraDescriptor->layout,
                                                         assetManager->getBindlessCubemapDescriptorLayout() };

    graphics::VulkanPipelineSpec skyboxSpec{
        .options = { .cullMode = VK_CULL_MODE_NONE, .polyMode = VK_POLYGON_MODE_FILL, .lineWidth = 0.1f },
        .descriptorLayout = descriptorLayout,
        .colorAttachmentCount = 1,
        .colorAttachmentFormat = { m_vkCtx->swapchain->getSwapchainImageFormat() },
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .vertexModule = vertex,
        .fragmentModule = fragment,
        .pushConstantSize = sizeof( resources::FramePushConstant ),
        .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };

    geometryInfo.skyboxPipeline.create( skyboxSpec, m_vkCtx );

    m_vkCtx->device->destroyShaderModule( vertex );
    m_vkCtx->device->destroyShaderModule( fragment );

    m_graph->addNode(
        std::string{ passId::SkyboxPass },
        []( NodeBuilder& b, Blackboard* blackboard ) {
            GeometryModuleData& geometryData = blackboard->get<GeometryModuleData>();
            PrepassModuleData& prepassData = blackboard->get<PrepassModuleData>();

            b.write( geometryData.color, rendering::FGResourceType::Color );
            b.read( prepassData.depth, rendering::FGResourceType::Depth );
        },
        [=]( VkCommandBuffer cmdBuffer ) {
            TracyVkZone( m_vkCtx->tracyContext->getCtx(), cmdBuffer, passId::SkyboxPass );

            core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
            core::Scene* scene = sceneManager->getCurrentScene();

            if ( !scene )
            {
                return;
            }

            // search for a used cubemap entity
            auto view = scene->getEnttRegistry().view<core::CubemapComponent>();

            core::CubemapComponent* comp{ nullptr };

            view.each( [&]( const entt::entity& entityId, core::CubemapComponent& cubemapComp ) {
                if ( cubemapComp.isUsed )
                {
                    comp = &cubemapComp;
                }
            } );

            // early return if we don't have a valid cubemap component texture used
            if ( !comp )
                return;

            SkyboxModuleData& skyboxData = m_graph->getBlackboard()->get<SkyboxModuleData>();
            GeometryModuleData& geometryData = m_graph->getBlackboard()->get<GeometryModuleData>();
            PrepassModuleData& prepassData = m_graph->getBlackboard()->get<PrepassModuleData>();

            graphics::VulkanPipeline& skyboxPipeline = skyboxData.skyboxPipeline;

            skyboxData.renderingInfo.colorAttachmentInfo =
                VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                           .imageView = geometryData.color->vulkanImage.vkImageView,
                                           .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                           .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD, // reuse
                                           .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                           .clearValue = { { 0.0f, 0.0f, 0.0f, 0.0f } } };

            skyboxData.renderingInfo.depthAttachmentInfo =
                VkRenderingAttachmentInfo{ .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                           .imageView = prepassData.depth->vulkanImage.vkImageView,
                                           .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                           .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD, // reuse the depth
                                           .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                           .clearValue = { .depthStencil = { 1.f, 0 } } };

            skyboxData.renderingInfo.vkRenderingInfo = VkRenderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = { { 0, 0 }, m_extent },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &skyboxData.renderingInfo.colorAttachmentInfo,
                .pDepthAttachment = &skyboxData.renderingInfo.depthAttachmentInfo,
            };

            m_vkCtx->swapchain->beginRendering( skyboxData.renderingInfo.vkRenderingInfo );
            m_vkCtx->swapchain->setupScissors( cmdBuffer );
            m_vkCtx->swapchain->setupViewport( cmdBuffer );

            skyboxPipeline.bind( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

            core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
            uint32_t currentFrame = m_vkCtx->swapchain->getCurrentFrameNumber();

            vkCmdBindDescriptorSets( cmdBuffer,
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     skyboxPipeline.getLayout(),
                                     0,
                                     1,
                                     &m_cameraDescriptor->set[currentFrame],
                                     0,
                                     nullptr );

            vkCmdBindDescriptorSets( cmdBuffer,
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     skyboxPipeline.getLayout(),
                                     1,
                                     1,
                                     &assetManager->getBindlessCubemapDescriptorSet(),
                                     0,
                                     nullptr );

            VkBuffer commandsBuffer = prepassData.indirectCommandsBuffer.buffers.at( currentFrame ).vkBuffer;

            struct DrawParams
            {
                int cubemapTextureIndex{ -1 };
            } drawParams;

            drawParams.cubemapTextureIndex = comp->cubemapTexture->textureIndex;

            vkCmdPushConstants( cmdBuffer,
                                skyboxData.skyboxPipeline.getLayout(),
                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                0,
                                sizeof( DrawParams ),
                                &drawParams );

            vkCmdDraw( cmdBuffer, 36, 1, 0, 0 );

            m_vkCtx->swapchain->endRendering();
        } );
}
