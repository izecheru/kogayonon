#include "gui/entity_properties.hpp"
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_stdlib.h>
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/model_component.hpp"
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
    : ImGuiWindow{ std::move( name ), ImGuiWindowFlags_None, { 300.0f, 300.0f } }
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
      if ( !entity.hasComponent<ModelComponent>() )
      {
        if ( ImGui::MenuItem( "Model component" ) )
        {
          entity.addComponent<ModelComponent>();
        }
      }
      if ( ImGui::MenuItem( "Texture component" ) )
      {
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }

  if ( entity.hasComponent<ModelComponent>() )
  {
    ImGui::SeparatorText( "Model" );
    drawModelComponent( entity );
  }

  if ( entity.hasComponent<TransformComponent>() )
  {
    ImGui::SeparatorText( "Transform" );
    drawTransformComponent( entity );
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
  const auto& pModelComponent = ent.tryGetComponent<ModelComponent>();
  if ( !pModelComponent )
    return;

  auto& meshes = pModelComponent->pModel->getMeshes();
  if ( meshes.empty() )
    return;

  ImGui::BeginGroup();
  if ( ImGui::BeginDragDropTarget() )
  {
    manageTexturePayload( ImGui::AcceptDragDropPayload( "ASSET_DROP" ) );
    ImGui::EndDragDropTarget();
  }

  ImGui::BeginListBox( "##texture_list" );
  std::unordered_set<uint32_t> seen;
  for ( auto& mesh : meshes )
  {
    auto& textures = mesh.getTextures();
    for ( int i = 0; i < textures.size(); i++ )
    {
      if ( const auto& texture = textures.at( i ); seen.insert( texture->getTextureId() ).second )
      {
        ImGui::Text( "%s", texture->getName().c_str() );
        if ( ImGui::IsItemHovered() )
        {
          ImGui::BeginTooltip();
          if ( texture )
          {
            ImGui::Image( (ImTextureID)texture->getTextureId(), ImVec2{ 100.0f, 100.0f } );
          }
          ImGui::EndTooltip();
        }
      }

      drawTextureContextMenu( textures, i );
    }
  }

  ImGui::EndListBox();
  ImGui::EndGroup();
}

void EntityPropertiesWindow::manageTexturePayload( const ImGuiPayload* payload ) const
{
  if ( !payload )
    return;

  const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();
  auto data = static_cast<const char*>( payload->Data );
  std::string dropResult( data, payload->DataSize );
  std::filesystem::path p{ dropResult };
  std::string extension = p.extension().string();

  // sometimes the file extension check would fail because of the hidden null character
  // so we clear it
  if ( !extension.empty() && extension.back() == '\0' )
  {
    extension.pop_back();
  }

  if ( extension != ".png" && extension != ".jpg" )
  {
    spdlog::info( "format currently unsupported {}", p.extension().string() );
    return;
  }

  auto pTexture = pAssetManager->addTexture( p.filename().string(), p.string() );
  auto scene = SceneManager::getCurrentScene().lock();
  Entity ent{ scene->getRegistry(), m_entity };

  if ( const auto& model = ent.tryGetComponent<ModelComponent>() )
  {
    auto& meshes = model->pModel->getMeshes();
    for ( auto& mesh : meshes )
    {
      // get the textures vector for each mesh
      auto& textures = mesh.getTextures();

      // add it to the vector
      textures.push_back( pTexture );
    }
  }
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

void EntityPropertiesWindow::drawModelComponent( Entity& ent )
{
  auto pModelComponent = ent.tryGetComponent<ModelComponent>();
  if ( ImGui::BeginDragDropTarget() )
  {
    // if we have a payload
    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ASSET_DROP" );
    manageModelPayload( payload );
    ImGui::EndDragDropTarget();
  }

  auto model = pModelComponent->pModel;

  if ( !model )
    return;

  Entity entity{ SceneManager::getCurrentScene().lock()->getRegistry(), m_entity };

  ImGui::Text( "Model has %d meshes", model->getMeshes().size() );
  ImGui::TextWrapped( "Path: %s", model->getPath().c_str() );

  if ( ImGui::Button( "Remove model" ) )
  {
    auto scene = SceneManager::getCurrentScene().lock();
    if ( !scene )
      return;

    scene->removeModelFromEntity( ent.getEntityId() );
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
    pTaskManager->enqueue( [entTemp, p, pScene]() {
      const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();
      const auto model = pAssetManager->addModel( p.filename().string(), p.string() );
      Entity entity{ pScene->getRegistry(), entTemp };
      pScene->addModelToEntity( entity.getEntityId(), model );
    } );
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

  // if it has transform, it definetely has a model component
  const auto& modelComponent = ent.tryGetComponent<ModelComponent>();

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
      const auto data = scene->getData( modelComponent->pModel );

      // update the matrix in the instance matrices vector
      ImGuizmo::RecomposeMatrixFromComponents( glm::value_ptr( translation ), glm::value_ptr( rotation ),
                                               glm::value_ptr( scale ),
                                               glm::value_ptr( data->instanceMatrices.at( indexComponent.index ) ) );

      scene->setupMultipleInstances( data );
    }
  }
}
} // namespace kogayonon_gui