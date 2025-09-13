add_library(
  kogayonon_core
  "include/core/event/app_event.h"
  "include/core/event/event.h"
  "include/core/event/event_dispatcher.h"
  "include/core/event/event_manager.h"
  "include/core/ecs/registry.h"
  "include/core/ecs/main_registry.h"
  "include/core/input/keyboard_events.h"
  "include/core/input/mouse_events.h"
  "include/core/input/mouse_codes.h"
  "include/core/input/keyboard_state.h"
  "include/core/scene/scene.h"
  "include/core/ecs/entity.h"
  "include/core/ecs/components/mesh_component.h"
  "src/event_manager.cpp"
  "src/main_registry.cpp"
  "src/entity.cpp"
  "src/scene.cpp"
  "include/core/scene/scene_manager.h"
  "src/scene_manager.cpp"
  "include/core/ecs/components/texture_component.h"
  "src/components/texture_component.cpp")

target_include_directories(kogayonon_core
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
  kogayonon_core
  PUBLIC EnTT::EnTT glm::glm-header-only
  PRIVATE soil2 kogayonon_utilities kogayonon_logger kogayonon_resources glad)
