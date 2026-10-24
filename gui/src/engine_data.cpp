#include "gui/imgui_windows/engine_data.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "gui/utils/font_keys.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "core/ecs/main_registry.hpp"
#include "graphics/vulkan_memory_allocator.hpp"
#include "graphics/vulkan_device.hpp"
#include "resources/vertex.hpp"

gui::EngineDataWindow::EngineDataWindow( const std::string& name,
                                         const EngineDataSpec& spec,
                                         graphics::VulkanDevice* device )
    : ImGuiWindow{ name }
    , m_device{ device }
    , m_spec{ spec }
    , m_smoothedDelta{ 0.0f }
    , m_timeUpdate{ 0.0f }
{
}

auto gui::EngineDataWindow::render() -> void
{
    if ( !begin() )
        return;

    graphics::VulkanMemoryAllocator* allocator = m_device->getMemAllocator();
    graphics::VulkanBuffer& gVertexBuffer = allocator->getGlobalVertexBuffer();
    graphics::VulkanBuffer& gIndexBuffer = allocator->getGlobalIndexBuffer();

    VmaAllocationInfo vertexAllocInfo{ allocator->getAllocInfo( gVertexBuffer.vmaAllocation ) };
    VmaAllocationInfo indexAllocInfo{ allocator->getAllocInfo( gIndexBuffer.vmaAllocation ) };

    VkDeviceSize vertexRemaining = vertexAllocInfo.size - gVertexBuffer.lastWrittenOffset;
    VkDeviceSize indexRemaining = indexAllocInfo.size - gIndexBuffer.lastWrittenOffset;

    VkDeviceSize vertexOccupied = vertexAllocInfo.size - vertexRemaining;
    VkDeviceSize indexOccupied = indexAllocInfo.size - indexRemaining;

    ImGui::SeparatorText( "GLobal Vertex allocation" );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
        ImGui::Text( "Total allocated size %s", allocator->formatSize( vertexAllocInfo.size ).c_str() );
        ImGui::Text( "Ocuppied byte size %s", allocator->formatSize( vertexOccupied ).c_str() );
        ImGui::Text( "Remaining byte size %s", allocator->formatSize( vertexRemaining ).c_str() );
    } );

    ImGui::SeparatorText( "GLobal Index allocation" );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
        ImGui::Text( "Total allocated size %s", allocator->formatSize( indexAllocInfo.size ).c_str() );
        ImGui::Text( "Ocuppied byte size %s", allocator->formatSize( indexOccupied ).c_str() );
        ImGui::Text( "Remaining byte size %s", allocator->formatSize( indexRemaining ).c_str() );
    } );

    VkDeviceSize bufferSize = allocator->getTotalBufferAllocationSize();

    ImGui::SeparatorText( "Buffer data" );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
        ImGui::Text( "Total allocated size %s", allocator->formatSize( bufferSize ).c_str() );
    } );

    VkDeviceSize imageSize = allocator->getTotalImageAllocationSize();

    ImGui::SeparatorText( "Texture image data" );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
        ImGui::Text( "Total allocated size %s", allocator->formatSize( imageSize ).c_str() );
    } );

    utilities::TimeTracker* timeTracker = core::MainRegistry::getInstance().getTimeTracker();

    ImGui::SeparatorText( "Frame time" );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
        const double deltaMs = timeTracker->getDurationMilliseconds( DELTA_TIME );
        const double deltaS = timeTracker->getDurationInSeconds( DELTA_TIME );

        m_smoothedDelta = m_smoothedDelta * 0.95 + deltaS * 0.05;
        const double fps = 1.0 / m_smoothedDelta;

        ImGui::Text( "Vertices: %d", vertexOccupied / sizeof( resources::Vertex ) );
        ImGui::Text( "Frame: %.2f ms  (%.1f FPS)", deltaMs, fps );
    } );

    core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

    ImGui::SeparatorText( "Materials" );

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
        ImGui::Text( "Material slots %d/ %d", assetManager->getMaterialAllocatedCount(), MAX_TEXTURE_SUPPORT );
        const VkDeviceSize currentAllocated = assetManager->getMaterialAllocatedCount() * sizeof( resources::Material );
        constexpr const VkDeviceSize maxCanOccupy = MAX_TEXTURE_SUPPORT * sizeof( resources::Material );
        const VkDeviceSize remaining = maxCanOccupy - currentAllocated;

        ImGui::Text( "Occupied bytes %s / %s",
                     allocator->formatSize( currentAllocated ).c_str(),
                     allocator->formatSize( maxCanOccupy ).c_str() );

        ImGui::Text( "Remaining bytes %s", allocator->formatSize( maxCanOccupy - currentAllocated ).c_str() );
    } );

    end();
}
