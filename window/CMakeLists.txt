set(targetSources 
"include/window/window.hpp"
"src/window.cpp"
)

add_library(window ${targetSources})

find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")
target_include_directories(
  window
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE ${SOL2_INCLUDE_DIRS})

target_link_libraries(
  window
  SDL3::SDL3
)
