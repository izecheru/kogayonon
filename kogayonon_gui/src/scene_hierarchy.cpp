#include "gui/scene_hierarchy.hpp"
#include <SDL.h>
#include <spdlog/spdlog.h>
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/outline_component.hpp"
#include "core/ecs/components/pointlight_component.hpp"

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

using namespace kogayonon_core;

namespace kogayonon_gui
{
SceneHierarchyWindow::SceneHierarchyWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_selectedEntity{ entt::null }
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<KeyPressedEvent, &SceneHierarchyWindow::onKeyPressed>( *this );
  pEventDispatcher->addHandler<SelectEntityEvent, &SceneHierarchyWindow::onEntitySelect>( *this );
}

void SceneHierarchyWindow::onEntitySelect( const SelectEntityEvent& e )
{
  if ( m_selectedEntity == e.getEntityId() || e.getEventSource() == SelectEntityEventSource::HierarchyWindow )
    return;

  m_selectedEntity = e.getEntityId();
}

void SceneHierarchyWindow::onKeyPressed( const KeyPressedEvent& e )
{
  if ( m_selectedEntity == entt::null )
    return;

  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  const auto& scene = SceneManager::getCurrentScene().lock();
  // Escape to deselect entity
  if ( e.getKeyScanCode() == KeyScanCode::Escape && e.getKeyModifier() == KeyScanCode::None )
  {
    m_selectedEntity = entt::null;
    scene->removeOutline();
    pEventDispatcher->dispatchEvent( SelectEntityEvent{} );
  }

  // Delete to delete entity
  if ( e.getKeyScanCode() == KeyScanCode::Delete && e.getKeyModifier() == KeyScanCode::None )
  {
    scene->removeOutline();
    scene->removeEntity( m_selectedEntity );
    m_selectedEntity = entt::null;
    pEventDispatcher->dispatchEvent( SelectEntityEvent{} );
  }
}

void SceneHierarchyWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };

  if ( !begin() )
    return;

  auto& assetManager = AssetManager::getInstance();
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();

  static auto cubeIcon = assetManager.getTexture( "3d-cube.png" );

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
      Entity entity{ scene->getRegistry(), scene->addEntity() };
      m_selectedEntity = entity.getEntityId();
      pEventDispatcher->dispatchEvent(
        SelectEntityEvent{ m_selectedEntity, SelectEntityEventSource::HierarchyWindow } );
    }
  }

  auto& enttRegistry = scene->getEnttRegistry();
  auto view = enttRegistry.view<IdentifierComponent>();

  drawContextMenu();

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

    view.each( [&]( const auto& entityId, auto& identifierComponent ) {
      // first column
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      std::string selectableId = std::format( "##{}{}", identifierComponent.name, static_cast<uint32_t>( entityId ) );

      ImGui::BeginGroup();

      bool selected = m_selectedEntity == entityId;
      auto hoverColor = ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered];
      auto normalColor = ImGui::GetStyle().Colors[ImGuiCol_Header];

      ImGui::PushStyleColor( ImGuiCol_Header, selected ? hoverColor : normalColor );

      if ( ImGui::Selectable( selectableId.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns ) )
      {
        m_selectedEntity = entityId;
        Entity entity{ scene->getRegistry(), entityId };
        pEventDispatcher->dispatchEvent( SelectEntityEvent{ entityId, SelectEntityEventSource::HierarchyWindow } );
      }
      ImGui::PopStyleColor();
      drawItemContexMenu( selectableId, entityId );

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
    } );
    ImGui::EndTable();
  }

  drawContextMenu();
  ImGui::EndChild();
  ImGui::End();
}

void SceneHierarchyWindow::drawContextMenu()
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  const auto scene = SceneManager::getCurrentScene().lock();

  if ( ImGui::BeginPopupContextWindow( "SceneHierarchyContext",
                                       ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight ) )
  {
    if ( ImGui::MenuItem( "Mesh" ) )
    {
      Entity entity{ scene->getRegistry(), scene->addEntity() };
      entity.setType( EntityType::Object );
      entity.addComponent<MeshComponent>();
      m_selectedEntity = entity.getEntityId();
      pEventDispatcher->dispatchEvent(
        SelectEntityEvent{ m_selectedEntity, SelectEntityEventSource::HierarchyWindow } );
    }

    if ( ImGui::MenuItem( "Point light" ) )
    {
      Entity entity{ scene->getRegistry(), scene->addEntity() };
      entity.setType( EntityType::Light );
      scene->addPointLight( entity.getEntityId() );
      spdlog::info( "{} number of point lights", scene->getLightCount( kogayonon_resources::LightType::Point ) );
      m_selectedEntity = entity.getEntityId();
      scene->updateLightBuffers();
      pEventDispatcher->dispatchEvent(
        SelectEntityEvent{ m_selectedEntity, SelectEntityEventSource::HierarchyWindow } );
    }

    if ( ImGui::MenuItem( "Directional light" ) )
    {
      Entity entity{ scene->getRegistry(), scene->addEntity() };
      entity.setType( EntityType::Light );
      scene->addDirectionalLight( entity.getEntityId() );
      m_selectedEntity = entity.getEntityId();
      scene->updateLightBuffers();
      spdlog::info( "{} number of directional lights",
                    scene->getLightCount( kogayonon_resources::LightType::Directional ) );
      pEventDispatcher->dispatchEvent(
        SelectEntityEvent{ m_selectedEntity, SelectEntityEventSource::HierarchyWindow } );
    }

    ImGui::EndPopup();
  }
}

void SceneHierarchyWindow::duplicateEntity( Entity& ent )
{
  auto pScene = SceneManager::getCurrentScene();
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  if ( auto scene = pScene.lock() )
  {
    Entity entity{ scene->getRegistry(), scene->addEntity() };
    if ( ent.hasComponent<MeshComponent>() )
    {
      const auto& meshComponent = ent.getComponent<MeshComponent>();
      const auto& transform = ent.getComponent<TransformComponent>();
      entity.addComponent<TransformComponent>( TransformComponent{
        .translation = transform.translation, .rotation = transform.rotation, .scale = transform.scale } );
      scene->addMeshToEntity( entity.getEntityId(), meshComponent.pMesh );
    }
    if ( ent.hasComponent<PointLightComponent>() )
    {
      const auto& pointLightComp = ent.getComponent<PointLightComponent>();
      scene->addPointLight( entity.getEntityId() );
      const auto& pointLight = entity.getComponent<PointLightComponent>();
    }
    pEventDispatcher->dispatchEvent(
      SelectEntityEvent{ entity.getEntityId(), SelectEntityEventSource::HierarchyWindow } );
  }
}

void SceneHierarchyWindow::deleteEntity( kogayonon_core::Entity& ent )
{
  auto pScene = SceneManager::getCurrentScene();
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  if ( auto scene = pScene.lock() )
  {
    scene->removeEntity( ent.getEntityId() );
    m_selectedEntity = entt::null;
    pEventDispatcher->dispatchEvent( SelectEntityEvent{} );
  }
}

void SceneHierarchyWindow::drawItemContexMenu( const std::string& itemId, entt::entity entity )
{
  auto scene = SceneManager::getCurrentScene().lock();
  Entity ent{ scene->getRegistry(), entity };
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  if ( ImGui::BeginPopupContextItem( itemId.c_str() ) )
  {
    if ( ImGui::MenuItem( "Duplicate entity" ) )
    {
      duplicateEntity( ent );
    }

    if ( ImGui::MenuItem( "Delete entity" ) )
    {
      deleteEntity( ent );
    }

    ImGui::EndPopup();
  }
}
} // namespace kogayonon_gui