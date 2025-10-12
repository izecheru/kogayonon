#include "gui/entity_properties.hpp"
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

using namespace kogayonon_utilities;
using namespace kogayonon_core;

namespace kogayonon_gui
{
EntityPropertiesWindow::EntityPropertiesWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_entity{ entt::null }
{
  EVENT_DISPATCHER()->addHandler<SelectEntityEvent, &EntityPropertiesWindow::onEntitySelect>( *this );
  EVENT_DISPATCHER()->addHandler<SelectEntityInViewportEvent, &EntityPropertiesWindow::onSelectEntityInViewport>(
    *this );
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
    ImGui::InputText( "##id", &pIdentifierComponent->name );
    if ( ImGui::BeginCombo( "##entity_type", "Change entity type" ) )
    {
      if ( ImGui::MenuItem( "Entity" ) )
      {
      }
      ImGui::EndCombo();
    }
  }

  if ( ImGui::BeginCombo( "##combo", "Add component" ) )
  {
    if ( !entity.hasComponent<ModelComponent>() )
    {
      if ( ImGui::MenuItem( "Model component" ) )
      {
      }
    }

    if ( !entity.hasComponent<TextureComponent>() )
    {
      if ( ImGui::MenuItem( "Texture component" ) )
      {
      }
    }

    ImGui::EndCombo();
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

  auto& meshes = pModelComponent->pModel.lock()->getMeshes();
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
      if ( const auto& texture = textures.at( i ).lock(); seen.insert( texture->getTextureId() ).second )
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

  const auto& pAssetManager = ASSET_MANAGER();
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

  auto pTexture = ASSET_MANAGER()->addTexture( p.filename().string(), p.string() );
  auto scene = SceneManager::getCurrentScene().lock();
  Entity ent{ scene->getRegistry(), m_entity };

  if ( const auto& model = ent.tryGetComponent<ModelComponent>() )
  {
    auto& meshes = model->pModel.lock()->getMeshes();
    for ( auto& mesh : meshes )
    {
      // get the textures vector for each mesh
      auto& textures = mesh.getTextures();

      // add it to the vector
      textures.push_back( pTexture );
    }
  }
}

void EntityPropertiesWindow::drawTextureContextMenu( std::vector<std::weak_ptr<kogayonon_resources::Texture>>& textures,
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

void EntityPropertiesWindow::drawModelComponent( Entity& ent ) const
{
  auto pModelComponent = ent.tryGetComponent<ModelComponent>();
  if ( !pModelComponent )
  {
    if ( ImGui::BeginDragDropTarget() )
    {
      // if we have a payload
      const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ASSET_DROP" );
      manageModelPayload( payload );
      ImGui::EndDragDropTarget();
    }
    return;
  }

  auto model = pModelComponent->pModel.lock();

  if ( !model )
    return;

  ImGui::Text( "Model has %d meshes", model->getMeshes().size() );
  ImGui::TextWrapped( "Path: %s", model->getPath().c_str() );

  if ( ImGui::Button( "Remove model" ) )
  {
    auto scene = SceneManager::getCurrentScene().lock();
    if ( !scene )
      return;

    scene->removeModelFromEntity( ent.getEntityId(), model );
  }
}

void EntityPropertiesWindow::manageModelPayload( const ImGuiPayload* payload ) const
{
  if ( !payload )
  {
    return;
  }

  const auto& pAssetManager = ASSET_MANAGER();
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

    auto model = ASSET_MANAGER()->addModel( p.filename().string(), p.string() );
    scene.lock()->addModelToEntity( m_entity, model );
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

  ImGui::Text( "Transform" );

  // if it has transform, it definetely has a model component
  const auto& modelComponent = ent.tryGetComponent<ModelComponent>();

  bool changed = false;
  auto& pos = transformComponent->pos;
  auto& scale = transformComponent->scale;
  changed |= ImGui::SliderFloat( "x", &pos.x, -100.0f, 100.0f );
  changed |= ImGui::SliderFloat( "y", &pos.y, -100.0f, 100.0f );
  changed |= ImGui::SliderFloat( "z", &pos.z, -100.0f, 100.0f );

  ImGui::Text( "Scale" );
  changed |= ImGui::SliderFloat( "x##scale", &scale.x, 1.0f, 100.0f );
  changed |= ImGui::SliderFloat( "y##scale", &scale.y, 1.0f, 100.0f );
  changed |= ImGui::SliderFloat( "z##scale", &scale.z, 1.0f, 100.0f );

  if ( changed )
  {
    auto scene = SceneManager::getCurrentScene().lock();

    if ( !scene )
      return;

    transformComponent->updateMatrix();

    // we need the index of the instance
    const auto& indexComponent = ent.getComponent<IndexComponent>();

    // get the instance data of this model
    const auto data = scene->getData( modelComponent->pModel.lock().get() );

    // update the matrix in the instance matrices vector
    data->instanceMatrices.at( indexComponent.index ) = transformComponent->modelMatrix;

    // if we have a multiple instances
    if ( data->count >= 1 )
    {
      scene->setupMultipleInstances( data );
    }
  }
}
} // namespace kogayonon_gui