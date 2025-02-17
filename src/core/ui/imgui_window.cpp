#include "core/ui/imgui_window.h"
#include <imgui-1.91.8/imgui.h>

namespace kogayonon
{
  void ImguiWindow::draw() {
    ImGui::Begin(m_name.c_str());
    ImGui::Text("Acum ca i-ai dat push ionute ai belit-o, cuvintele urate raman :(");
    ImGui::End();
  }

  bool ImguiWindow::isHovered() {
    return m_is_hovered;
  }
}