 find_package(SDL2 CONFIG REQUIRED)

add_library(imgui
  "src/imgui.cpp"
  "src/imgui_demo.cpp"
  "src/imgui_draw.cpp"
  "src/imgui_impl_opengl3.cpp"
  "src/imgui_impl_sdl2.cpp"
  "src/imgui_tables.cpp"
  "src/imgui_widgets.cpp"
  "src/imgui_stdlib.cpp"

  "include/imconfig.h"
  "include/imgui.h"
  "include/imgui_impl_opengl3.h"
  "include/imgui_impl_opengl3_loader.h"
  "include/imgui_impl_sdl2.h"
  "include/imgui_internal.h"
  "include/imstb_rectpack.h"
  "include/imstb_textedit.h"
  "include/imstb_truetype.h"
  "include/imgui_stdlib.h"
)

target_include_directories(imgui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

    target_link_libraries(imgui
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )