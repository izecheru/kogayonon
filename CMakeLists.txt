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

include(FetchContent)
# ======================================
# MSDF Atlas Generator for font rendering
# ======================================
FetchContent_Declare(
    msdf-atlas-gen
    GIT_REPOSITORY https://github.com/Chlumsky/msdf-atlas-gen.git
    GIT_TAG 6148900d59423059bafde2f51a0cb303184404bd # latest commit as of 07 jun 2026
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(msdf-atlas-gen)

# ======================================
# EnTT library for entity component system
# ======================================
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG main 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(EnTT)
find_package(EnTT CONFIG REQUIRED)

# ======================================
# SDL2
# ======================================
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.x
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(SDL2)
find_package(SDL2 CONFIG REQUIRED)

# ======================================
# Sol2 for c++ -> lua -> c++ bindings
# ======================================
FetchContent_Declare(
    sol2 
    GIT_REPOSITORY https://github.com/ThePhd/sol2.git
    GIT_TAG develop
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(sol2)
find_package(sol2 CONFIG REQUIRED)

# ======================================
# GLM 
# ======================================
FetchContent_Declare(
    glm 
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(glm)
find_package(glm CONFIG REQUIRED)

# ======================================
# Spdlog logging lib 
# ======================================
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(spdlog)
find_package(spdlog CONFIG REQUIRED)

# ======================================
# Yaml cpp 
# ======================================
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(yaml-cpp)
find_package(yaml-cpp CONFIG REQUIRED)

# ======================================
# Tinyfiledialogs 
# ======================================
FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY https://github.com/native-toolkit/libtinyfiledialogs.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(tinyfiledialogs)
find_package(tinyfiledialogs CONFIG REQUIRED)

# ======================================
# Google benchmark 
# ======================================
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG main 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(benchmark)
find_package(benchmark CONFIG REQUIRED)

# ======================================
# Rapid json 
# ======================================
FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(rapidjson)
find_package(rapidjson CONFIG REQUIRED)

# ======================================
# Assimp 
# ======================================
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(assimp)
find_package(assimp CONFIG REQUIRED)

# ======================================
# Slang 
# ======================================
FetchContent_Declare(
    slang
    GIT_REPOSITORY https://github.com/shader-slang/slang.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(slang)
find_package(slang CONFIG REQUIRED)

# ======================================
# Jolt 
# ======================================
FetchContent_Declare(
    Jolt
    GIT_REPOSITORY https://github.com/srand/jolt.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(Jolt)
find_package(Jolt CONFIG REQUIRED)

# ======================================
# VMA
# ======================================
FetchContent_Declare(
	VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG master 
    FIND_PACKAGE_ARGS # if we already have the package, don't download it
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(Vulkan)

add_subdirectory("dependencies/imgui")
#add_subdirectory("dependencies/glad")
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
