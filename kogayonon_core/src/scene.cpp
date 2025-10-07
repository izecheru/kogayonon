#include "core/scene/scene.hpp"
#include <glad/glad.h>
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/registry.hpp"
#include "utilities/math/math.hpp"
using namespace kogayonon_utilities;

namespace kogayonon_core
{
Scene::Scene( const std::string& name )
    : m_entityCount{ 0 }
    , m_name{ name }
    , m_pRegistry{ std::make_unique<Registry>() }

{
}

Registry& Scene::getRegistry()
{
  return *m_pRegistry.get();
}

entt::registry& Scene::getEnttRegistry()
{
  return m_pRegistry->getRegistry();
}

std::string Scene::getName() const
{
  return m_name;
}

void Scene::changeName( const std::string& name )
{
  m_name = name;
}

void Scene::removeEntity( entt::entity ent )
{
  auto& registry = m_pRegistry->getRegistry();

  // upon deletion we must remove that specific instance
  if ( Entity entity{ *m_pRegistry, ent }; entity.hasComponent<ModelComponent>() )
  {
    const auto& modelComponent = entity.getComponent<ModelComponent>();

    // if we have instances
    if ( auto pModel = modelComponent.pModel.lock().get(); m_instances.contains( pModel ) )
    {
      const auto& instance = m_instances.at( pModel );

      // decrease count
      --instance->count;

      // get the index component and then erase that specific instance matrix
      const auto& indexComponent = entity.getComponent<IndexComponent>();
      auto toErase = indexComponent.index;
      instance->instanceMatrices.erase( instance->instanceMatrices.begin() + toErase );
      instance->entityIds.erase( instance->entityIds.begin() + toErase );

      // now we need to update all the index components that are to the right of the deleted index
      for ( const auto& [entity, indexComp] : m_pRegistry->getRegistry().view<IndexComponent>().each() )
      {
        Entity ent{ *m_pRegistry, entity };

        // if the index from the iteration is >= to the index component we erased from the instances
        if ( indexComp.index >= toErase )
        {
          // we subtract one since we deleted it above
          --indexComp.index;
        }
      }

      // finally set up instances again to update the instance buffer
      setupMultipleInstances( instance.get() );
    }
  }

  if ( registry.valid( ent ) )
    registry.destroy( ent );

  --m_entityCount;
}

void Scene::addEntity()
{
  Entity ent{ *m_pRegistry, "DefaultEntity" };
  ++m_entityCount;
}

constexpr float scale = 10.0f;

void Scene::addEntity( std::weak_ptr<kogayonon_resources::Model> pModel )
{
  Entity ent{ *m_pRegistry, "ModelEntity" };
  ent.addComponent<kogayonon_core::ModelComponent>( ModelComponent{ .pModel = pModel, .loaded = true } );
  ent.addComponent<kogayonon_core::TransformComponent>();
  ++m_entityCount;

  // now that we created the entity using the scene registry and added a model and transform components to it
  // we must look for model pointer in the map to check for instances
  if ( m_instances.contains( pModel.lock().get() ) )
  {
    if ( const auto& instanceData = m_instances.at( pModel.lock().get() ) )
    {
      // we increment the amount of models we need to render
      ++instanceData->count;

      // we create a new instance matrix
      const auto& transform = ent.getComponent<TransformComponent>();
      instanceData->instanceMatrices.push_back(
        math::computeModelMatrix( transform.pos, transform.rotation, transform.scale ) );

      // first we get the index of the instance matrix
      uint32_t size = instanceData->instanceMatrices.size();
      ent.addComponent<kogayonon_core::IndexComponent>( IndexComponent{ .index = size - 1 } );

      instanceData->entityIds.push_back( static_cast<uint32_t>( ent.getEnttEntity() ) );

      setupMultipleInstances( instanceData.get() );
    }
  }
  else
  {
    const auto& transform = ent.getComponent<TransformComponent>();
    auto instanceData = std::make_unique<InstanceData>( InstanceData{
      .entityIdBuffer = 0,
      .entityIds = { static_cast<uint32_t>( ent.getEnttEntity() ) },
      .instanceBuffer = 0,
      .instanceMatrices = { math::computeModelMatrix( transform.pos, transform.rotation, transform.scale ) },
      .count = 1,
      .pModel = pModel.lock().get() } );

    // first we get the index of the instance matrix
    uint32_t size = instanceData->instanceMatrices.size();
    ent.addComponent<kogayonon_core::IndexComponent>( IndexComponent{ .index = size - 1 } );

    setupMultipleInstances( instanceData.get() );
    m_instances.try_emplace( pModel.lock().get(), std::move( instanceData ) );
  }
}

InstanceData* Scene::getData( kogayonon_resources::Model* pModel )
{
  if ( m_instances.find( pModel ) == m_instances.end() )
    return nullptr;

  return m_instances.at( pModel ).get();
}

void Scene::setupMultipleInstances( InstanceData* data )
{
  if ( data->instanceBuffer == 0 )
    glCreateBuffers( 1, &data->instanceBuffer );

  if ( data->entityIdBuffer == 0 )
    glCreateBuffers( 1, &data->entityIdBuffer );

  if ( data->instanceBuffer != 0 && data->entityIdBuffer != 0 )
  {
    // did not use glNamedBufferStorage cause it is immutable and instances change based on the amount of them
    glNamedBufferData( data->instanceBuffer, sizeof( glm::mat4 ) * data->count, data->instanceMatrices.data(),
                       GL_DYNAMIC_DRAW );

    glNamedBufferData( data->entityIdBuffer, sizeof( uint32_t ) * data->count, data->entityIds.data(),
                       GL_DYNAMIC_DRAW );

    auto& meshes = data->pModel->getMeshes();
    for ( int i = 0; i < meshes.size(); i++ )
    {
      const auto& vao = meshes.at( i ).getVao();

      glVertexArrayVertexBuffer( vao, 1, data->instanceBuffer, 0, sizeof( glm::mat4 ) );
      glVertexArrayVertexBuffer( vao, 2, data->entityIdBuffer, 0, sizeof( uint32_t ) );

      glEnableVertexArrayAttrib( vao, 3 );
      glEnableVertexArrayAttrib( vao, 4 );
      glEnableVertexArrayAttrib( vao, 5 );
      glEnableVertexArrayAttrib( vao, 6 );
      glEnableVertexArrayAttrib( vao, 7 );

      glVertexArrayAttribFormat( vao, 3, 4, GL_FLOAT, GL_FALSE, 0 );
      glVertexArrayAttribFormat( vao, 4, 4, GL_FLOAT, GL_FALSE, sizeof( glm::vec4 ) );
      glVertexArrayAttribFormat( vao, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof( glm::vec4 ) );
      glVertexArrayAttribFormat( vao, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof( glm::vec4 ) );
      glVertexArrayAttribIFormat( vao, 7, 2, GL_UNSIGNED_INT, 0 );

      glVertexArrayAttribBinding( vao, 3, 1 );
      glVertexArrayAttribBinding( vao, 4, 1 );
      glVertexArrayAttribBinding( vao, 5, 1 );
      glVertexArrayAttribBinding( vao, 6, 1 );
      glVertexArrayAttribBinding( vao, 7, 2 );

      glVertexArrayBindingDivisor( vao, 1, 1 );
      glVertexArrayBindingDivisor( vao, 2, 1 );
    }
  }
  else
  {
    // Resize existing buffer (optional, only if m_amount changed)
    glNamedBufferData( data->instanceBuffer, sizeof( glm::mat4 ) * data->count, nullptr, GL_DYNAMIC_DRAW );
    glNamedBufferData( data->entityIdBuffer, sizeof( uint32_t ) * data->count, nullptr, GL_DYNAMIC_DRAW );

    // Upload new data
    glNamedBufferSubData( data->instanceBuffer, 0, sizeof( glm::mat4 ) * data->count, data->instanceMatrices.data() );
    glNamedBufferSubData( data->entityIdBuffer, 0, sizeof( uint32_t ) * data->count, data->entityIds.data() );
  }
}

void Scene::setupInstance( InstanceData* data )
{
}
} // namespace kogayonon_core