
set(HEADER_FILES
	"include/graphics/vulkan_device.hpp"
	"include/graphics/vulkan_device.inl"
	"include/graphics/vulkan_swapchain.hpp"
	"include/graphics/vulkan_pipeline.hpp"
	"include/graphics/vulkan_context.hpp"
	"include/graphics/vulkan_buffer.hpp"
	"include/graphics/vulkan_descriptor.hpp"
	"include/graphics/vulkan_image.hpp"
	"include/graphics/shader_compiler.hpp"
	"include/graphics/utils.hpp"
)

set(SOURCE_FILES
	"src/vulkan_device.cpp"
	"src/vulkan_swapchain.cpp"
	"src/vulkan_pipeline.cpp"
	"src/shader_compiler.cpp"
)

if(KOGAYONON_ENABLE_TRACY)
    list(APPEND HEADER_FILES
        "include/graphics/vulkan_tracy_context.hpp"
    )

    list(APPEND SOURCE_FILES
        "src/vulkan_tracy_context.cpp"
    )
endif()

add_library(graphics ${HEADER_FILES} ${SOURCE_FILES})

target_compile_definitions(graphics PUBLIC VERTEX_PROVIDED)

target_include_directories(graphics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(graphics 
	PUBLIC  
	Vulkan::Vulkan 
	Vulkan::Headers
	GPUOpen::VulkanMemoryAllocator
	utilities
 PRIVATE $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
         $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
 precompiled
 resources
)

if(KOGAYONON_ENABLE_TRACY)
	message("tracy enabled, linking it to graphics")
	target_link_libraries(graphics PRIVATE Tracy::TracyClient)
endif()

option(GRAPHICS_ENABLE_LEAKS "Enable VMA Debug print for leaks" OFF)

if(GRAPHICS_ENABLE_LEAKS)
	add_compile_definitions(
		PRINT_LEAKS
	)
endif()

