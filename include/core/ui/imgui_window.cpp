#include "imgui_window.h"
#include <imgui-1.91.8/imgui.h>

namespace kogayonon
{
  void ImguiWindow::draw() {
    ImGui::Begin(m_name.c_str());
    ImGui::Text("Hello manca-mi-ai pula");
    ImGui::End();
  }
}

