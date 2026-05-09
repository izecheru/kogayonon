set(targetSources
	"include/gui/utils/imgui_utils.hpp"
	"include/gui/utils/font_keys.hpp"
	"include/gui/vulkan_imgui_renderer.hpp"
	"include/gui/imgui_windows/imgui_base.hpp"
	"include/gui/imgui_windows/file_explorer.hpp"
	"include/gui/imgui_windows/viewport.hpp"
	"include/gui/imgui_windows/scene_hierarchy.hpp"
	"include/gui/imgui_windows/entity_properties.hpp"

	"src/vulkan_imgui_renderer.cpp"
	"src/imgui_base.cpp"
	"src/file_explorer.cpp"
	"src/viewport.cpp"
	"src/scene_hierarchy.cpp"
	"src/entity_properties.cpp"
)

add_library(gui ${targetSources})

target_include_directories(gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(gui PRIVATE imgui Vulkan::Vulkan graphics core utilities spdlog::spdlog resources EnTT::EnTT imguizmo)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /ZI /Ob0 /Od /RTC1")
endif()

