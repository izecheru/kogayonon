set(targetSources
"include/window/window.hpp"
"src/window.cpp"
)

add_library(window ${targetSources})

find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")
target_include_directories(window
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           PRIVATE ${SOL2_INCLUDE_DIRS})

target_link_libraries(
  window
  PUBLIC $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
         $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
         lua
  PRIVATE spdlog::spdlog )
