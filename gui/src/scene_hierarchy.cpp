#include "gui/imgui_windows/scene_hierarchy.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/utils/font_keys.hpp"
#include "utilities/utils/utils.hpp"

using namespace core;

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

  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();

  initProps();

  auto scene = SceneManager::getCurrentScene().lock();

  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  // if there is no scene to render return
  if ( !scene )
  {
    ImGui::Text( "No scene to render..." );
    ImGui::PopFont();
    ImGui::End();
    return;
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
      ImGui::SetCursorScreenPos( { selectablePosMin.x + ImGui::GetStyle().FramePadding.x,
                                   selectablePosMin.y + ( selectableHeight - iconSize.y ) * 0.5f } );

      // draw the icon
      ImGui::Image( m_spec.cubeIcon, { 15.0f, 15.0f } );
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

  ImGui::PopFont();
  ImGui::End();
}

void gui::SceneHierarchy::drawItemContexMenu( const std::string& itemId, entt::entity ent )
{
  auto scene = SceneManager::getCurrentScene().lock();
  Entity entity{ scene->getRegistry(), ent };
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  if ( ImGui::BeginPopupContextItem( itemId.c_str() ) )
  {

    if ( ImGui::MenuItem( "Delete entity" ) )
    {
      // first deselect entity
      auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
      pEventDispatcher->dispatchEvent<SelectEntityEvent>(
        SelectEntityEvent{ SelectEntityEventSource::HierarchyWindow } );

      scene->removeEntity( entity.getEntityId() );
    }

    ImGui::EndPopup();
  }
}

void gui::SceneHierarchy::drawContextMenu()
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  const auto scene = SceneManager::getCurrentScene().lock();
  auto& assetManager = AssetManager::getInstance();

  if ( ImGui::BeginPopupContextWindow( "##SceneHierarchyContext",
                                       ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight ) )
  {
    if ( ImGui::MenuItem( "Default entity" ) )
    {
      Entity ent{ scene->getRegistry(), "DefaultEnt" };

      pEventDispatcher->dispatchEvent<SelectEntityEvent>(
        SelectEntityEvent{ ent.getEntityId(), SelectEntityEventSource::HierarchyWindow } );

      m_selectedEntity = ent.getEntityId();
    }

    if ( ImGui::MenuItem( "Create object" ) )
    {
      std::filesystem::path p{ std::filesystem::absolute( "." ) / "engine_resources\\models\\default_cone.gltf" };

      Entity ent{ scene->getRegistry(), "ObjectEntity" };

      ent.addComponent<TransformComponent>( TransformComponent{} );
      ent.addComponent<MeshComponent>(
        MeshComponent{ .pMesh = assetManager.loadMesh( "test", p.string() ), .loaded = true } );

      pEventDispatcher->dispatchEvent<SelectEntityEvent>(
        SelectEntityEvent{ ent.getEntityId(), SelectEntityEventSource::HierarchyWindow } );

      m_selectedEntity = ent.getEntityId();
    }

    ImGui::EndPopup();
  }
}
