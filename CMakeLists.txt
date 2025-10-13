cmake_minimum_required(VERSION 3.10)
project(kogayonon)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Tell CMake to use vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

# -------------------------------------------------------------------
# Output directories for different artifact types
# -------------------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") # .lib, .a
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") # .dll, .so
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") # .exe
# -------------------------------------------------------------------
# Thirdparty dependencies
# -------------------------------------------------------------------

find_package(SDL2 CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(rapidjson CONFIG REQUIRED)

add_subdirectory(dependencies/imgui)
add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/cgltf)
add_subdirectory(dependencies/imguizmo)

# My own modules
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

add_executable(kogayonon "kogayonon_app/src/main.cpp")

target_include_directories(kogayonon PRIVATE)

target_link_libraries(
  kogayonon
  PRIVATE kogayonon_app $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
          $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)