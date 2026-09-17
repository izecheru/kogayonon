set(HEADER_FILES
	"include/gui/utils/imgui_utils.hpp"
	"include/gui/directory_hierarchy.hpp"
	"include/gui/utils/imgui_dragdrop_defines.hpp"
	"include/gui/utils/font_keys.hpp"
	"include/gui/vulkan_imgui_renderer.hpp"
	"include/gui/imgui_windows/imgui_base.hpp"
	"include/gui/imgui_windows/file_explorer.hpp"
	"include/gui/imgui_windows/viewport.hpp"
	"include/gui/imgui_windows/scene_hierarchy.hpp"
	"include/gui/imgui_windows/entity_properties.hpp"
)

set(SOURCE_FILES
	"src/vulkan_imgui_renderer.cpp"
	"src/imgui_base.cpp"
	"src/file_explorer.cpp"
	"src/viewport.cpp"
	"src/scene_hierarchy.cpp"
	"src/entity_properties.cpp"
 "src/directory_hierarchy.cpp")

add_library(gui ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(gui 
PUBLIC imgui imguizmo 
PRIVATE Vulkan::Vulkan graphics core utilities  resources EnTT::EnTT tinyfiledialogs::tinyfiledialogs)



