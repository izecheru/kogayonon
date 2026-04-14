#include "gui/imgui_windows/scene_hierarchy.hpp"

gui::SceneHierarchy::SceneHierarchy( const std::string& name, const SceneHierarchySpec& spec )
    : ImGuiWindow{ name }
    , m_spec{ spec }
    , m_selectedEntity{ entt::null }
{
}

void gui::SceneHierarchy::render()
{
  if ( !begin() )
    return;

  ImGui::Text( "This is the scene hierarchy window" );

  ImGui::End();
}

void gui::SceneHierarchy::drawItemContexMenu( const std::string& itemId, entt::entity ent )
{
}

void gui::SceneHierarchy::drawContextMenu()
{
}
