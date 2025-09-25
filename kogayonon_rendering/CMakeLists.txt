add_library(kogayonon_rendering 
"include/rendering/framebuffer.hpp"
"include/rendering/renderer.hpp"
"src/framebuffer.cpp"
"src/renderer.cpp"
 "src/camera.cpp" "include/rendering/camera/camera.hpp"
)
target_include_directories(kogayonon_rendering PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_rendering PUBLIC glad PRIVATE kogayonon_resources spdlog::spdlog glm::glm-header-only)