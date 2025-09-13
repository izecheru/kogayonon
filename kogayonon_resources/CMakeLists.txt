add_library(
  kogayonon_resources
  "include/resources/mesh.h"
  "include/resources/vertex.h"
  "include/resources/model.h"
  "include/resources/texture.h"
  "src/mesh.cpp"
  "src/mesh_helper.cpp"
  "src/model.cpp")

target_include_directories(kogayonon_resources
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_resources PUBLIC glm::glm-header-only)
