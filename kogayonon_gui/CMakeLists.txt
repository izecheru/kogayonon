add_library(
  kogayonon_gui
  "include/gui/debug_window.hpp"
  "include/gui/display.hpp"
  "include/gui/file_explorer.hpp"
  "include/gui/folder_display.hpp"
  "include/gui/imgui_manager.hpp"
  "include/gui/imgui_window.hpp"
  "include/gui/scene_viewport.hpp"
  "src/debug_window.cpp"
  "src/display.cpp"
  "src/file_explorer.cpp"
  "src/folder_display.cpp"
  "src/imgui_manager.cpp"
  "src/scene_viewport.cpp" "src/imgui_window.cpp" "include/gui/scene_hierarchy.hpp" "src/scene_hierarchy.cpp" "src/performance_window.cpp" "src/entity_properties.cpp" "include/imgui_utils/imgui_utils.h")

target_include_directories(kogayonon_gui
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
  kogayonon_gui
  PUBLIC imgui
  PRIVATE $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
          $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
          kogayonon_rendering kogayonon_core spdlog::spdlog kogayonon_utilities kogayonon_resources)
