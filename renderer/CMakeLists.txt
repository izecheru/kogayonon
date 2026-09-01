# Define headers
set(MODULE_HEADERS
    "include/renderer/modules/geometry_module.hpp"
    "include/renderer/modules/prepass_module.hpp"
    "include/renderer/modules/picking_module.hpp"
    "include/renderer/modules/imgui_module.hpp"
    "include/renderer/modules/shadowmap_module.hpp"
    "include/renderer/modules/module_base.hpp"
    "include/renderer/modules/module_descriptor.hpp"
    "include/renderer/modules/module_rendering_info.hpp"
)

set(HEADER_FILES
    "include/renderer/vulkan_renderer.hpp"
    "include/renderer/frame_graph.hpp"
    "include/renderer/blackboard.hpp"
    "include/renderer/node.hpp"
    ${MODULE_HEADERS}
)

add_library(renderer)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(renderer PRIVATE
    "src/vulkan_renderer.cpp"
    "src/frame_graph.cpp"
    "src/geometry_module.cpp"
    "src/imgui_module.cpp"
    "src/shadowmap_module.cpp"
    "src/prepass_module.cpp"
    "src/picking_module.cpp"
)

target_sources(renderer PUBLIC
    ${HEADER_FILES}
)

target_include_directories(renderer PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(renderer PUBLIC PICKING)
target_link_libraries(renderer PRIVATE gui SDL2::SDL2 Vulkan::Vulkan graphics core utilities resources EnTT::EnTT)

if(KOGAYONON_ENABLE_TRACY)
    message("tracy is enabled, linking it to renderer")
    target_link_libraries(renderer PRIVATE Tracy::TracyClient)
endif()
