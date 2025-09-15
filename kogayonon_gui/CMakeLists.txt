add_library(
  kogayonon_gui
  "include/gui/debug_window.h"
  "include/gui/display.h"
  "include/gui/file_explorer.h"
  "include/gui/folder_display.h"
  "include/gui/imgui_manager.h"
  "include/gui/imgui_window.h"
  "include/gui/scene_viewport.h"
  "src/debug_window.cpp"
  "src/display.cpp"
  "src/file_explorer.cpp"
  "src/folder_display.cpp"
  "src/imgui_manager.cpp"
  "src/scene_viewport.cpp" "src/imgui_window.cpp" "include/gui/scene_hierarchy.h" "src/scene_hierarchy.cpp")

target_include_directories(kogayonon_gui
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
  kogayonon_gui
  PUBLIC imgui
  PRIVATE $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
          $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
          kogayonon_rendering kogayonon_core kogayonon_logger kogayonon_utilities kogayonon_resources)