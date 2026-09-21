#include "renderer/vulkan_renderer.hpp"
#include <SDL2/SDL.h>
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/text_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "graphics/vulkan_context.hpp"
#include "gui/imgui_windows/viewport.hpp"
#include "gui/vulkan_imgui_renderer.hpp"
#include "physics/jolt_physics.hpp"
#include "renderer/blackboard.hpp"
#include "renderer/frame_graph.hpp"
#include "resources/mesh_push_constant.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "utilities/tracy_utils/tracy_vulkan_utils.hpp"
#include "utilities/utils/utils.hpp"

rendering::VulkanRenderer::VulkanRenderer( graphics::VulkanContext* pCtx, SDL_Window* window )
    : m_vkCtx{ pCtx }
    , m_wnd{ window }
    , m_mouseCoords{ -1, -1 }
    , m_modulesInit{ false }
    , m_resizeRequested{ false }
    , m_extent{ m_vkCtx->swapchain->getSwapchainExtent() } // careful with order of initialization
    , m_frameGraph{ std::make_unique<FrameGraph>( m_vkCtx->device.get() ) }
{
    core::EventDispatcher* eventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();

    eventDispatcher->addHandler<core::MouseClickedEvent, &VulkanRenderer::onMouseClicked>( *this );
    eventDispatcher->addHandler<core::WindowResizeEvent, &VulkanRenderer::onWindowResize>( *this );

    createCameraBuffers();
    createCameraDescriptorSetLayout();
    createCameraDescriptorSet();

    initImgui();
    initModules();
}

auto rendering::VulkanRenderer::presentToScreen() -> void
{
    m_vkCtx->swapchain->presentFrame();
}

auto rendering::VulkanRenderer::onUpdate() -> void
{
    render();
    presentToScreen();

    if ( !m_resizeRequested )
        return;

    m_resizeRequested = false;

    destroyModules();

    m_extent = m_vkCtx->swapchain->getSwapchainExtent();
    initModules();
}

auto rendering::VulkanRenderer::initModules() -> void
{
    core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

    m_imguiModule = std::make_unique<ImGuiModule>( m_frameGraph.get(), m_vkCtx, m_pImguiRenderer.get() );
    m_prepassModule = std::make_unique<PrepassModule>( m_frameGraph.get(), m_vkCtx, m_extent, &m_cameraDescriptor );

    m_geometryModule = std::make_unique<GeometryModule>(
        m_frameGraph.get(), m_vkCtx, m_extent, &m_cameraDescriptor, glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f } );

    // WIP
    // m_pickingModule = std::make_unique<PickingModule>(
    //     m_frameGraph.get(), m_vkCtx, m_pImguiRenderer.get(), m_extent, &m_cameraDescriptor );

    m_imguiModule->setViewport();

    m_frameGraph->compile();
}

rendering::VulkanRenderer::~VulkanRenderer()
{
    m_vkCtx->device->destroyBuffer( m_cameraBuffers );
    m_vkCtx->device->destroyDescriptorSetLayout( m_cameraDescriptor.layout );
}

auto rendering::VulkanRenderer::render() -> void
{
    m_vkCtx->swapchain->waitForFences();
    m_vkCtx->swapchain->resetFences();
    updateCameraBuffer();

    VkCommandBuffer cmdBuffer = m_vkCtx->swapchain->getCurrentCommandBuffer();

    m_vkCtx->swapchain->aquireNextImage();
    m_vkCtx->swapchain->beginCommandBuffer();

    m_vkCtx->swapchain->prepareAttachment();

    m_frameGraph->execute( cmdBuffer );

#ifdef TRACY_ENABLE
    m_vkCtx->tracyContext->collect( cmdBuffer );
#endif
}

auto rendering::VulkanRenderer::createCameraBuffers() -> void
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof( core::CameraUbo );
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    m_vkCtx->device->createBuffer( m_cameraBuffers, bufferInfo, vmaAllocInfo, "cameraBuffer" );
}

