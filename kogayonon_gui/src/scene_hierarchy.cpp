#include "gui/scene_hierarchy.hpp"
#include <entt/entity/entity.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/components/name_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "imgui_utils/imgui_utils.h"
#include "utilities/input/mouse_codes.hpp"

using namespace kogayonon_core;

namespace kogayonon_gui
{
SceneHierarchyWindow::SceneHierarchyWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_selectedIndex{ -1 }
    , m_popUp{}
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
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };

  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }

  m_props->hovered = ImGui::IsWindowHovered();
  m_props->focused = ImGui::IsWindowFocused();

  m_pCurrentScene = kogayonon_core::SceneManager::getCurrentScene();
  auto scene = m_pCurrentScene.lock();
  if ( m_selectedIndex == -1 )
  {
    SelectEntityEvent entEvent;
    EVENT_DISPATCHER()->emitEvent<SelectEntityEvent>( entEvent );
  }

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
    entities.emplace_back( scene->getRegistry(), entity );
  }

  if ( entSize != entities.size() )
  {
    entSize = entities.size();
    m_selectedIndex = -1;
  }

  const auto& io = ImGui::GetIO();
  auto avail = ImGui::GetContentRegionAvail();
  ImGui::BeginChild( "EntityListRegion", avail, false, ImGuiChildFlags_ResizeY );

  if ( ImGui::BeginListBox( "##EntityList", avail ) )
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
      drawItemContexMenu( label, entity );
    }
    drawContextMenu();
    ImGui::EndListBox();
  }

  ImGui::EndChild();
  ImGui::End();
}

void SceneHierarchyWindow::drawContextMenu()
{
  if ( ImGui::BeginPopupContextWindow( "SceneHierarchyContext",
                                       ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight ) )
  {
    if ( ImGui::MenuItem( "Create Entity" ) )
    {
      if ( auto scene = SceneManager::getCurrentScene().lock() )
      {
        scene->addEntity();
      }
    }
    if ( ImGui::MenuItem( "Clear Selection" ) )
    {
      m_selectedIndex = -1;
    }
    ImGui::EndPopup();
  }
}

void SceneHierarchyWindow::drawItemContexMenu( const std::string& itemId, Entity& ent )
{
  if ( ImGui::BeginPopupContextItem( itemId.c_str() ) )
  {
    if ( ImGui::MenuItem( "Delete entity" ) )
    {
      auto pScene = SceneManager::getCurrentScene();
      if ( auto scene = pScene.lock() )
      {
        scene->removeEntity( ent.getEnttEntity() );
      }
    }
    ImGui::EndPopup();
  }
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