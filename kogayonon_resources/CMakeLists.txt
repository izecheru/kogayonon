add_library(
  kogayonon_resources
  "include/resources/mesh.hpp"
  "include/resources/vertex.hpp"
  "include/resources/texture.hpp"
  "include/resources/pointlight.hpp"
  "include/resources/spotlight.hpp"
  "include/resources/directional_light.hpp"
  "include/resources/light_types.hpp"
  "include/resources/skin.hpp"

  "src/mesh.cpp"
  "src/texture.cpp")

target_include_directories(kogayonon_resources
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")


target_link_libraries(kogayonon_resources PUBLIC glm::glm-header-only)