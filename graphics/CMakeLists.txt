set(TargetSources 
	"include/graphics/vulkan_device.hpp"
	"include/graphics/vulkan_swapchain.hpp"
	"include/graphics/vulkan_pipeline.hpp"
	"include/graphics/vulkan_context.hpp"
	"include/graphics/vulkan_buffer.hpp"
	"include/graphics/vulkan_memory_allocator.hpp"
	"include/graphics/vulkan_descriptor.hpp"
	"include/graphics/vulkan_defines.hpp"
	"include/graphics/utils.hpp"

	"src/vulkan_device.cpp"
	"src/vulkan_swapchain.cpp"
	"src/vulkan_pipeline.cpp"
	"src/vulkan_memory_allocator.cpp"
)

add_library(graphics ${TargetSources})

target_include_directories(graphics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(graphics 
	PUBLIC  
	Vulkan::Vulkan 
	Vulkan::Headers
	GPUOpen::VulkanMemoryAllocator
	utilities
 PRIVATE $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
         $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
 spdlog::spdlog
 precompiled
 resources
)

