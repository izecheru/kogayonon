cmake_minimum_required(VERSION 3.10)

project(kogayonon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------------------------------------------------
# Output directories for different artifact types
# -------------------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") # .lib, .a
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") # .dll, .so
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") # .exe

# -------------------------------------------------------------------
# Thirdparty dependencies
# -------------------------------------------------------------------

set(CMAKE_PREFIX_PATH "dependencies/sdl2/cmake")
find_package(SDL2 REQUIRED CONFIG)

add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/imgui)
add_subdirectory(dependencies/cgltf)
add_subdirectory(dependencies/glm)
add_subdirectory(dependencies/entt)
add_subdirectory(dependencies/soil2)

# My own modules
add_subdirectory(kogayonon_logger)
add_subdirectory(kogayonon_utilities)
add_subdirectory(kogayonon_window)
add_subdirectory(kogayonon_core)
add_subdirectory(kogayonon_resources)
add_subdirectory(kogayonon_app)
add_subdirectory(kogayonon_gui)
add_subdirectory(kogayonon_rendering)
# -------------------------------------------------------------------
# Source groups
# -------------------------------------------------------------------

add_executable(kogayonon  "kogayonon_app/src/main.cpp")

target_include_directories(
    kogayonon PRIVATE
    "${CMAKE_SOURCE_DIR}/dependencies/sdl2/include"
)

target_link_libraries(
    kogayonon PRIVATE
    kogayonon_app
)