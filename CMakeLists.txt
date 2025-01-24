# CMakeList.txt : CMake project for kogayonon, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("kogayonon")


file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(kogayonon main.cpp ${SOURCES})
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )

set(DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/dependencies")


add_subdirectory(
${DEPENDENCIES_DIR}/glad
)

add_subdirectory(
${DEPENDENCIES_DIR}/glfw
)

add_subdirectory(
${DEPENDENCIES_DIR}/glm
)


target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw glad )
target_include_directories( kogayonon PRIVATE

"${CMAKE_SOURCE_DIR}/include"

"${DEPENDENCIES_DIR}/glad/include"

"${DEPENDENCIES_DIR}/glfw/include"

"${DEPENDENCIES_DIR}/glm"
)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET kogayonon PROPERTY CXX_STANDARD 17)
endif()

# TODO: Add tests and install targets if needed.
