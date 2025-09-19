#include "gui/scene_hierarchy.hpp"
#include "core/ecs/components/name_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "logger/logger.hpp"
using namespace kogayonon_logger;
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
  if ( !scene )
  {
    ImGui::End();
    return;
  }
  auto& enttRegistry = scene->getEnttRegistry();
  auto& view = enttRegistry.view<TextureComponent, NameComponent>();
  std::vector<Entity> entities;
  for ( auto& [entity, textureComponent, nameComponent] : view.each() )
  {
    Entity ent( scene->getRegistry(), entity );
    entities.push_back( ent );
  }
  static int selectedIndex = -1;
  static int hoveredIndex = -1;

  ImVec2 listSize = ImVec2( -FLT_MIN, ImGui::GetContentRegionAvail().y );
  ImVec2 avail = ImGui::GetContentRegionAvail();
  ImGui::BeginChild( "EntityListRegion", avail, ImGuiChildFlags_ResizeY );
  ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0, 0, 0, 0 ) );
  if ( ImGui::BeginListBox( "##EntityList", listSize ) )
  {
    for ( int i = 0; i < entities.size(); i++ )
    {
      bool isSelected = selectedIndex == i;
      auto& entity = entities.at( i );
      auto& nameComp = entity.getComponent<NameComponent>();
      if ( ImGui::Selectable( nameComp.name.c_str(), isSelected ) )
      {
        if ( selectedIndex != i )
        {
          selectedIndex = i;
          Logger::info( "selected index = ", i );
        }
      }
      if ( ImGui::IsItemHovered() )
      {
        if ( auto* pTexture = entity.tryGetComponent<TextureComponent>() )
        {
          drawTextureTooltip( pTexture, ImVec2{ 128.0f, 128.0f } );
        }
      }
    }
  }
  ImGui::PopStyleColor( 1 );
  ImGui::EndListBox();
  ImGui::EndChild();
  ImGui::End();
}

void SceneHierarchyWindow::drawTextureTooltip( TextureComponent* textureComp, ImVec2 size )
{
  ImGui::BeginTooltip();
  if ( auto tex = textureComp->pTexture.lock() )
  {
    ImGui::Text( "%s", tex->getName().c_str() );
    ImGui::Text( "%d/%d", tex->getWidth(), tex->getHeight() );
    ImGui::Text( "%s", tex->getPath().c_str() );
    ImGui::Image( (ImTextureID)(intptr_t)tex->getTextureId(), size );
  }
  ImGui::EndTooltip();
}
} // namespace kogayonon_gui