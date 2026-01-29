cmake_minimum_required(VERSION 3.26)

project(kogayonon_game)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_executable(kogayonon_game
		"include/game/game.hpp"
		"src/game.cpp"
	)
else()
    add_executable(kogayonon_game WIN32
		"include/game/game.hpp"
		"src/game.cpp"
    )
endif()

target_include_directories(kogayonon_game
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(
    kogayonon_game
  PRIVATE kogayonon_window
          glad
          kogayonon_core
          kogayonon_utilities
          kogayonon_rendering
          kogayonon_resources
          kogayonon_physics
          SDL2::SDL2
)
