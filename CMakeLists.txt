cmake_minimum_required(VERSION 3.10)
project(kogayonon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Thirdparty dependencies
add_subdirectory(dependencies/imgui)
add_subdirectory(dependencies/stb)
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/cgltf)
add_subdirectory(dependencies/glm)

set(RENDERER_SOURCES
    "src/renderer/camera.cpp"
    "src/renderer/camera.h"
    "src/renderer/renderer.cpp"
    "src/renderer/renderer.h"
)

set(ASSET_MANAGER_SOURCES
    "src/asset_manager/loader/mesh.cpp"
    "src/asset_manager/loader/mesh.h"
    "src/asset_manager/loader/model.cpp"
    "src/asset_manager/loader/model.h"
    "src/asset_manager/loader/model_loader.cpp"
    "src/asset_manager/loader/model_loader.h"
    "src/asset_manager/loader/texture_loader.cpp"
    "src/asset_manager/loader/texture_loader.h"
    "src/asset_manager/asset_manager.cpp"
    "src/asset_manager/asset_manager.h"
)

set(LAYER_SOURCES
    "src/layer/imgui_layer.cpp"
    "src/layer/imgui_layer.h"
    "src/layer/layer.h"
    "src/layer/layer_stack.cpp"
    "src/layer/layer_stack.h"
    "src/layer/world_layer.cpp"
    "src/layer/world_layer.h"
)

set(USER_INTERFACE_SOURCES
    "src/ui/imgui_manager.h"
    "src/ui/imgui_window.h"
    "src/ui/imgui_manager.cpp"
    "src/ui/win_camera_settings.h"
    "src/ui/win_camera_settings.cpp"
    "src/ui/scene_viewport.h"
    "src/ui/scene_viewport.cpp"
)

set(SHADER_SOURCES
    "src/shader/shader.cpp"
    "src/shader/shader_manager.h"
    "src/shader/shader_manager.cpp"
    "src/shader/shader.h"
)

set(INPUT_SOURCES
    "src/input/keyboard_events.h"
    "src/input/keyboard_state.h"
    "src/input/key_codes.h"
    "src/input/mouse_codes.h"
    "src/input/mouse_events.h"
)

set(UTILS_SOURCES
    "src/klogger/klogger.cpp"
    "src/klogger/klogger.h"

    "src/serialize/mesh_serializer.cpp"
    "src/serialize/mesh_serializer.h"
    "src/serialize/serializer.h"

    "src/task/task_manager.cpp"
    "src/task/task_manager.h"

    "src/time_tracker/time_tracker.cpp"
    "src/time_tracker/time_tracker.h"

    "src/context_manager/context_manager.h"

    "src/singleton/singleton.h"
)

set(EVENTS_SOURCES
    "src/event/app_event.h"
    "src/event/event.h"
    "src/event/event_manager.h"
    "src/event/event_manager.cpp"
 "src/event/event_dispatcher.h")

add_executable(kogayonon
    "main.cpp"
    "src/app/app.cpp"
    "src/app/app.h"
    "src/window/window.h"
    "src/window/window.cpp"

    ${RENDERER_SOURCES}
    ${ASSET_MANAGER_SOURCES}
    ${LAYER_SOURCES}
    ${USER_INTERFACE_SOURCES}
    ${SHADER_SOURCES}
    ${INPUT_SOURCES}
    ${UTILS_SOURCES}
    ${EVENTS_SOURCES}
)

target_include_directories(
  kogayonon PRIVATE
  "${CMAKE_SOURCE_DIR}/src/"
)

target_link_libraries(
  kogayonon PRIVATE
  cgltf stb imgui glad glfw glm
)