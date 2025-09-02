#include "folder_display.h"

namespace kogayonon
{
void FolderDisplay::draw()
{
  ImGui::Text("Current path ", m_path);
}
} // namespace kogayonon