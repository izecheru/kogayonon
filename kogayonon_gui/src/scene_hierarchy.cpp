#include "gui/scene_hierarchy.hpp"
#include "core/ecs/components/name_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
using namespace kogayonon_core;

namespace kogayonon_gui
{
SceneHierarchyWindow::SceneHierarchyWindow( std::string name )
    : ImGuiWindow( name )
{
}

void SceneHierarchyWindow::draw()
{
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }

  auto& sceneManager = ::SceneManager::getInstance();
  auto& currentScene = sceneManager.getCurrentScene();
  auto& scene = currentScene.lock();
  if ( scene )
  {
    auto& enttRegistry = scene->getEnttRegistry();
    auto& view = enttRegistry.view<NameComponent>();
    std::vector<std::string> entityNames;
    for ( auto& [entity, nameComponent] : view.each() )
    {
      entityNames.push_back( nameComponent.name );
    }

    std::vector<const char*> items;
    items.reserve( entityNames.size() );
    for ( auto& str : entityNames )
    {
      items.push_back( str.c_str() );
    }
    static int selectedIndex = -1;

    ImGui::BeginChild( "EntityListRegion", ImVec2( 0, 0 ), true );
    if ( ImGui::BeginListBox( "##EntityList", ImVec2( 0, 0 ) ) )
    {
      for ( int i = 0; i < items.size(); i++ )
      {
        bool isSelected = selectedIndex == i;
        if ( ImGui::Selectable( items.at( i ), isSelected ) )
        {
          selectedIndex = i;
        }
      }
      ImGui::EndListBox();
    }
  }
  ImGui::EndChild();
  ImGui::End();
}
} // namespace kogayonon_gui