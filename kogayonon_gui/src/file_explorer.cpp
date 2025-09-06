#include "gui/file_explorer.h"

namespace kogayonon_gui {
void FileExplorerWindow::draw()
{
    if ( ImGui::Begin( m_props->m_name.c_str(), nullptr, m_props->flags ) )
    {
        for ( auto& it : m_displays )
        {
            it.second->draw();
        }
    }
    ImGui::End();
}
} // namespace kogayonon_gui