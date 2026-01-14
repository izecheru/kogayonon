cmake_minimum_required(VERSION 3.18)

project(kogayonon)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
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
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)

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
add_subdirectory(kogayonon_physics)
