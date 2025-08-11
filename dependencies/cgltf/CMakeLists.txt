cmake_minimum_required(VERSION 3.10)
project(cgltf)

add_library(
  cgltf
)

target_sources(cgltf PRIVATE "src/cgltf.c")

target_include_directories(cgltf PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
