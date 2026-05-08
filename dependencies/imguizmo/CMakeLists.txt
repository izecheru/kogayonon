add_library(imguizmo
  "src/GraphEditor.cpp"
  "src/ImCurveEdit.cpp"
  "src/ImGradient.cpp"
  "src/ImGuizmo.cpp"
  "src/ImSequencer.cpp"

  "include/GraphEditor.h"
  "include/ImCurveEdit.h"
  "include/ImGradient.h"
  "include/ImGuizmo.h"
  "include/ImSequencer.h"
  "include/ImZoomSlider.h"
)

target_include_directories(imguizmo PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

target_link_libraries(imguizmo PUBLIC imgui)