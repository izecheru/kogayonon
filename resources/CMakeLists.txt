set(
 targetSources 
  "include/resources/mesh.hpp"
  "include/resources/vertex.hpp"
  "include/resources/texture.hpp"
  "include/resources/pointlight.hpp"
  "include/resources/spotlight.hpp"
  "include/resources/directional_light.hpp"
  "include/resources/light_types.hpp"
  "include/resources/skeleton.hpp"
  "include/resources/mesh_push_constant.hpp"
  "include/resources/material.hpp"
  "include/resources/font.hpp"

  "src/mesh.cpp"
  "src/font.cpp"
  "src/texture.cpp")

  add_library(resources ${targetSources})


target_include_directories(resources
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_precompile_headers(resources REUSE_FROM precompiled)
target_link_libraries(resources PUBLIC glm::glm-header-only Vulkan::Vulkan precompiled graphics rapidjson)
