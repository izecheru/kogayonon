#include "core/scene/scene.hpp"
#include <glad/glad.h>
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/pointlight_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "resources/pointlight.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/math/math.hpp"
using namespace kogayonon_utilities;

namespace kogayonon_core
{
Scene::Scene( const std::string& name )
    : m_entityCount{ 0 }
    , m_name{ name }
    , m_pRegistry{ std::make_unique<Registry>() }

{
  m_lightUBO.initialize( 0 );
  m_lightSSBO.initialize( 1 );
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
  Entity entity{ *m_pRegistry, ent };

  if ( entity.hasComponent<MeshComponent>() )
    removeInstanceData( ent );

  // then destroy the entity
  if ( registry.valid( ent ) )
    registry.destroy( ent );

  --m_entityCount;
}

Entity Scene::addEntity()
{
  Entity ent{ *m_pRegistry, "DefaultEntity" };
  ++m_entityCount;
  return ent;
}

bool Scene::addInstanceData( entt::entity entityId )
{
  Entity entity{ *m_pRegistry, entityId };

  // now that we created the entity using the scene registry and added a model and transform components to it
  // we must look for model pointer in the map to check for instances
  if ( auto pMeshComponent = entity.tryGetComponent<MeshComponent>(); m_instances.contains( pMeshComponent->pMesh ) )
  {
    auto pMesh = pMeshComponent->pMesh;
    if ( m_instances.contains( pMesh ) )
    {
      auto& instanceData = m_instances.at( pMesh );
      // we increment the amount of models we need to render
      ++instanceData->count;

      // we create a new instance matrix
      const auto& transform = entity.getComponent<TransformComponent>();

      // insert in the vector
      instanceData->instanceMatrices.push_back(
        math::computeTransform( transform.translation, transform.rotation, transform.scale ) );

      // get the index of the instance matrix
      uint32_t size = instanceData->instanceMatrices.size();

      // add the index component and assing the index to size-1 (last added element)
      entity.addComponent<IndexComponent>( IndexComponent{ .index = size - 1 } );

      // insert the entityId
      instanceData->entityIds.push_back( static_cast<int>( entity.getEntityId() ) );

      // we found the model in the instance map so the geometry is also already uploaded to the gpu and we use the
      // instance matrix for per model translation/ rotation/ scale
      return true;
    }
  }
  else
  {
    auto pMesh = pMeshComponent->pMesh;
    const auto& transform = entity.getComponent<TransformComponent>();
    auto instanceData = std::make_unique<InstanceData>( InstanceData{
      .entityIdBuffer = 0,
      .entityIds = { static_cast<int>( entity.getEntityId() ) },
      .instanceBuffer = 0,
      .instanceMatrices = { math::computeTransform( transform.translation, transform.rotation, transform.scale ) },
      .count = 1,
      .pMesh = pMesh } );

    // first we get the index of the instance matrix
    uint32_t size = instanceData->instanceMatrices.size();
    entity.addComponent<kogayonon_core::IndexComponent>( IndexComponent{ .index = size - 1 } );

    m_instances.try_emplace( pMesh, std::move( instanceData ) );
    // we did not find the model in the instance map so we must also upload geometry to the GPU
    return false;
  }
}

void Scene::addMeshToEntity( entt::entity entity, kogayonon_resources::Mesh* pMesh )
{
  std::lock_guard lock{ m_registryMutex };
  m_registryModified = true;
  Entity ent{ *m_pRegistry, entity };
  ent.replaceComponent<MeshComponent>( MeshComponent{ .pMesh = pMesh } );

  // if we did not setup the transform from somewhere else like deserialized, we initialise a default one
  if ( !ent.hasComponent<TransformComponent>() )
    ent.addComponent<TransformComponent>();
}

void Scene::removeInstanceData( entt::entity ent )
{
  // upon deletion we must remove that specific instance
  Entity entity{ *m_pRegistry, ent };

  const auto& meshComponent = entity.getComponent<MeshComponent>();

  // if we have instances
  auto pMesh = meshComponent.pMesh;

  if ( !pMesh )
    return;

  if ( m_instances.contains( pMesh ) )
  {
    const auto& instance = m_instances.at( pMesh );

    --instance->count;

    const auto& indexComponent = entity.getComponent<IndexComponent>();

    // get the index component and then erase that specific instance matrix
    auto toErase = indexComponent.index;
    instance->instanceMatrices.erase( instance->instanceMatrices.begin() + toErase );

    // erase the entityId too
    instance->entityIds.erase( instance->entityIds.begin() + toErase );

    // now we need to update all the index components that are to the right of the deleted index
    for ( const auto& [entity, indexComp] : m_pRegistry->getRegistry().view<IndexComponent>().each() )
    {
      Entity ent{ *m_pRegistry, entity };

      // if the index from the iteration is > to the index component we erased from the instances
      if ( indexComp.index > toErase )
        --indexComp.index;
    }

    // finally set up instances again to update the instance buffer and entityIds buffer
    setupMultipleInstances( instance.get() );
  }
}

void Scene::removeMeshFromEntity( entt::entity entity )
{
  m_registryModified = true;
  Entity ent{ *m_pRegistry, entity };

  removeInstanceData( entity );

  ent.removeComponent<MeshComponent>();
  ent.removeComponent<IndexComponent>();
  ent.removeComponent<TransformComponent>();
}

InstanceData* Scene::getData( kogayonon_resources::Mesh* pModel )
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

