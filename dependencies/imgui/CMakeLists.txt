 find_package(SDL2 CONFIG REQUIRED)

add_library(imgui
  "src/imgui.cpp"
  "src/imgui_demo.cpp"
  "src/imgui_draw.cpp"
  "src/imgui_impl_opengl3.cpp"
  "src/imgui_impl_sdl2.cpp"
  "src/imgui_tables.cpp"
  "src/imgui_widgets.cpp"
)

target_include_directories(imgui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

    target_link_libraries(imgui
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )