add_library(
  kogayonon_rendering
  "include/rendering/framebuffer.h" "include/rendering/renderer.h"
  "src/framebuffer.cpp" "src/renderer.cpp" "include/rendering/batcher.h"  "include/rendering/model_batch_renderer.h" "include/rendering/batches/batches.h")

target_include_directories(kogayonon_rendering
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
  kogayonon_rendering
  PUBLIC glad
  PRIVATE kogayonon_resources kogayonon_logger)
