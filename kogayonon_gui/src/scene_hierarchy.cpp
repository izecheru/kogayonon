#include "gui/scene_hierarchy.hpp"
#include <spdlog/spdlog.h>
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
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
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"

using namespace kogayonon_core;

namespace kogayonon_gui
{
SceneHierarchyWindow::SceneHierarchyWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_selectedEntity{ entt::null }
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<KeyPressedEvent, &SceneHierarchyWindow::onKeyPressed>( *this );
  pEventDispatcher->addHandler<SelectEntityInViewportEvent, &SceneHierarchyWindow::onEntitySelectInViewport>( *this );
}

void SceneHierarchyWindow::onEntitySelectInViewport( const SelectEntityInViewportEvent& e )
{
  if ( m_selectedEntity == e.getEntity() )
    return;

  m_selectedEntity = e.getEntity();
}

void SceneHierarchyWindow::onKeyPressed( const KeyPressedEvent& e )
{
  if ( e.getKeyCode() == KeyCode::Escape )
  {
    const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
    auto pTaskManager = MainRegistry::getInstance().getTaskManager();

    m_selectedEntity = entt::null;
    pEventDispatcher->emitEvent( SelectEntityEvent{} );
  }
}

void SceneHierarchyWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };

  if ( !begin() )
    return;

  const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();

  static auto cubeIcon = pAssetManager->getTexture( "3d-cube.png" );

  initProps();

  auto scene = SceneManager::getCurrentScene().lock();

  // if there is no scene to render return
  if ( !scene )
  {
    ImGui::End();
    return;
  }

  ImGui::Text( "%d entities", scene->getEntityCount() );
  ImGui::TextUnformatted( "Add entity" );
  ImGui::SameLine();
  if ( ImGui::Button( "+" ) )
  {
    if ( auto scene = SceneManager::getCurrentScene().lock() )
    {
      // no component entity
      auto entity = scene->addEntity();
      m_selectedEntity = entity.getEntityId();
      pEventDispatcher->emitEvent( SelectEntityEvent{ m_selectedEntity } );
    }
  }

  auto& enttRegistry = scene->getEnttRegistry();
  auto view = enttRegistry.view<IdentifierComponent>();
  std::vector<Entity> entities;

  for ( auto [entity, nameComponent] : view.each() )
  {
    entities.emplace_back( scene->getRegistry(), entity );
  }

  const auto& io = ImGui::GetIO();
  auto avail = ImGui::GetContentRegionAvail();

  ImGui::BeginChild( "##entity_table", avail, false, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX );
  if ( ImGui::BeginTable( "##entity_table_contents", 3, ImGuiTableFlags_Borders ) )
  {
    // table headers
    ImGui::TableSetupColumn( "name" );
    ImGui::TableSetupColumn( "type" );
    ImGui::TableSetupColumn( "group" );

    ImGui::TableHeadersRow();

    for ( int i = 0; i < entities.size(); i++ )
    {
      // first column
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      auto& entity = entities.at( i );
      const auto& identifierComponent = entity.getComponent<IdentifierComponent>();

      std::string selectableId = std::format( "##{}{}", identifierComponent.name, i );

      ImGui::BeginGroup();

      bool selected = m_selectedEntity == entity.getEntityId();
      auto hoverColor = ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered];
      auto normalColor = ImGui::GetStyle().Colors[ImGuiCol_Header];

      ImGui::PushStyleColor( ImGuiCol_Header, selected ? hoverColor : normalColor );

      if ( ImGui::Selectable( selectableId.c_str(), m_selectedEntity == entity.getEntityId(),
                              ImGuiSelectableFlags_SpanAllColumns ) )
      {
        m_selectedEntity = entity.getEntityId();
        pEventDispatcher->emitEvent( SelectEntityEvent{ m_selectedEntity } );
      }
      ImGui::PopStyleColor();
      drawItemContexMenu( selectableId, entity );

      auto labelSize = ImGui::CalcTextSize( selectableId.c_str() );
      static auto iconSize = ImVec2{ 20.0f, 20.0f };

      // get the bounds
      ImVec2 selectablePosMin = ImGui::GetItemRectMin();
      ImVec2 selectablePosMax = ImGui::GetItemRectMax();

      // calculate the height of the selectable
      float selectableHeight = std::max( iconSize.y, labelSize.y ) + ImGui::GetStyle().FramePadding.y * 2.0f;

      // set the cursor position so that it also takes into account the padding and center it vertically
      ImGui::SetCursorScreenPos( ImVec2{ selectablePosMin.x + ImGui::GetStyle().FramePadding.x,
                                         selectablePosMin.y + ( selectableHeight - iconSize.y ) * 0.5f } );

      // draw the icon
      ImGui::Image( cubeIcon.lock()->getTextureId(), ImVec2{ 15.0f, 15.0f } );
      ImGui::SameLine();

      // draw the text without ##id
      ImGui::Text( identifierComponent.name.c_str() );

      // type column
      ImGui::TableNextColumn();
      ImGui::Text( "%s", typeToString( identifierComponent.type ).c_str() );

      // group column
      ImGui::TableNextColumn();
      ImGui::Text( "%s", identifierComponent.group.c_str() );

      ImGui::EndGroup();
    }
    drawContextMenu();
    ImGui::EndTable();
  }

  ImGui::EndChild();
  ImGui::End();
}

void SceneHierarchyWindow::drawContextMenu()
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();

  if ( ImGui::BeginPopupContextWindow( "SceneHierarchyContext",
                                       ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight ) )
  {
    if ( ImGui::MenuItem( "Create Entity" ) )
    {
      if ( auto scene = SceneManager::getCurrentScene().lock() )
      {
        // no component entity
        auto entity = scene->addEntity();
        m_selectedEntity = entity.getEntityId();
        pEventDispatcher->emitEvent( SelectEntityEvent{ m_selectedEntity } );
      }
    }
    if ( ImGui::MenuItem( "Clear Selection" ) )
    {
      m_selectedEntity = entt::null;
      pEventDispatcher->emitEvent( SelectEntityEvent{} );
    }
    ImGui::EndPopup();
  }
}

void SceneHierarchyWindow::drawItemContexMenu( const std::string& itemId, Entity& ent )
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  if ( ImGui::BeginPopupContextItem( itemId.c_str() ) )
  {
    if ( ImGui::MenuItem( "Duplicate entity" ) )
    {
      auto pScene = SceneManager::getCurrentScene();
      if ( auto scene = pScene.lock() )
      {
        auto entity = scene->addEntity();
        if ( ent.hasComponent<ModelComponent>() )
        {
          const auto& modelComponent = ent.getComponent<ModelComponent>();
          const auto& transform = ent.getComponent<TransformComponent>();
          entity.addComponent<TransformComponent>( TransformComponent{
            .translation = transform.translation, .rotation = transform.rotation, .scale = transform.scale } );
          scene->addModelToEntity( entity.getEntityId(), modelComponent.pModel );
        }
        pEventDispatcher->emitEvent( SelectEntityEvent{ entity.getEntityId() } );
      }
    }

    if ( ImGui::MenuItem( "Delete entity" ) )
    {
      auto pScene = SceneManager::getCurrentScene();
      if ( auto scene = pScene.lock() )
      {
        scene->removeEntity( ent.getEntityId() );
        m_selectedEntity = entt::null;
        pEventDispatcher->emitEvent( SelectEntityEvent{} );
      }
    }

    ImGui::EndPopup();
  }
}
} // namespace kogayonon_gui