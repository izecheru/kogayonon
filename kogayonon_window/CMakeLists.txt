add_library(kogayonon_window "include/window/window.hpp" "src/window.cpp")

find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")
target_include_directories(kogayonon_window
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           PRIVATE ${SOL2_INCLUDE_DIRS})

target_link_libraries(
  kogayonon_window
  PUBLIC $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
         $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
  PRIVATE spdlog::spdlog lua)