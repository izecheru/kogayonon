set(TargetSources 
	"include/graphics/vulkan_device.hpp"
	"include/graphics/vulkan_swapchain.hpp"
	"include/graphics/vulkan_pipeline.hpp"
	"include/graphics/vulkan_context.hpp"
	"include/graphics/utils.hpp"

	"src/vulkan_device.cpp"
	"src/vulkan_swapchain.cpp"
)

add_library(graphics ${TargetSources})

target_include_directories(graphics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(graphics PRIVATE Vulkan::Vulkan 
 PRIVATE $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
         $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
 spdlog::spdlog)

