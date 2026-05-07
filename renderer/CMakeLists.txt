
set(targetSources
"include/renderer/vulkan_renderer.hpp"
"src/vulkan_renderer.cpp"
)

add_library(renderer ${targetSources})

target_include_directories(renderer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(renderer PRIVATE gui SDL2::SDL2 Vulkan::Vulkan graphics core utilities spdlog::spdlog resources EnTT::EnTT)
