add_library(kogayonon_core

"include/core/event/app_event.h"
"include/core/event/event.h"
"include/core/event/event_dispatcher.h"
"include/core/event/event_manager.h"
"src/event_manager.cpp"

"include/core/ecs/registry.h"
"include/core/ecs/registry_manager.h"
"src/registry_manager.cpp"

"include/core/input/keyboard_events.h"
"include/core/input/mouse_events.h"
"include/core/input/mouse_codes.h"
"include/core/input/keyboard_state.h"
)

target_include_directories(kogayonon_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_core
PUBLIC EnTT glm kogayonon_utilities
PRIVATE cgltf stb  glad kogayonon_logger kogayonon_gui kogayonon_window SDL2::SDL2)