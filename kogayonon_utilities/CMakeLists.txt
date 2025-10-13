add_library(
  kogayonon_utilities
  "include/utilities/task_manager/task_manager.hpp"
  "include/utilities/shader_manager/shader.hpp"
  "include/utilities/shader_manager/shader_manager.hpp"
  "src/task_manager.cpp"
  "src/shader_manager.cpp"
  "src/shader.cpp"
  "include/utilities/asset_manager/asset_manager.hpp"
  "src/asset_manager.cpp"
  "include/utilities/time_tracker/time_tracker.hpp"
  "src/time_tracker.cpp"
  "src/directory_watcher.cpp"
  "include/utilities/directory_watcher/directory_watcher.hpp"
  "include/utilities/math/math.hpp"
  "include/utilities/input/keyboard_state.hpp"
  "include/utilities/input/key_codes.hpp"
  "include/utilities/input/mouse_codes.hpp"
  "include/utilities/configurator/configurator.hpp"
 "src/configurator.cpp")

target_include_directories(kogayonon_utilities
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(kogayonon_utilities PRIVATE cgltf spdlog::spdlog glad glm::glm-header-only kogayonon_resources soil2)