add_library(
  utilities
  "include/utilities/task_manager/task_manager.hpp"
  "include/utilities/directory_watcher/directory_watcher.hpp"
  "include/utilities/math/math.hpp"
  "include/utilities/input/keyboard_state.hpp"
  "include/utilities/input/key_codes.hpp"
  "include/utilities/input/mouse_codes.hpp"
  "include/utilities/configurator/configurator.hpp"
  "include/utilities/time_tracker/time_tracker.hpp"
  "include/utilities/utils/utils.hpp"
  "include/utilities/script/script_compiler.hpp"
  "include/utilities/script/script.hpp"
  "include/utilities/yaml_serializer/yaml_serializer.hpp"
  "include/utilities/json_serializer/json_serializer.hpp"
  "include/utilities/utils/yaml_utils.hpp"
  "include/utilities/fonts/fontawesome7.hpp"

  "src/task_manager.cpp"
  "src/time_tracker.cpp"
  "src/directory_watcher.cpp"
  "src/configurator.cpp"
  "src/script_compiler.cpp"
  "src/script.cpp"
  "src/yaml_serializer.cpp" 
  "src/json_serializer.cpp"
 )

target_include_directories(utilities
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           PRIVATE ${SOL2_INCLUDE_DIRS})

target_link_libraries(utilities PUBLIC cgltf 
 PRIVATE
	glm::glm-header-only 
	yaml-cpp::yaml-cpp
	spdlog::spdlog 
	lua 
	glad 
	rapidjson
	assimp::assimp
	resources
)