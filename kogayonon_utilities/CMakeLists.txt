add_library(
  kogayonon_utilities
  "include/utilities/task_manager/task_manager.h"
  "include/utilities/shader_manager/shader.h"
  "include/utilities/shader_manager/shader_manager.h"
  "src/task_manager.cpp"
  "src/shader_manager.cpp"
  "src/shader.cpp")
target_include_directories(kogayonon_utilities
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_utilities PRIVATE kogayonon_logger glad
                                                  glm::glm-header-only)
