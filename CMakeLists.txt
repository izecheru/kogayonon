cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("kogayonon")
add_subdirectory("dependencies/GLFW")			#window oppener
add_subdirectory("dependencies/glad")			#window oppener
add_subdirectory("dependencies/stb")			#window oppener
add_subdirectory("dependencies/glm")			#window oppener
add_subdirectory("dependencies/imgui")		#ui
add_subdirectory("dependencies/assimp")		#ui
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" )

add_executable(kogayonon main.cpp ${SOURCES})
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

set(DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/dependencies")

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw 
	glad stb imgui assimp)

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC 
"${CMAKE_CURRENT_SOURCE_DIR}/include/"
"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm/glm"
"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include"
"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui"
"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/stb/include"
"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/GLFW"
"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/assimp/include"
)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET kogayonon PROPERTY CXX_STANDARD 17)
endif()

#set_target_properties(kogayonon PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# copy assets to the built folder
add_custom_command(TARGET kogayonon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"  # Source (shaders folder)
    "${CMAKE_BINARY_DIR}/shaders"  # Destination (where the executable runs)
)

add_custom_command(TARGET kogayonon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/models"  # Source (shaders folder)
    "${CMAKE_BINARY_DIR}/models"  # Destination (where the executable runs)
)

# TODO: Add tests and install targets if needed.