#include "gui/entity_properties.hpp"
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_stdlib.h>
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/pointlight_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "imgui_utils/imgui_utils.h"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/math/math.hpp"
#include "utilities/task_manager/task_manager.hpp"

using namespace kogayonon_utilities;
using namespace kogayonon_core;

namespace kogayonon_gui
{
EntityPropertiesWindow::EntityPropertiesWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_entity{ entt::null }
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<SelectEntityEvent, &EntityPropertiesWindow::onEntitySelect>( *this );
  pEventDispatcher->addHandler<SelectEntityInViewportEvent, &EntityPropertiesWindow::onSelectEntityInViewport>( *this );
}

void EntityPropertiesWindow::onSelectEntityInViewport( const SelectEntityInViewportEvent& e )
{
  if ( m_entity == e.getEntity() )
    return;

  m_entity = e.getEntity();
}

void EntityPropertiesWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };

  if ( !begin() )
    return;

  auto pScene = SceneManager::getCurrentScene();
  if ( auto scene = pScene.lock() )
  {
    if ( m_entity != entt::null )
    {
      drawEnttProperties( scene );
    }
    else
    {
      ImGui::Text( "No entity is currently selected" );
    }
  }
  ImGui::End();
}

void EntityPropertiesWindow::drawEnttProperties( std::shared_ptr<Scene> scene )
{
  Entity entity{ scene->getRegistry(), m_entity };

  if ( auto pIdentifierComponent = entity.tryGetComponent<IdentifierComponent>() )
  {
    ImGui::SeparatorText( "Entity idenfitification" );
    ImGui::InputText( "##id", &pIdentifierComponent->name );
    ImGui::Text( "Group: %s", pIdentifierComponent->group.c_str() );
    ImGui::Text( "Type: %s", typeToString( pIdentifierComponent->type ).c_str() );
  }

  if ( ImGui::BeginPopupContextWindow( "##model_context" ) )
  {
    if ( ImGui::BeginMenu( "Add component" ) )
    {
      if ( !entity.hasComponent<MeshComponent>() )
      {
        if ( ImGui::MenuItem( "Mesh component" ) )
        {
          entity.addComponent<MeshComponent>();
        }
      }

      if ( !entity.hasComponent<PointLightComponent>() )
      {
        if ( ImGui::MenuItem( "Point light" ) )
        {
          scene->addPointLight( entity.getEntityId() );
        }
      }

      if ( ImGui::MenuItem( "Texture component" ) )
      {
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }

  if ( entity.hasComponent<MeshComponent>() )
  {
    ImGui::SeparatorText( "  Mesh  " );
    drawMeshComponent( entity );
  }

  if ( entity.hasComponent<TransformComponent>() )
  {
    ImGui::SeparatorText( "  Transform  " );
    drawTransformComponent( entity );
  }

  if ( entity.hasComponent<PointLightComponent>() )
  {
    ImGui::SeparatorText( "  Point light " );
    drawPointLightComponent( entity );
  }
}

void EntityPropertiesWindow::onEntitySelect( const SelectEntityEvent& e )
{
  if ( m_entity == e.getEntity() )
    return;

  m_entity = e.getEntity();
}

void EntityPropertiesWindow::drawTextureComponent( Entity& ent ) const
{
  const auto& pMeshComponent = ent.tryGetComponent<MeshComponent>();
  if ( !pMeshComponent )
    return;

  auto& mesh = pMeshComponent->pMesh;
  if ( !mesh )
    return;

  ImGui::BeginGroup();
  if ( ImGui::BeginDragDropTarget() )
  {
    manageTexturePayload( ImGui::AcceptDragDropPayload( "ASSET_DROP" ) );
    ImGui::EndDragDropTarget();
  }

  ImGui::BeginListBox( "##texture_list" );
  ImGui::EndListBox();
  ImGui::EndGroup();
}

void EntityPropertiesWindow::manageTexturePayload( const ImGuiPayload* payload ) const
{
}

void EntityPropertiesWindow::drawTextureContextMenu( std::vector<kogayonon_resources::Texture*>& textures,
                                                     int index ) const
{
  std::string label = std::format( "##{}", index );
  if ( ImGui::BeginPopupContextItem( label.c_str() ) )
  {
    if ( ImGui::MenuItem( "Delete texture" ) )
    {
      textures.erase( textures.begin() + index );
    }
    ImGui::EndPopup();
  }
}

void EntityPropertiesWindow::drawMeshComponent( Entity& ent )
{
  auto pMeshComponent = ent.tryGetComponent<MeshComponent>();
  if ( ImGui::BeginDragDropTarget() )
  {
    // if we have a payload
    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ASSET_DROP" );
    manageModelPayload( payload );
    ImGui::EndDragDropTarget();
  }

  auto& mesh = pMeshComponent->pMesh;

  if ( !mesh )
    return;

  Entity entity{ SceneManager::getCurrentScene().lock()->getRegistry(), m_entity };

  ImGui::Text( "Mesh has %d submeshes", mesh->getSubmeshes().size() );
  ImGui::TextWrapped( "Path: %s", mesh->getPath().c_str() );

  if ( ImGui::Button( "Remove mesh" ) )
  {
    auto scene = SceneManager::getCurrentScene().lock();
    if ( !scene )
      return;

    scene->removeMeshFromEntity( ent.getEntityId() );
  }
}

void EntityPropertiesWindow::manageModelPayload( const ImGuiPayload* payload )
{
  if ( !payload )
  {
    return;
  }

  const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();
  auto data = static_cast<const char*>( payload->Data );
  std::string dropResult( data, payload->DataSize );
  std::filesystem::path p{ dropResult };

  auto scene = SceneManager::getCurrentScene();
  auto pScene = scene.lock();

  const auto& extension = p.extension().string();
  if ( extension.find( ".gltf" ) != std::string::npos )
  {
    spdlog::info( "loaded {}", dropResult, p.extension().string() );
    if ( !pScene )
      return;

    const auto& pTaskManager = MainRegistry::getInstance().getTaskManager();

    auto entTemp = m_entity;
    Entity ent{ pScene->getRegistry(), m_entity };
    if ( ent.hasComponent<MeshComponent>() )
    {
      pScene->removeMeshFromEntity( ent.getEntityId() );
    }
    else
    {
      ent.addComponent<MeshComponent>();
    }

    const auto model = pAssetManager->getMesh( p.filename().string() );
    if ( model != nullptr )
    {
      pScene->addMeshToEntity( entTemp, model );
    }
    else
    {
      pTaskManager->enqueue( [entTemp, p, pScene]() {
        const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();
        const auto model = pAssetManager->addMesh( p.filename().string(), p.string() );
        Entity ent{ pScene->getRegistry(), entTemp };
        pScene->addMeshToEntity( entTemp, model );
      } );
    }
  }
  else
  {
    spdlog::info( "format currently unsupported in viewport: {}", p.extension().string() );
  }
}

void EntityPropertiesWindow::drawTransformComponent( Entity& ent ) const
{
  const auto& transformComponent = ent.tryGetComponent<TransformComponent>();
  if ( !transformComponent )
    return;

  // if it has transform, it definetely has a mesh component
  const auto& modelComponent = ent.tryGetComponent<MeshComponent>();

  bool changed = false;
  auto& translation = transformComponent->translation;
  auto& scale = transformComponent->scale;

  // since the first two are the largest we make the width = largest size
  // position
  // rotation
  // scale
  static auto textSize = ImGui::CalcTextSize( "Translation" );

  if ( ImGui::BeginTable( "##transform_table", 4 ) )
  {
    ImGui::TableSetupColumn( "label", ImGuiTableColumnFlags_WidthFixed, textSize.x );
    ImGui::TableSetupColumn( "x", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "y", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "z", ImGuiTableColumnFlags_WidthFixed, 100.0f );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Translation" );

    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xtranslation", &translation.x, 0.1f, translation.x - 100.0f, translation.x + 100.0f,
                                 "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ytranslation", &translation.y, 0.1f, translation.y - 100.0f, translation.y + 100.0f,
                                 "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ztranslation", &translation.z, 0.1f, translation.z - 100.0f, translation.z + 100.0f,
                                 "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Scale" );

    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xscale", &scale.x, 0.1f, 0.0f, 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Yscale", &scale.y, 0.1f, 0.0f, 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Zscale", &scale.z, 0.1f, 0.0f, 100.0f, "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Rotation" );

    auto& rotation = transformComponent->rotation;

    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xrotation", &rotation.x, 0.1f, -180.0f, 180.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Yrotation", &rotation.y, 0.1f, -180.0f, 180.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Zrotation", &rotation.z, 0.1f, -180.0f, 180.0f, "%.2f" );

    ImGui::EndTable();

    if ( changed )
    {
      auto scene = SceneManager::getCurrentScene().lock();

      if ( !scene )
        return;

      // we need the index of the instance
      const auto& indexComponent = ent.getComponent<IndexComponent>();

      // get the instance data of this model
      const auto data = scene->getData( modelComponent->pMesh );

      // update the matrix in the instance matrices vector
      ImGuizmo::RecomposeMatrixFromComponents( glm::value_ptr( translation ), glm::value_ptr( rotation ),
                                               glm::value_ptr( scale ),
                                               glm::value_ptr( data->instanceMatrices.at( indexComponent.index ) ) );

      scene->setupMultipleInstances( data );
    }
  }
}

void EntityPropertiesWindow::drawPointLightComponent( kogayonon_core::Entity& ent ) const
{
  const auto& pPointLightComponent = ent.tryGetComponent<PointLightComponent>();
  if ( !pPointLightComponent )
    return;
  auto scene = SceneManager::getCurrentScene().lock();

  if ( !scene )
    return;

  auto& pointLight = scene->getPointLight( pPointLightComponent->pointLightIndex );

  bool changed = false;
  auto& translation = pointLight.position;
  auto& diffuse = pointLight.diffuse;
  auto& ambient = pointLight.ambient;
  auto& quadratic = pointLight.quadratic;
  auto& specular = pointLight.specular;

  static auto textSize = ImGui::CalcTextSize( "Quadratic" );

  if ( ImGui::BeginTable( "##table_pointLight", 4 ) )
  {
    ImGui::TableSetupColumn( "label", ImGuiTableColumnFlags_WidthFixed, textSize.x );
    ImGui::TableSetupColumn( "x", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "y", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "z", ImGuiTableColumnFlags_WidthFixed, 100.0f );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Translation" );

    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xtranslation", &translation.x, 0.1f, translation.x - 100.0f, translation.x + 100.0f,
                                 "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ytranslation", &translation.y, 0.1f, translation.y - 100.0f, translation.y + 100.0f,
                                 "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ztranslation", &translation.z, 0.1f, translation.z - 100.0f, translation.z + 100.0f,
                                 "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Ambient" );

    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xscale", &ambient.x, 0.1f, 0.0f, 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Yscale", &ambient.y, 0.1f, 0.0f, 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Zscale", &ambient.z, 0.1f, 0.0f, 100.0f, "%.2f" );

    ImGui::EndTable();

    if ( changed )
    {
      auto scene = SceneManager::getCurrentScene().lock();
      // update the ubo and ssbo if needed
      scene->updateLightBuffers();
    }
  }
}
} // namespace kogayonon_gui