#include "renderer/modules/prepass_module.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "graphics/vulkan_memory_allocator.hpp"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "renderer/blackboard.hpp"
#include "resources/mesh_push_constant.hpp"
#include "resources/vertex.hpp"
#include "tracy/TracyVulkan.hpp"

rendering::PrepassModule::PrepassModule( FrameGraph* graph,
                                         graphics::VulkanContext* vkCtx,
                                         VkExtent2D extent,
                                         graphics::FrameInFlightVulkanDescriptor* cameraDescriptor )
    : m_graph{ graph }
    , m_vkCtx{ vkCtx }
    , m_cameraDescriptor{ cameraDescriptor }
    , m_extent{ extent }
{
    registerPasses();
}

rendering::PrepassModule::~PrepassModule()
{
    destroyModuleResources();
}

auto rendering::PrepassModule::registerPasses() -> void
{
    createModuleResources( m_extent );
    registerDepthPrepass();
}

auto rendering::PrepassModule::registerDepthPrepass() -> void
{
    PrepassModuleData& prepassData = m_graph->getBlackboard()->get<PrepassModuleData>();

    VkShaderModule vertex = m_vkCtx->device->createShaderModule( "depth", "vertexMain" );
    VkShaderModule fragment = m_vkCtx->device->createShaderModule( "depth", "fragmentMain" );

    std::vector<VkDescriptorSetLayout> descriptorLayout{ m_cameraDescriptor->layout };

    graphics::VulkanPipelineSpec defaultPipelineSpec{
        .options = { .cullMode = VK_CULL_MODE_BACK_BIT, .polyMode = VK_POLYGON_MODE_FILL },
        .descriptorLayout = descriptorLayout,
        .colorAttachmentCount = 0u,
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .blendEnable = VK_FALSE,
        .vertexModule = vertex,
        .fragmentModule = fragment,
        .pushConstantSize = sizeof( resources::FramePushConstant ),
        .pushConstantVisibility = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };

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

            core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
            core::Scene* scene = sceneManager->getCurrentScene();

            if ( !scene )
            {
                return;
            }

            if ( m_extent.width == 0 || m_extent.height == 0 )
            {
                return;
            }

            PrepassModuleData& prepassData = m_graph->getBlackboard()->get<PrepassModuleData>();
            uint32_t currentFrame = m_vkCtx->swapchain->getCurrentFrameNumber();

            graphics::VulkanMemoryAllocator* allocator = m_vkCtx->device->getMemAllocator();

            VkDrawIndexedIndirectCommand* indirectPtr = prepassData.indirectCommandsPtr[currentFrame];
            EntityParams* paramsPtr = prepassData.entitiesParamsPtr[currentFrame];

            // fill the vectors
            auto view = scene->getEnttRegistry().view<core::ModelComponent, core::TransformComponent>();

            uint32_t drawIndex{ 0u };
            {
                ZoneScopedN( "build indirect commands" );
                view.each( [&]( const entt::entity& id,
                                core::ModelComponent& modelComp,
                                core::TransformComponent& transformComp ) {
                    if ( !modelComp.pMesh )
                        return;

                    for ( const resources::Mesh& mesh : modelComp.pMesh->getMeshes() )
                    {
                        paramsPtr[drawIndex] =
                            EntityParams{ .transform = transformComp.getMatrix(), .materialIndex = mesh.materialIndex };

                        indirectPtr[drawIndex] =
                            VkDrawIndexedIndirectCommand{ .indexCount = mesh.indexCount,
                                                          .instanceCount = 1,
                                                          .firstIndex = mesh.indexOffset,
                                                          .vertexOffset = static_cast<int32_t>( mesh.vertexOffset ),
                                                          .firstInstance = drawIndex };
                        ++drawIndex;
                    }
                } );
            }

            if ( drawIndex == 0 )
            {
                return;
            }

            prepassData.drawCount[currentFrame] = drawIndex;

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

            graphics::VulkanBuffer& globalVertex = allocator->getGlobalVertexBuffer();
            graphics::VulkanBuffer& globalIndex = allocator->getGlobalIndexBuffer();

            graphics::VulkanBuffer& entityParams = prepassData.entitiesParamsBuffer.buffers.at( currentFrame );

            m_vkCtx->swapchain->beginRendering( prepassData.renderingInfo.vkRenderingInfo );
            m_vkCtx->swapchain->setupScissors( cmdBuffer );
            m_vkCtx->swapchain->setupViewport( cmdBuffer );

            prepassData.depthPrepassPipeline.bind( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS );

            vkCmdBindDescriptorSets( cmdBuffer,
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     prepassData.depthPrepassPipeline.getLayout(),
                                     0,
                                     1,
                                     &m_cameraDescriptor->set[currentFrame],
                                     0,
                                     nullptr );

            core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
            resources::FramePushConstant framePushConst{
                .vertexBufferAddress = allocator->getGlobalVertexBuffer().deviceAdress,
                .materialBufferAddress = assetManager->getMaterialBufferAddress(),
                .entityParamsBufferAddress = entityParams.deviceAdress };

            vkCmdPushConstants( cmdBuffer,
                                prepassData.depthPrepassPipeline.getLayout(),
                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                0,
                                sizeof( resources::FramePushConstant ),
                                &framePushConst );

            graphics::VulkanBuffer& commandsBuffer = prepassData.indirectCommandsBuffer.buffers.at( currentFrame );

            {
                TracyVkZone( m_vkCtx->tracyContext->getCtx(), cmdBuffer, "BindIndexBuffer" );
                vkCmdBindIndexBuffer( cmdBuffer, allocator->getGlobalIndexBuffer().vkBuffer, 0, VK_INDEX_TYPE_UINT32 );
            }

            {
                TracyVkZone( m_vkCtx->tracyContext->getCtx(), cmdBuffer, "CmdDrawIndirect" );
                vkCmdDrawIndexedIndirect(
                    cmdBuffer, commandsBuffer.vkBuffer, 0, drawIndex, sizeof( VkDrawIndexedIndirectCommand ) );
            }

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

    constexpr uint32_t maxIndirectCommands = 1u << 10;

    graphics::VulkanMemoryAllocator* allocator = m_vkCtx->device->getMemAllocator();
    // create the indirect commands buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof( VkDrawIndexedIndirectCommand ) * maxIndirectCommands;
    bufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    allocator->createBuffer( prepassData.indirectCommandsBuffer, bufferInfo, vmaAllocInfo, "indirect_commands_buffer" );

    constexpr uint32_t madIndirectEntityParams = 1u << 10;
    // create the entity params buffer
    VkBufferCreateInfo paramsBufferInfo{};
    paramsBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    paramsBufferInfo.size = sizeof( EntityParams ) * madIndirectEntityParams;
    paramsBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocationCreateInfo vmaParamsAllocInfo{};
    vmaParamsAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaParamsAllocInfo.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    allocator->createBuffer(
        prepassData.entitiesParamsBuffer, paramsBufferInfo, vmaParamsAllocInfo, "entity_params_buffer" );

    for ( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
    {
        void* ptr = nullptr;

        allocator->mapBuffer( prepassData.indirectCommandsBuffer.buffers[i], &ptr );
        prepassData.indirectCommandsPtr[i] = static_cast<VkDrawIndexedIndirectCommand*>( ptr );

        ptr = nullptr;
        allocator->mapBuffer( prepassData.entitiesParamsBuffer.buffers[i], &ptr );
        prepassData.entitiesParamsPtr[i] = static_cast<EntityParams*>( ptr );
    }
}

auto rendering::PrepassModule::destroyModuleResources() -> void
{
    rendering::Blackboard* blackboard = m_graph->getBlackboard();
    PrepassModuleData& prepassData = blackboard->get<PrepassModuleData>();

    graphics::VulkanMemoryAllocator* allocator = m_vkCtx->device->getMemAllocator();

    m_vkCtx->device->destroyImageView( prepassData.depth->vulkanImage.vkImageView );
    allocator->destroyImage( prepassData.depth->vulkanImage.vkImage, prepassData.depth->vulkanImage.vmaAllocation );

    m_vkCtx->device->destroyPipelineLayout( prepassData.depthPrepassPipeline.getLayout() );
    m_vkCtx->device->destroyPipeline( prepassData.depthPrepassPipeline.getPipeline() );

    for ( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++ )
    {
        allocator->unmapBuffer( prepassData.entitiesParamsBuffer.buffers.at( i ) );
        allocator->unmapBuffer( prepassData.indirectCommandsBuffer.buffers.at( i ) );
    }

    allocator->destroyBuffer( prepassData.entitiesParamsBuffer );
    allocator->destroyBuffer( prepassData.indirectCommandsBuffer );

    blackboard->removeFromStorage<PrepassModuleData>();
}

auto rendering::PrepassModule::setExtent( VkExtent2D extent ) -> void
{
    m_extent = extent;
}

auto rendering::PrepassModule::recreate( VkExtent2D extent ) -> void
{
    destroyModuleResources();
    registerPasses();
}

auto rendering::PrepassModule::registerIndirectCommandPrepass() -> void
{
}