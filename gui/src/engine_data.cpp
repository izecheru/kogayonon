#include "gui/imgui_windows/engine_data.hpp"
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
    ImGui::Text( "Total allocated size %s", allocator->formatSize( vertexAllocInfo.size ).c_str() );
    ImGui::Text( "Ocuppied byte size %s", allocator->formatSize( vertexOccupied ).c_str() );
    ImGui::Text( "Remaining byte size %s", allocator->formatSize( vertexRemaining ).c_str() );

    ImGui::SeparatorText( "GLobal Index allocation" );
    ImGui::Text( "Total allocated size %s", allocator->formatSize( indexAllocInfo.size ).c_str() );
    ImGui::Text( "Ocuppied byte size %s", allocator->formatSize( indexOccupied ).c_str() );
    ImGui::Text( "Remaining byte size %s", allocator->formatSize( indexRemaining ).c_str() );

    ImGui::SeparatorText( "Buffer data" );
    VkDeviceSize bufferSize = allocator->getTotalBufferAllocationSize();
    ImGui::Text( "Total allocated size %s", allocator->formatSize( bufferSize ).c_str() );

    ImGui::SeparatorText( "Texture image data" );
    VkDeviceSize imageSize = allocator->getTotalImageAllocationSize();
    ImGui::Text( "Total allocated size %s", allocator->formatSize( imageSize ).c_str() );

    ImGui::SeparatorText( "Frame time" );
    utilities::TimeTracker* timeTracker = core::MainRegistry::getInstance().getTimeTracker();
    ImGui::Text( "%.2fms", timeTracker->getDurationMilliseconds( DELTA_TIME ) );

    end();
}
