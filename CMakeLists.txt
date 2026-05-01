cmake_minimum_required(VERSION 3.26)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

project(kogayonon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") # .lib, .a
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") # .dll, .so
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") # .exe

if(MSVC)
	add_compile_options(/bigobj)
	add_compile_options(/utf-8)
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(rapidjson CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(Vulkan)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_subdirectory("dependencies/imgui")
add_subdirectory("dependencies/glad")
add_subdirectory("dependencies/cgltf")
add_subdirectory("dependencies/imguizmo")
# using lua 5.4 since sol is not yet compatible with latest 5.5 and vcpkg verison is indeed 5.5
add_subdirectory("dependencies/lua")

# My own modules
add_subdirectory("window")
add_subdirectory("editor")
add_subdirectory("graphics")
add_subdirectory("gui")
add_subdirectory("core")
add_subdirectory("utilities")
add_subdirectory("resources")
add_subdirectory("physics")
add_subdirectory("renderer")
add_subdirectory("precompiled")