auto rendering::VulkanRenderer::updateCameraBuffer() -> void
{
    core::SceneManager* sceneManager = core::MainRegistry::getInstance().getSceneManager();
    core::Scene* scene = sceneManager->getCurrentScene();
    auto view = scene->getEnttRegistry().view<core::PerspectiveCameraComponent>();
    view.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComp ) {
        if ( !cameraComp.isUsed )
        {
            return;
        }

        VkExtent2D extent = m_pImguiRenderer->getViewportExtent();
        if ( ( cameraComp.props.extent.x != extent.width || cameraComp.props.extent.y != extent.height ) &&
             extent.width > 1 && extent.height > 1 )
        {
            cameraComp.props.extent = glm::ivec2{ extent.width, extent.height };
            cameraComp.props.changed = true;
        }

        if ( cameraComp.props.changed )
        {
            cameraComp.updateUbo();
        }

        uint32_t currentFrame = m_vkCtx->swapchain->getCurrentFrameNumber();

        m_vkCtx->device->copyToBuffer( cameraComp.ubo, m_cameraBuffers.buffers.at( currentFrame ) );
    } );
}

auto rendering::VulkanRenderer::createCameraDescriptorSet() -> void
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vkCtx->globalDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_cameraDescriptor.layout;
    allocInfo.pNext = nullptr;

    for ( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++ )
    {
        m_vkCtx->device->allocateDescriptorSet( m_cameraDescriptor.set[i], allocInfo );
    }

    std::vector<VkDescriptorBufferInfo> bufferInfo{};
    bufferInfo.resize( MAX_FRAMES_IN_FLIGHT );
    for ( auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
    {
        VkDescriptorBufferInfo info{};
        bufferInfo.at( i ).buffer = m_cameraBuffers.buffers.at( i ).vkBuffer;
        bufferInfo.at( i ).offset = 0;
        bufferInfo.at( i ).range = sizeof( core::CameraUbo );
    }

    std::vector<VkWriteDescriptorSet> writeDescriptors{};

    for ( auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++ )
    {
        VkWriteDescriptorSet uniformBufferDescriptor{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_cameraDescriptor.set[i],
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo.at( i ),
        };
        writeDescriptors.push_back( uniformBufferDescriptor );
    }

    m_vkCtx->device->updateDescriptorSet( writeDescriptors );
}

auto rendering::VulkanRenderer::createCameraDescriptorSetLayout() -> void
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

    m_vkCtx->device->createDescriptorSetLayout( m_cameraDescriptor.layout, layoutInfo );
}

auto rendering::VulkanRenderer::initImgui() -> void
{
    m_pImguiRenderer =
        std::make_shared<gui::VulkanImguiRenderer>( m_wnd, m_vkCtx->device.get(), m_vkCtx->swapchain.get() );
}

auto rendering::VulkanRenderer::onMouseClicked( const core::MouseClickedEvent& e ) -> void
{
    if ( !m_pickingModule )
        return;

    core::SceneEventHandler* sceneHandler = core::MainRegistry::getInstance().getSceneManager()->getEventHandler();
    entt::entity currentEntity = sceneHandler->getCurrentEntityId();

    int mouseX, mouseY;
    SDL_GetMouseState( &mouseX, &mouseY );

    gui::Viewport* viewport =
        dynamic_cast<gui::Viewport*>( m_pImguiRenderer->getImGuiWindows().at( gui::ImGuiWindowName::Viewport ).get() );

    gui::ImGuiProps* props = viewport->getProps();

    float localX = ( mouseX - props->x ) / props->width;
    float localY = ( mouseY - props->y ) / props->height;

    if ( localX >= 0.0f && localX <= 1.0f && localY >= 0.0f && localY <= 1.0f )
    {
        m_mouseCoords.x = static_cast<int>( localX * m_extent.width );
        m_mouseCoords.y = static_cast<int>( localY * m_extent.height );
        KINFO( "x {} y {}", m_mouseCoords.x, m_mouseCoords.y );
        m_pickingModule->setCoords( m_mouseCoords );
    }
}

auto rendering::VulkanRenderer::onWindowResize( const core::WindowResizeEvent& e ) -> void
{
    m_resizeRequested = true;
}

auto rendering::VulkanRenderer::destroyModules() -> void
{
    m_vkCtx->device->waitIdle();

    m_geometryModule.reset();
    m_pickingModule.reset();
    m_imguiModule.reset();
    m_prepassModule.reset();

    m_frameGraph->clearGraph();
}
