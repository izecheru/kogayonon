set(HEADER_FILES
    "include/core/systems/undo_redo_system.hpp"
    "include/core/event/imgui_event.hpp"
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
    "include/core/scene/scene_event_handler.hpp"
    "include/core/ecs/components/transform_component.hpp"
    "include/core/ecs/components/pointlight_component.hpp"
    "include/core/ecs/components/camera_component.hpp"
    "include/core/event/file_events.hpp"
    "include/core/ecs/components/index_component.hpp"
    "include/core/ecs/components/text_component.hpp"
    "include/core/event/project_event.hpp"
    "include/core/project/project.hpp"
    "include/core/project/project_manager.hpp"
    "include/core/ecs/components/rigidbody_component.hpp"
    "include/core/ecs/components/outline_component.hpp"
    "include/core/event/event_emitter.hpp"
    "include/core/utils/meta_utilities.hpp"
    "include/core/systems/scripting_system.hpp"
    "include/core/systems/scene_rendering_system.hpp"
    "include/core/asset_manager/asset_manager.hpp"
    "include/core/asset_manager/cgltf_loader.hpp"
    "include/core/asset_manager/assimp_loader.hpp"
    "include/core/asset_manager/font_loader.hpp"
    "include/core/asset_manager/tinygltf_loader.hpp"
)

set(SOURCE_FILES
    "src/main_registry.cpp"
    "src/undo_redo_system.cpp"
    "src/entity.cpp"
    "src/scene.cpp"
    "src/scene_manager.cpp"
    "src/scene_events.cpp"
    "src/asset_manager.cpp"
    "src/assimp_loader.cpp"
    "src/cgltf_loader.cpp"
    "src/font_loader.cpp"
    "src/project_manager.cpp"
    "src/scripting_system.cpp"
    "src/registry.cpp"
    "src/event_dispatcher.cpp"
    "src/scene_rendering_system.cpp"
    "src/scene_event_handler.cpp"
    "src/tinygltf_loader.cpp"
)

add_library(core ${SOURCE_FILES} ${HEADER_FILES})

find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf_v3.h")
target_include_directories(
  core
  PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PUBLIC ${TINYGLTF_INCLUDE_DIRS}
  PRIVATE ${SOL2_INCLUDE_DIRS})

target_precompile_headers(core REUSE_FROM precompiled)
target_link_libraries(
  core
  PUBLIC EnTT::EnTT glm::glm-header-only lua physics precompiled msdf-atlas-gen  PRIVATE  window yaml-cpp::yaml-cpp utilities
          resources
)



if(KOGAYONON_ENABLE_TRACY)
	message("tracy is enabled, linking it to core")
	target_link_libraries(core PRIVATE Tracy::TracyClient)
endif()
