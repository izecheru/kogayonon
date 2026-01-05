if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_executable(kogayonon_app
		"include/app/app.hpp"
		"src/app.cpp"
		"src/main.cpp"
	)
else()
    add_executable(kogayonon_app WIN32
        "include/app/app.hpp"
        "src/app.cpp"
        "src/main.cpp"
    )
endif()

target_include_directories(kogayonon_app
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(
  kogayonon_app
  PRIVATE kogayonon_window
          glad
          spdlog::spdlog
          kogayonon_core
          kogayonon_gui
          kogayonon_utilities
          kogayonon_rendering
          kogayonon_resources
          kogayonon_physics
          SDL2::SDL2
)
