add_library(
  kogayonon_utilities
  "include/utilities/task_manager/task_manager.hpp"
  "include/utilities/shader/shader.hpp"
  "include/utilities/shader/shader_manager.hpp"
  "include/utilities/directory_watcher/directory_watcher.hpp"
  "include/utilities/math/math.hpp"
  "include/utilities/input/keyboard_state.hpp"
  "include/utilities/input/key_codes.hpp"
  "include/utilities/input/mouse_codes.hpp"
  "include/utilities/configurator/configurator.hpp"
  "include/utilities/asset_manager/asset_manager.hpp"
  "include/utilities/time_tracker/time_tracker.hpp"
  "include/utilities/utils/utils.hpp"
  "include/utilities/script/script_compiler.hpp"
  "include/utilities/script/script.hpp"

  "src/task_manager.cpp"
  "src/shader_manager.cpp"
  "src/shader.cpp"
  "src/asset_manager.cpp"
  "src/time_tracker.cpp"
  "src/directory_watcher.cpp"
  "src/configurator.cpp"
  "src/serializer.cpp"
  "src/jsoner.cpp"
  "include/utilities/serializer/serializer.hpp"
 "src/jsoner.cpp"  "include/utilities/jsoner/jsoner.hpp"
 "src/script_compiler.cpp" "src/script.cpp")

target_include_directories(kogayonon_utilities
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(kogayonon_utilities PRIVATE cgltf spdlog::spdlog glad glm::glm-header-only kogayonon_resources soil2 rapidjson)