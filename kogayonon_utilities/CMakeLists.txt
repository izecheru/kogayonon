add_library(
  kogayonon_utilities
  "include/utilities/task_manager/task_manager.hpp"
  "include/utilities/shader/shader.hpp"
  "include/utilities/directory_watcher/directory_watcher.hpp"
  "include/utilities/shader/shader_manager.hpp"
  "include/utilities/math/math.hpp"
  "include/utilities/input/keyboard_state.hpp"
  "include/utilities/input/key_codes.hpp"
  "include/utilities/input/mouse_codes.hpp"
  "include/utilities/configurator/configurator.hpp"
  "include/utilities/asset_manager/asset_manager.hpp"
  "include/utilities/asset_manager/cgltf_loader.hpp"
  "include/utilities/asset_manager/assimp_loader.hpp"
  "include/utilities/time_tracker/time_tracker.hpp"
  "include/utilities/utils/utils.hpp"
  "include/utilities/script/script_compiler.hpp"
  "include/utilities/script/script.hpp"
  "include/utilities/yaml_serializer/yaml_serializer.hpp"
  "include/utilities/json_serializer/json_serializer.hpp"
  "include/utilities/utils/yaml_utils.hpp"

  "src/task_manager.cpp"
  "src/shader_manager.cpp"
  "src/shader.cpp"
  "src/asset_manager.cpp"
  "src/time_tracker.cpp"
  "src/directory_watcher.cpp"
  "src/configurator.cpp"
 "src/script_compiler.cpp"
 "src/script.cpp"
 "src/yaml_serializer.cpp" 
 "src/json_serializer.cpp" "src/assimp_loader.cpp" "src/cgltf_loader.cpp")

target_include_directories(kogayonon_utilities
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           PRIVATE ${SOL2_INCLUDE_DIRS})

target_link_libraries(kogayonon_utilities PUBLIC cgltf 
 PRIVATE
glm::glm-header-only 
kogayonon_resources 
yaml-cpp::yaml-cpp
spdlog::spdlog 
lua 
glad 
rapidjson
soil2 
assimp::assimp
)