#include "gui/folder_display.h"

namespace kogayonon_gui {
void FolderDisplay::draw()
{
    ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "Current path: %s", m_path.c_str());
}
} // namespace kogayonon_gui