add_library(
  kogayonon_core
  "include/core/event/app_event.hpp"
  "include/core/event/event.hpp"
  "include/core/event/event_dispatcher.hpp"
  "include/core/ecs/registry.hpp"
  "include/core/ecs/main_registry.hpp"
  "include/core/input/keyboard_events.hpp"
  "include/core/input/mouse_events.hpp"
  "include/core/input/mouse_codes.hpp"
  "include/core/input/keyboard_state.hpp"
  "include/core/scene/scene.hpp"
  "include/core/ecs/entity.hpp"

  "src/main_registry.cpp"
  "src/entity.cpp"
  "src/scene.cpp"
  "include/core/scene/scene_manager.hpp"
  "src/scene_manager.cpp"
  "include/core/ecs/components/texture_component.hpp"
  "include/core/ecs/components/name_component.hpp" "src/scene_events.cpp" "include/core/ecs/components/transform_component.hpp"
  "include/core/event/file_events.hpp" "include/core/systems/rendering_system.h" "src/rendering_system.cpp")

target_include_directories(kogayonon_core
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
  kogayonon_core
  PUBLIC EnTT::EnTT glm::glm-header-only
  PRIVATE soil2 kogayonon_utilities kogayonon_rendering spdlog::spdlog kogayonon_resources glad)