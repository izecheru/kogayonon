set(HEADER_FILES
  "include/imconfig.h"
  "include/imgui.h"
  "include/imgui_impl_sdl3.h"
  "include/imgui_impl_vulkan.h"
  "include/imgui_internal.h"
  "include/imstb_rectpack.h"
  "include/imstb_textedit.h"
  "include/imstb_truetype.h"
  "include/imgui_stdlib.h"
)

set(SOURCE_FILES
  "src/imgui.cpp"
  "src/imgui_draw.cpp"
  "src/imgui_impl_sdl3.cpp"
  "src/imgui_tables.cpp"
  "src/imgui_widgets.cpp"
  "src/imgui_stdlib.cpp"
  "src/imgui_impl_vulkan.cpp"
)

add_library(imgui
	${SOURCE_FILES}
)

target_include_directories(imgui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)
target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_link_libraries(imgui
        PRIVATE
        SDL3::SDL3
    )