cmake_minimum_required(VERSION 3.10)
project(kogayonon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(dependencies/imgui)
add_subdirectory(dependencies/stb)
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/cgltf)
add_subdirectory(dependencies/glm)

# Add your executable or library
add_executable(kogayonon
    "main.cpp"
    "src/app/app.cpp"
    "src/app/app.h"
    "src/core/asset_manager/asset_manager.cpp"
    "src/core/asset_manager/asset_manager.h"
    "src/core/asset_manager/loader/mesh.cpp"
    "src/core/asset_manager/loader/mesh.h"
    "src/core/asset_manager/loader/model.cpp"
    "src/core/asset_manager/loader/model.h"
    "src/core/asset_manager/loader/model_loader.cpp"
    "src/core/asset_manager/loader/model_loader.h"
    "src/core/asset_manager/loader/texture_loader.cpp"
    "src/core/asset_manager/loader/texture_loader.h"
    "src/core/input/input.cpp"
    "src/core/input/input.h"
    "src/core/klogger/klogger.cpp"
    "src/core/klogger/klogger.h"
    "src/core/layer/imgui_layer.cpp"
    "src/core/layer/imgui_layer.h"
    "src/core/layer/layer_stack.cpp"
    "src/core/layer/layer_stack.h"
    "src/core/layer/world_layer.cpp"
    "src/core/layer/world_layer.h"
    "src/core/renderer/camera.cpp"
    "src/core/renderer/camera.h"
    "src/core/renderer/renderer.cpp"
    "src/core/renderer/renderer.h"
    "src/core/serialize/mesh_serializer.cpp"
    "src/core/serialize/mesh_serializer.h"
    "src/core/task/task_manager.cpp"
    "src/core/task/task_manager.h"
    "src/core/time_tracker/time_tracker.cpp"
    "src/core/time_tracker/time_tracker.h"
    "src/core/ui/imgui_interface.cpp"
    "src/core/ui/imgui_interface.h"
    "src/core/ui/win_camera_settings.cpp"
    "src/core/ui/win_camera_settings.h"
    "src/event/event.cpp"
    "src/event/event.h"
    "src/shader/shader.cpp"
    "src/shader/shader.h"
    "src/window/window.cpp"
    "src/window/window.h"
)

  
target_include_directories(
  kogayonon PRIVATE
  "${CMAKE_SOURCE_DIR}/src/"
)

target_link_libraries(
  kogayonon PRIVATE
  cgltf stb imgui glad glfw glm
)

