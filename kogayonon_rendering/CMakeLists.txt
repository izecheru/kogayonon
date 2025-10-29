add_library(kogayonon_rendering
"include/rendering/camera/camera.hpp"
"include/rendering/framebuffer.hpp"
"include/rendering/lightcount_uniformbuffer.hpp"
"include/rendering/light_shader_storagebuffer.hpp"
"include/rendering/opengl_framebuffer.hpp"
"include/rendering/renderer.hpp"
"include/rendering/shader_storagebuffer.hpp"
"include/rendering/uniformbuffer.hpp"

"src/camera.cpp" 
"src/framebuffer.cpp"
"src/lightcount_uniformbuffer.cpp"
"src/light_shader_storagebuffer.cpp"
"src/opengl_framebuffer.cpp"
"src/renderer.cpp"
)
target_include_directories(kogayonon_rendering PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_rendering PUBLIC glad PRIVATE kogayonon_utilities kogayonon_resources spdlog::spdlog glm::glm-header-only)