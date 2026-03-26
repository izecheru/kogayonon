if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_executable(editor
		"include/editor/editor.hpp"
		"src/editor.cpp"
		"src/main.cpp"
	)
else()
    add_executable(editor WIN32
        "include/editor/editor.hpp"
        "src/editor.cpp"
        "src/main.cpp"
    )
endif()

target_include_directories(editor
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(
  editor
  PRIVATE 
	  SDL2::SDL2main
	  spdlog::spdlog
	  SDL2::SDL2
	  utilities
	  graphics
	  resources
	  window 
	  imgui
	  core
	  gui
)