    const auto& vao = data->pMesh->getVao();

    glVertexArrayVertexBuffer( vao, 1, data->instanceBuffer, 0, sizeof( glm::mat4 ) );
    glVertexArrayVertexBuffer( vao, 2, data->entityIdBuffer, 0, sizeof( int ) );

    glEnableVertexArrayAttrib( vao, 3 );
    glEnableVertexArrayAttrib( vao, 4 );
    glEnableVertexArrayAttrib( vao, 5 );
    glEnableVertexArrayAttrib( vao, 6 );
    glEnableVertexArrayAttrib( vao, 7 );

    glVertexArrayAttribFormat( vao, 3, 4, GL_FLOAT, GL_FALSE, 0 );
    glVertexArrayAttribFormat( vao, 4, 4, GL_FLOAT, GL_FALSE, sizeof( glm::vec4 ) );
    glVertexArrayAttribFormat( vao, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof( glm::vec4 ) );
    glVertexArrayAttribFormat( vao, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof( glm::vec4 ) );
    glVertexArrayAttribIFormat( vao, 7, 1, GL_INT, 0 );

    glVertexArrayAttribBinding( vao, 3, 1 );
    glVertexArrayAttribBinding( vao, 4, 1 );
    glVertexArrayAttribBinding( vao, 5, 1 );
    glVertexArrayAttribBinding( vao, 6, 1 );
    glVertexArrayAttribBinding( vao, 7, 2 );

    glVertexArrayBindingDivisor( vao, 1, 1 );
    glVertexArrayBindingDivisor( vao, 2, 1 );
  }
  else
  {
    // upload new data
    glNamedBufferSubData( data->instanceBuffer, 0, sizeof( glm::mat4 ) * data->count, data->instanceMatrices.data() );
    glNamedBufferSubData( data->entityIdBuffer, 0, sizeof( int ) * data->count, data->entityIds.data() );
  }
}

void Scene::prepareForRendering()
{
  // skip this function if we did not add a new entity or something
  if ( !m_registryModified )
    return;

  m_registryModified = false;

  const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();

  for ( const auto& [entity, meshComponent] : getRegistry().getRegistry().view<MeshComponent>().each() )
  {
    Entity ent{ getRegistry(), entity };
    if ( meshComponent.pMesh == nullptr )
      continue;

    if ( meshComponent.loaded )
      continue;

    // if we don't have the model in the instance map, we upload the geometry
    if ( !addInstanceData( entity ) )
      pAssetManager->uploadMeshGeometry( meshComponent.pMesh );

    const auto& data = getData( meshComponent.pMesh );
    setupMultipleInstances( data );

    // check textures
    for ( auto& texture : meshComponent.pMesh->getTextures() )
    {

      if ( texture == nullptr )
        continue;

      if ( texture->getLoaded() == true )
        continue;

      auto texture_ = pAssetManager->addTexture( texture->getName() );
      texture_->setLoaded( true );
      texture = std::move( texture_ );
    }
    meshComponent.loaded = true;
  }
}

void Scene::addPointLight()
{
  auto entity = addEntity();
  m_lightUBO.incrementPointLights();
  auto index = m_lightSSBO.addPointLight();
  entity.addComponent<PointLightComponent>( PointLightComponent{ .pointLightIndex = index } );
}

void Scene::addPointLight( entt::entity entityId )
{
  Entity ent{ *m_pRegistry, entityId };
  m_lightUBO.incrementPointLights();
  auto index = m_lightSSBO.addPointLight();
  ent.addComponent<PointLightComponent>( PointLightComponent{ .pointLightIndex = index } );
}

void Scene::bindLightBuffers()
{
  m_lightSSBO.bind();
  m_lightUBO.bind();
}

void Scene::unbindLightBuffers()
{
  m_lightSSBO.unbind();
  m_lightUBO.unbind();
}

kogayonon_resources::PointLight& Scene::getPointLight( uint32_t index )
{
  auto& lights = m_lightSSBO.getPointLights();
  return lights.at( index );
}

void Scene::updateLightBuffers()
{
  m_lightSSBO.update();
  m_lightUBO.update();
}

} // namespace kogayonon_core