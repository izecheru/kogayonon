add_library(kogayonon_rendering
"include/rendering/framebuffer.h"
"src/framebuffer.cpp"

 "include/rendering/renderer.h")

target_include_directories(kogayonon_rendering PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_rendering PUBLIC glad PRIVATE kogayonon_logger)