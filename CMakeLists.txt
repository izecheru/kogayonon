cmake_minimum_required(VERSION 3.30)

enable_testing()

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

option(KOGAYONON_ENABLE_TRACY "Enable Tracy profiler" OFF)

add_compile_definitions(MAX_FRAMES_IN_FLIGHT=3)

if(KOGAYONON_ENABLE_TRACY)
add_compile_definitions(
    TRACY_ENABLE 
)
endif()

add_compile_definitions(
    NOMINMAX
)

include(FetchContent)
# ======================================
# MSDF Atlas Generator for font rendering
# ======================================
set(MSDF_ATLAS_DYNAMIC_RUNTIME ON CACHE BOOL "" FORCE)
set(MSDFGEN_USE_SKIA OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_ENABLE_SKIA OFF CACHE BOOL "" FORCE)
set(MSDFGEN_USE_VCPKG ON CACHE BOOL "Use vcpkg package manager" FORCE)

FetchContent_Declare(
  msdf-atlas-gen
  GIT_REPOSITORY https://github.com/Chlumsky/msdf-atlas-gen.git
  GIT_TAG master
)
FetchContent_MakeAvailable(msdf-atlas-gen)

# run this command before vcpkg install enkiTS freetype sdl2[vulkan] sol2 glm
# entt spdlog yaml-cpp tinyfiledialogs benchmark rapidjson assimp shader-slang
# joltphysics vulkan-memory-allocator vulkan 
if(KOGAYONON_ENABLE_TRACY)
	find_package(Tracy CONFIG REQUIRED)
endif()
# GTest should also be enabled through flags like tracy
find_package(GTest CONFIG REQUIRED)
find_package(Freetype CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)
find_package(rapidjson CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(Jolt CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(Vulkan)
find_package(enkiTS CONFIG REQUIRED)

add_subdirectory("dependencies/imgui")
add_subdirectory("dependencies/cgltf")
add_subdirectory("dependencies/imguizmo")
# using lua 5.4 since sol is not yet compatible with latest 5.5 and vcpkg
# verison is indeed 5.5
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
#add_subdirectory("test_runner")

# add_subdirectory("widget_loader") add_subdirectory("widgets")
