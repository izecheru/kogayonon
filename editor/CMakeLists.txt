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
	  SDL2::SDL2
	  utilities
	  resources
	  renderer
	  graphics
	  physics
	  window 
	  imgui
	  core
	  gui
)

if(KOGAYONON_ENABLE_TRACY)
	message("tracy is enabled, linking it to editor")
	target_link_libraries(editor PRIVATE Tracy::TracyClient)
	add_custom_command(TARGET editor POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:Tracy::TracyClient>
		$<TARGET_FILE_DIR:editor>
	)
endif()


add_custom_command(
    TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/engine_resources"
        "$<TARGET_FILE_DIR:editor>/engine_resources"
    COMMENT "Copying engine resources"
)
