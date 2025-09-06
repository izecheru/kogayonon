add_library(kogayonon_window
"include/window/window.h"
"src/window.cpp"
)

target_include_directories(kogayonon_window PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(kogayonon_window PUBLIC SDL2::SDL2 PRIVATE kogayonon_logger)