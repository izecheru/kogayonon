cmake_minimum_required(VERSION 3.10)

project(imgui)

add_library(imgui)

target_sources(imgui PRIVATE 
  "src/imgui.cpp"
  "src/imgui_demo.cpp"
  "src/imgui_draw.cpp"
  "src/imgui_impl_glfw.cpp"
  "src/imgui_impl_opengl3.cpp"
  "src/imgui_tables.cpp"
  "src/imgui_widgets.cpp"
)

target_include_directories(imgui PUBLIC 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  "${CMAKE_SOURCE_DIR}/dependencies/glfw/include/"
)
