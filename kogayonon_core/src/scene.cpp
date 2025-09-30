#include "core/scene/scene.hpp"
#include <glad/glad.h>
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/registry.hpp"

namespace kogayonon_core
{
Scene::Scene( const std::string& name )
    : m_name( name )
    , m_pRegistry( std::make_unique<Registry>() )
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

      // now we need to update all the index components that are to the right of the deleted index
      for ( const auto& [entity, indexComp] : m_pRegistry->getRegistry().view<IndexComponent>().each() )
      {
        Entity ent{ *m_pRegistry, entity };

        // if the index from the iteration is >= to the index component we erased from the instances
        // if we delete entity 0 we get a higher index
        if ( indexComp.index >= toErase )
        {
          // we subtract one since we deleted it above
          --indexComp.index;
        }
      }

      setupInstance( instance.get() );
    }
  }

  if ( registry.valid( ent ) )
    registry.destroy( ent );
}

void Scene::addEntity()
{
  Entity ent{ *m_pRegistry, "DefaultEntity" };
}

constexpr float scale = 10.0f;

void Scene::addEntity( std::weak_ptr<kogayonon_resources::Model> pModel )
{
  Entity ent{ *m_pRegistry, "ModelEntity" };
  ent.addComponent<kogayonon_core::ModelComponent>( ModelComponent{ .pModel = pModel, .loaded = true } );
  ent.addComponent<kogayonon_core::TransformComponent>();

  // now that we created the entity using the scene registry and added a model and transform components to it
  // we must look for model pointer in the map to check for instances
  if ( m_instances.contains( pModel.lock().get() ) )
  {
    for ( const auto& [pModel_, instanceData] : m_instances )
    {
      if ( pModel_ == pModel.lock().get() )
      {
        // we increment the amount of models we need to render
        ++instanceData->count;

        // we create a new instance matrix
        float x = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;
        float y = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;
        float z = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;

        instanceData->instanceMatrices.emplace_back( glm::translate( glm::mat4{ 1.0f }, glm::vec3{ x, y, z } ) );

        // first we get the index of the instance matrix
        uint32_t size = instanceData->instanceMatrices.size();
        ent.addComponent<kogayonon_core::IndexComponent>( IndexComponent{ .index = size - 1 } );

        setupInstance( instanceData.get() );
      }
    }
  }
  else
  {
    // we create a new instance matrix
    float x = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;
    float y = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;
    float z = ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f ) * scale;

    auto instanceData = std::make_unique<InstanceData>(
      InstanceData{ .instanceBuffer = 0,
                    .instanceMatrices = { glm::translate( glm::mat4{ 1.0f }, glm::vec3{ x, y, z } ) },
                    .count = 1,
                    .pModel = pModel.lock().get() } );

    // first we get the index of the instance matrix
    uint32_t size = instanceData->instanceMatrices.size();
    ent.addComponent<kogayonon_core::IndexComponent>( IndexComponent{ .index = size - 1 } );

    setupInstance( instanceData.get() );
    m_instances.try_emplace( pModel.lock().get(), std::move( instanceData ) );
  }
}

InstanceData* Scene::getData( kogayonon_resources::Model* pModel )
{
  if ( m_instances.find( pModel ) == m_instances.end() )
    return nullptr;

  return m_instances.at( pModel ).get();
}

void Scene::setupInstance( InstanceData* data )
{
  if ( data->instanceBuffer == 0 )
  {
    glCreateBuffers( 1, &data->instanceBuffer );

    // did not use glNamedBufferStorage cause it is immutable and instances change based on the amount of them
    glNamedBufferData( data->instanceBuffer, sizeof( glm::mat4 ) * data->instanceMatrices.size(),
                       data->instanceMatrices.data(), GL_DYNAMIC_DRAW );

    auto& meshes = data->pModel->getMeshes();
    for ( int i = 0; i < meshes.size(); i++ )
    {
      const auto& vao = meshes.at( i ).getVao();

      glVertexArrayVertexBuffer( vao, 1, data->instanceBuffer, 0, sizeof( glm::mat4 ) );

      glEnableVertexArrayAttrib( vao, 3 );
      glEnableVertexArrayAttrib( vao, 4 );
      glEnableVertexArrayAttrib( vao, 5 );
      glEnableVertexArrayAttrib( vao, 6 );

      glVertexArrayAttribFormat( vao, 3, 4, GL_FLOAT, GL_FALSE, 0 );
      glVertexArrayAttribFormat( vao, 4, 4, GL_FLOAT, GL_FALSE, sizeof( glm::vec4 ) );
      glVertexArrayAttribFormat( vao, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof( glm::vec4 ) );
      glVertexArrayAttribFormat( vao, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof( glm::vec4 ) );

      glVertexArrayAttribBinding( vao, 3, 1 );
      glVertexArrayAttribBinding( vao, 4, 1 );
      glVertexArrayAttribBinding( vao, 5, 1 );
      glVertexArrayAttribBinding( vao, 6, 1 );

      glVertexArrayBindingDivisor( vao, 1, 1 );
    }
  }
  else
  {
    // Resize existing buffer (optional, only if m_amount changed)
    glNamedBufferData( data->instanceBuffer, sizeof( glm::mat4 ) * data->count, nullptr, GL_DYNAMIC_DRAW );

    // Upload new data
    glNamedBufferSubData( data->instanceBuffer, 0, sizeof( glm::mat4 ) * data->count, data->instanceMatrices.data() );
  }
}
} // namespace kogayonon_core