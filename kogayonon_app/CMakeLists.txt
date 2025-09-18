add_library(kogayonon_app "include/app/app.hpp" "src/app.cpp")
target_include_directories(kogayonon_app
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(
  kogayonon_app
  PRIVATE kogayonon_window
          glad
          kogayonon_logger
          kogayonon_core
          kogayonon_gui
          kogayonon_utilities
          kogayonon_rendering
          kogayonon_resources
)