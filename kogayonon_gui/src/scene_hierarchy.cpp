#include "gui/scene_hierarchy.hpp"
#include <spdlog/spdlog.h>
#include "core/ecs/components/name_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"

using namespace kogayonon_core;

namespace kogayonon_gui
{
SceneHierarchyWindow::SceneHierarchyWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_selectedIndex{ -1 }
{
  EVENT_DISPATCHER()->addHandler<kogayonon_core::KeyPressedEvent, &SceneHierarchyWindow::onKeyPressed>( *this );
}

void SceneHierarchyWindow::onKeyPressed( const kogayonon_core::KeyPressedEvent& e )
{
  if ( e.getKeyCode() == KeyCode::Escape && m_selectedIndex != -1 )
  {
    m_selectedIndex = -1;
  }
}

void SceneHierarchyWindow::draw()
{
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }

  m_pCurrentScene = kogayonon_core::SceneManager::getCurrentScene();
  auto scene = m_pCurrentScene.lock();

  // if there is no scene to render return
  if ( !scene )
  {
    ImGui::End();
    return;
  }

  auto& enttRegistry = scene->getEnttRegistry();
  auto view = enttRegistry.view<NameComponent>();
  const auto& pEventDispatcher = EVENT_DISPATCHER();
  std::vector<Entity> entities;

  static int entSize = 0;
  for ( auto [entity, nameComponent] : view.each() )
  {
    Entity ent( scene->getRegistry(), entity );
    entities.emplace_back( ent );
  }

  if ( entSize != entities.size() )
  {
    entSize = entities.size();
    m_selectedIndex = -1;
  }

  auto listSize = ImVec2( -FLT_MIN, ImGui::GetContentRegionAvail().y );
  auto avail = ImGui::GetContentRegionAvail();
  ImGui::BeginChild( "EntityListRegion", avail, ImGuiChildFlags_ResizeY );

  // Push the style color
  ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0, 0, 0, 0 ) );

  if ( ImGui::BeginListBox( "##EntityList", listSize ) )
  {
    for ( int i = 0; i < entities.size(); i++ )
    {
      bool isSelected = ( m_selectedIndex == i );
      auto& entity = entities.at( i );
      const auto& nameComp = entity.getComponent<NameComponent>();

      std::string label = std::format( "{}{}{}", nameComp.name, "##", std::to_string( i ) );
      if ( ImGui::Selectable( label.c_str(), isSelected ) )
      {
        m_selectedIndex = i;
        pEventDispatcher->emitEvent( SelectEntityEvent( entity.getEnttEntity() ) );
      }

      auto pTexture = entity.tryGetComponent<TextureComponent>();
      if ( ImGui::IsItemHovered() && pTexture )
      {
        drawTextureTooltip( pTexture, ImVec2( 250.0f, 250.0f ) );
      }
    }
    ImGui::EndListBox();
  }

  ImGui::PopStyleColor( 1 );

  ImGui::EndChild();
  ImGui::End();
}

/**
 * @brief Draws some info of the texture loaded, currently only for texture components
 * @param textureComp The texture component we get the texture id and width/ height, path and so on
 * @param size The size of the tooltip window
 */
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