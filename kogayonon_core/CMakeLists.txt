add_library(
  kogayonon_core
  "include/core/event/app_event.hpp"
  "include/core/event/event.hpp"
  "include/core/event/event_dispatcher.hpp"
  "include/core/ecs/registry.hpp"
  "include/core/ecs/main_registry.hpp"
  "include/core/scene/scene.hpp"
  "include/core/ecs/entity.hpp"
  "include/core/input/keyboard_events.hpp"
  "include/core/input/mouse_events.hpp"
  "include/core/ecs/components/texture_component.hpp"
  "include/core/ecs/entity_types.hpp"
  "include/core/ecs/components/mesh_component.hpp"
  "include/core/ecs/components/identifier_component.hpp"
  "include/core/scene/scene_manager.hpp"
  "include/core/ecs/components/transform_component.hpp"
  "include/core/ecs/components/pointlight_component.hpp"
  "include/core/event/file_events.hpp"
  "include/core/systems/rendering_system.hpp"
  "include/core/ecs/components/index_component.hpp"
  "include/core/event/project_event.hpp"
  "include/core/project/project.hpp"
  "include/core/project/project_manager.hpp"
  "include/core/ecs/components/rigidbody_component.hpp"
  "include/core/ecs/components/outline_component.hpp"
  "include/core/event/event_emitter.hpp"
  "include/core/utils/meta_utilities.hpp"

  "src/main_registry.cpp"
  "src/entity.cpp"
  "src/scene.cpp"
  "src/scene_manager.cpp"
  "src/scene_events.cpp"
  "src/rendering_system.cpp"
  "src/project_manager.cpp"  "include/core/systems/scripting_system.hpp" "src/scripting_system.cpp" "src/registry.cpp" "src/event_dispatcher.cpp")


find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")
target_include_directories(kogayonon_core
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" 
                           PRIVATE ${SOL2_INCLUDE_DIRS})

target_link_libraries(
  kogayonon_core
  PUBLIC EnTT::EnTT glm::glm-header-only kogayonon_physics lua
  PRIVATE  soil2 kogayonon_utilities kogayonon_rendering spdlog::spdlog kogayonon_resources glad kogayonon_window
)