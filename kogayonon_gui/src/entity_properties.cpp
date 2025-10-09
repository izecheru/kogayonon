#include "gui/entity_properties.hpp"
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/name_component.hpp"
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

namespace kogayonon_gui
{
EntityPropertiesWindow::EntityPropertiesWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_entity{ entt::null }
{
  EVENT_DISPATCHER()->addHandler<kogayonon_core::SelectEntityEvent, &EntityPropertiesWindow::onEntitySelect>( *this );
  EVENT_DISPATCHER()
    ->addHandler<kogayonon_core::SelectEntityInViewportEvent, &EntityPropertiesWindow::onSelectEntityInViewport>(
      *this );
}

void EntityPropertiesWindow::onSelectEntityInViewport( const kogayonon_core::SelectEntityInViewportEvent& e )
{
  if ( m_entity == e.getEntity() )
  {
    return;
  }
  m_entity = e.getEntity();
}

void EntityPropertiesWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };

  if ( !begin() )
    return;

  auto pScene = kogayonon_core::SceneManager::getCurrentScene();
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

void EntityPropertiesWindow::drawEnttProperties( std::shared_ptr<kogayonon_core::Scene> scene )
{
  kogayonon_core::Entity entity{ scene->getRegistry(), m_entity };

  // entity always has a name
  auto pNameComp = entity.tryGetComponent<kogayonon_core::NameComponent>();
  if ( pNameComp )
  {
    // change the entity name
    ImGui::TextUnformatted( "Change name" );
    ImGui::SameLine();
    if ( char buffer[50] = { 0 };
         ImGui::InputText( "##change_name", buffer, IM_ARRAYSIZE( buffer ), ImGuiInputTextFlags_EnterReturnsTrue ) )
    {
      std::string result{ buffer };
      pNameComp->name = result;
    }
  }

  // has textures?
  ImGui_Utils::addPaddedGui( [this, &entity]() { drawTextureComponent( entity ); }, ImVec2{ 1.0f, 10.0f } );

  // has transform component?
  ImGui_Utils::addPaddedGui( [this, &entity]() { drawTransformComponent( entity ); }, ImVec2{ 1.0f, 10.0f } );

  // has model component?
  ImGui_Utils::addPaddedGui( [this, &entity]() { drawModelComponent( entity ); }, ImVec2{ 1.0f, 10.0f } );
}

void EntityPropertiesWindow::onEntitySelect( const kogayonon_core::SelectEntityEvent& e )
{
  m_entity = e.getEntity();
}

void EntityPropertiesWindow::drawTextureComponent( kogayonon_core::Entity& ent ) const
{
  const auto& pModelComponent = ent.tryGetComponent<kogayonon_core::ModelComponent>();
  if ( !pModelComponent )
    return;

  ImGui::Text( "Textures" );

  auto& meshes = pModelComponent->pModel.lock()->getMeshes();
  if ( meshes.empty() )
    return;

  ImGui::BeginGroup();
  if ( ImGui::BeginDragDropTarget() )
  {
    manageAssetPayload( ImGui::AcceptDragDropPayload( "ASSET_DROP" ) );
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

void EntityPropertiesWindow::manageAssetPayload( const ImGuiPayload* payload ) const
{
  if ( !payload )
    return;

  const auto& pAssetManager = ASSET_MANAGER();
  auto data = static_cast<const char*>( payload->Data );
  std::string dropResult( data, payload->DataSize );
  std::filesystem::path p{ dropResult };

  if ( p.extension().string() != ".png" || p.extension().string() != ".jpg" )
  {
    spdlog::info( "format currently unsupported {}", p.extension().string() );
    return;
  }

  auto pTexture = ASSET_MANAGER()->addTexture( p.filename().string(), p.string() );
  auto scene = kogayonon_core::SceneManager::getCurrentScene().lock();
  kogayonon_core::Entity ent{ scene->getRegistry(), m_entity };

  if ( const auto& model = ent.tryGetComponent<kogayonon_core::ModelComponent>() )
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

void EntityPropertiesWindow::drawModelComponent( kogayonon_core::Entity& ent ) const
{
  auto pModelComponent = ent.tryGetComponent<kogayonon_core::ModelComponent>();
  if ( !pModelComponent )
    return;

  ImGui::Text( "Model component" );
  auto model = pModelComponent->pModel.lock();

  if ( !model )
    return;

  ImGui::Text( "Model has %d meshes", model->getMeshes().size() );
}

void EntityPropertiesWindow::drawTransformComponent( kogayonon_core::Entity& ent ) const
{
  const auto& transformComponent = ent.tryGetComponent<kogayonon_core::TransformComponent>();
  if ( !transformComponent )
    return;

  ImGui::Text( "Transform" );

  // if it has transform, it definetely has a model component
  const auto& modelComponent = ent.tryGetComponent<kogayonon_core::ModelComponent>();

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
    auto scene = kogayonon_core::SceneManager::getCurrentScene().lock();

    if ( !scene )
      return;

    transformComponent->dirty = true;
    transformComponent->updateMatrix();

    // we need the index of the instance
    const auto& indexComponent = ent.getComponent<kogayonon_core::IndexComponent>();

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