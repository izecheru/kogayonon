set(targetSources
	"include/gui/utils/imgui_utils.hpp"
	"include/gui/vulkan_imgui_renderer.hpp"
	"include/gui/imgui_windows/imgui_base.hpp"
	"include/gui/imgui_windows/file_explorer.hpp"
	"include/gui/imgui_windows/viewport.hpp"
	"include/gui/imgui_windows/scene_hierarchy.hpp"

	"src/vulkan_imgui_renderer.cpp"
	"src/imgui_base.cpp"
	"src/file_explorer.cpp"
	"src/viewport.cpp"
	"src/scene_hierarchy.cpp"
)

add_library(gui ${targetSources})

target_include_directories(gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(gui PRIVATE imgui Vulkan::Vulkan graphics core utilities spdlog::spdlog resources EnTT::EnTT)
