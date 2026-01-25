#define GLM_ENABLE_EXPERIMENTAL
#include "core/scene/scene.hpp"
#include <glad/glad.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/outline_component.hpp"
#include "core/ecs/components/pointlight_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "physics/nvidia_physx.hpp"
#include "resources/light_types.hpp"
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
  m_lightUBO.initialize( 3 );
  m_lightSSBO.initialize();
}

auto Scene::getRegistry() -> Registry*
{
  return m_pRegistry.get();
}

auto Scene::getEnttRegistry() -> entt::registry&
{
  return m_pRegistry->getRegistry();
}

auto Scene::getName() const -> std::string
{
  return m_name;
}

void Scene::changeName( const std::string& name )
{
  m_name = name;
}

void Scene::removeEntity( entt::entity ent )
{
  Entity entity{ getRegistry(), ent };

  if ( entity.hasComponent<MeshComponent>() )
    removeInstanceData( ent );

  if ( entity.hasComponent<PointLightComponent>() )
  {
    const auto& pLightComponent = entity.getComponent<PointLightComponent>();
    const auto toErase = pLightComponent.pointLightIndex;
    m_lightSSBO.removeLight( kogayonon_resources::LightType::Point, toErase );
    m_lightUBO.decrementLightCount( kogayonon_resources::LightType::Point );

    for ( const auto& [entity, pLightComponent_] : m_pRegistry->getRegistry().view<PointLightComponent>().each() )
    {
      if ( pLightComponent_.pointLightIndex > toErase )
        --pLightComponent_.pointLightIndex;
    }
    updateLightBuffers();
  }

  if ( entity.hasComponent<DirectionalLightComponent>() )
  {
    const auto& pDirectionalLightComponent = entity.getComponent<DirectionalLightComponent>();
    const auto toErase = pDirectionalLightComponent.directionalLightIndex;
    m_lightSSBO.removeLight( kogayonon_resources::LightType::Directional, toErase );
    m_lightUBO.decrementLightCount( kogayonon_resources::LightType::Directional );

    for ( const auto& [entity, pDirectionalLightComponent_] :
          m_pRegistry->getRegistry().view<DirectionalLightComponent>().each() )
    {
      if ( pDirectionalLightComponent_.directionalLightIndex > toErase )
        --pDirectionalLightComponent_.directionalLightIndex;
    }
    updateLightBuffers();
  }

  // then destroy the entity
  if ( m_pRegistry->getRegistry().valid( ent ) )
    m_pRegistry->getRegistry().destroy( ent );

  --m_entityCount;
}

auto Scene::addEntity() -> entt::entity
{
  Entity ent{ getRegistry(), "DefaultEntity" };
  ++m_entityCount;
  return ent.getEntityId();
}

bool Scene::addInstanceData( entt::entity entityId )
{
  Entity entity{ m_pRegistry.get(), entityId };

  // now that we created the entity using the scene registry and added a model and transform components to it
  // we must look for model pointer in the map to check for instances
  if ( auto pMeshComponent = entity.tryGetComponent<MeshComponent>(); m_instances.contains( pMeshComponent->pMesh ) )
  {
    auto pMesh = pMeshComponent->pMesh;
    if ( m_instances.contains( pMesh ) )
    {
      auto& instanceData = m_instances.at( pMesh );

      // we create a new instance matrix
      const auto& transform = entity.getComponent<TransformComponent>();

      // increment the count
      ++instanceData->count;

      // insert the gpu instance in the vector
      instanceData->instances.emplace_back( GPUInstance{
        .entityId = static_cast<int>( entityId ),
        .instanceMatrix = math::computeTransform( transform.translation, transform.rotation, transform.scale ),
      } );

      // get the index of the instance matrix
      uint32_t size = instanceData->instances.size();

      // add the index component and assing the index to size-1 (last added element)
      entity.addComponent<IndexComponent>( IndexComponent{ .index = size - 1 } );

      // we found the model in the instance map so the geometry is also already uploaded to the gpu and we use the
      // instance matrix for per model translation/ rotation/ scale
      return true;
    }
  }
  else
  {
    auto pMesh = pMeshComponent->pMesh;
    const auto& transform = entity.getComponent<TransformComponent>();
    auto instanceData = std::make_unique<InstanceData>(
      InstanceData{ .instances = { GPUInstance{ .entityId = static_cast<int>( entityId ),
                                                .instanceMatrix = math::computeTransform(
                                                  transform.translation, transform.rotation, transform.scale ) } },
                    .count = 1,
                    .pMesh = pMesh } );

    // first we get the index of the instance matrix
    uint32_t size = instanceData->instances.size();
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
  Entity ent{ m_pRegistry.get(), entity };
  ent.setType( EntityType::Object );
  ent.replaceComponent<MeshComponent>( MeshComponent{ .pMesh = pMesh, .staticMesh = false, .loaded = false } );

  // if we did not setup the transform from somewhere else like deserialized, we initialise a default one
  if ( !ent.hasComponent<TransformComponent>() )
    ent.addComponent<TransformComponent>();
}

void Scene::removeInstanceData( entt::entity ent )
{
  // upon deletion we must remove that specific instance
  Entity entity{ m_pRegistry.get(), ent };

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
    instance->instances.erase( instance->instances.begin() + toErase );

    // now we need to update all the index components that are to the right of the deleted index
    for ( const auto& [entity, indexComp, meshComp] :
          m_pRegistry->getRegistry().view<IndexComponent, MeshComponent>().each() )
    {
      Entity ent{ m_pRegistry.get(), entity };

      // update the index just to those who use the same mesh
      if ( meshComp.pMesh == pMesh )
      {
        // if the index from the iteration is > to the index component we erased from the instances
        if ( indexComp.index > toErase )
          --indexComp.index;
      }
    }

    // finally set up instances again to update the instance buffer and entityIds buffer
    setupInstances( instance.get() );
  }
}

void Scene::removeMeshFromEntity( entt::entity entity )
{
  m_registryModified = true;
  Entity ent{ m_pRegistry.get(), entity };

  removeInstanceData( entity );

  ent.removeComponent<MeshComponent>();
  ent.removeComponent<IndexComponent>();
  ent.removeComponent<TransformComponent>();
}

void Scene::addOutline( entt::entity ent )
{
  Entity entity{ m_pRegistry.get(), ent };

  removeOutline();

  if ( entity.hasComponent<MeshComponent>() && !entity.hasComponent<OutlineComponent>() )
  {
    auto& meshComp = entity.getComponent<MeshComponent>();

    if ( !meshComp.loaded || meshComp.pMesh == nullptr )
      return;

    auto data = getData( meshComp.pMesh );
    auto index = entity.getComponent<IndexComponent>().index;
    data->instances.at( index ).selected = 1;
    entity.addComponent<OutlineComponent>();
    updateInstances( data );
  }
}

void Scene::removeOutline()
{
  const auto& view = m_pRegistry->getRegistry().view<OutlineComponent>();
  if ( view.size() == 0 )
    return;

  view.each( [&]( const auto& ent, auto& outlineComp ) {
    Entity entity{ m_pRegistry.get(), ent };

    auto& meshComp = entity.getComponent<MeshComponent>();
    auto data = getData( meshComp.pMesh );
    auto index = entity.getComponent<IndexComponent>().index;
    data->instances.at( index ).selected = 0;
    entity.removeComponent<OutlineComponent>();
    updateInstances( data );
  } );
}

auto Scene::getData( kogayonon_resources::Mesh* pModel ) -> InstanceData*
{
  if ( m_instances.find( pModel ) == m_instances.end() )
    return nullptr;

  return m_instances.at( pModel ).get();
}

void Scene::updateInstances( InstanceData* data )
{
  // upload new data
  glNamedBufferSubData( data->instanceBuffer, 0, sizeof( GPUInstance ) * data->count, data->instances.data() );
}

void Scene::setupInstances( InstanceData* data )
{
  if ( data->instanceBuffer == 0 )
    glCreateBuffers( 1, &data->instanceBuffer );

  glNamedBufferData(
    data->instanceBuffer, sizeof( GPUInstance ) * data->count, data->instances.data(), GL_DYNAMIC_DRAW );

  const auto& vao = data->pMesh->getVao();

  glVertexArrayVertexBuffer( vao, 1, data->instanceBuffer, 0, sizeof( GPUInstance ) );

  glEnableVertexArrayAttrib( vao, 3 );
  glEnableVertexArrayAttrib( vao, 4 );
  glEnableVertexArrayAttrib( vao, 5 );
  glEnableVertexArrayAttrib( vao, 6 );
  glEnableVertexArrayAttrib( vao, 7 );
  glEnableVertexArrayAttrib( vao, 8 );

  glVertexArrayAttribIFormat( vao, 3, 1, GL_UNSIGNED_INT, offsetof( GPUInstance, selected ) );
  glVertexArrayAttribIFormat( vao, 4, 1, GL_INT, offsetof( GPUInstance, entityId ) );

  std::size_t matrixOffset = offsetof( GPUInstance, instanceMatrix );

  for ( auto i = 0u; i < 4; i++ )
  {
    glVertexArrayAttribFormat( vao, 5 + i, 4, GL_FLOAT, GL_FALSE, matrixOffset + i * sizeof( glm::vec4 ) );
  }

  glVertexArrayAttribBinding( vao, 3, 1 );
  glVertexArrayAttribBinding( vao, 4, 1 );
  glVertexArrayAttribBinding( vao, 5, 1 );
  glVertexArrayAttribBinding( vao, 6, 1 );
  glVertexArrayAttribBinding( vao, 7, 1 );
  glVertexArrayAttribBinding( vao, 8, 1 );

  glVertexArrayBindingDivisor( vao, 1, 1 );
}

void Scene::updateRigidbodyEntities()
{
  if ( !kogayonon_physics::NvidiaPhysx::getInstance().isRunning() )
    return;

  const auto& view =
    m_pRegistry->getRegistry().view<DynamicRigidbodyComponent, TransformComponent, MeshComponent, IndexComponent>();

  view.each( [&]( const auto& entity,
                  auto& dynamicRigidbodyComponent,
                  auto& transformComponent,
                  auto& meshComponent,
                  auto& indexComponent ) {
    // get the physics pose
    auto pose = dynamicRigidbodyComponent.pBody->getGlobalPose();

    // construct position and rotation from the global pose
    glm::vec3 position{ pose.p.x, pose.p.y, pose.p.z };
    glm::quat rotation{ pose.q.w, pose.q.x, pose.q.y, pose.q.z };

    // create the model matrix with those matrices
    glm::mat4 model = glm::translate( glm::mat4{ 1.0f }, position ) * glm::mat4{ rotation } *
                      glm::scale( glm::mat4{ 1.0f }, transformComponent.scale );

    // get the instance data
    auto instanceData = getData( meshComponent.pMesh );

    // update the instance matrix
    auto& instanceMatrix = instanceData->instances.at( indexComponent.index ).instanceMatrix;
    instanceMatrix = model;
    // rotation is using euler angles, yaw pitch roll (glm::vec3)
    transformComponent.rotation = glm::eulerAngles( rotation );

    // finally update the instances
    updateInstances( instanceData );
  } );
}

auto Scene::getLightCount( const kogayonon_resources::LightType& type ) -> uint32_t
{
  return m_lightUBO.getLightCount( type );
}

void Scene::prepareForRendering()
{
  // skip this function if we did not add a new entity or something
  if ( !m_registryModified )
    return;

  m_registryModified = false;

  updateLightBuffers();

  auto& assetManager = AssetManager::getInstance();

  for ( const auto& [entity, meshComponent] : getRegistry()->getRegistry().view<MeshComponent>().each() )
  {
    Entity ent{ getRegistry(), entity };

    if ( meshComponent.pMesh == nullptr )
      continue;

    if ( !meshComponent.loaded )
    {
      // if we don't have the model in the instance map, we insert it and based on the condition of
      // it being already in the map OR NOT we return the boolean value of that statement and then we upload the
      // geometry since if it is not in the map we just created a fresh InstanceData that is not on the gpu
      if ( !addInstanceData( entity ) )
      {
        assetManager.uploadMeshGeometry( meshComponent.pMesh );
      }

      setupInstances( getData( meshComponent.pMesh ) );

      // check textures
      for ( auto& texture : meshComponent.pMesh->getTextures() )
      {

        // if either one of them is true then continue to the next one
        if ( !texture || texture->getLoaded() )
          continue;

        texture = assetManager.addTexture( texture->getName() );
        texture->setLoaded( true );
      }

      meshComponent.loaded = true;
    }
  }
}

void Scene::addPointLight()
{
  Entity entity{ getRegistry(), addEntity() };
  entity.setType( EntityType::Light );
  m_lightUBO.incrementLightCount( kogayonon_resources::LightType::Point );
  auto index = m_lightSSBO.addLight( kogayonon_resources::LightType::Point );
  entity.addComponent<PointLightComponent>( PointLightComponent{ .pointLightIndex = index } );
}

void Scene::addPointLight( entt::entity entityId )
{
  Entity ent{ m_pRegistry.get(), entityId };
  ent.setType( EntityType::Light );
  m_lightUBO.incrementLightCount( kogayonon_resources::LightType::Point );
  auto index = m_lightSSBO.addLight( kogayonon_resources::LightType::Point );
  ent.addComponent<PointLightComponent>( PointLightComponent{ .pointLightIndex = index } );
}

void Scene::addDirectionalLight()
{
  // maximum amount of directional lights is = 1
  if ( m_lightUBO.getLightCount( kogayonon_resources::LightType::Directional ) == 1 )
  {
    spdlog::debug( "Entity discarded, we already have a directional light" );
    return;
  }

  Entity entity{ getRegistry(), addEntity() };
  entity.setType( EntityType::Light );
  m_lightUBO.incrementLightCount( kogayonon_resources::LightType::Directional );
  int index = m_lightSSBO.addLight( kogayonon_resources::LightType::Directional );
  entity.addComponent<DirectionalLightComponent>( DirectionalLightComponent{ .directionalLightIndex = index } );
}

void Scene::addDirectionalLight( entt::entity entityId )
{
  // maximum amount of directional lights is = 1
  if ( m_lightUBO.getLightCount( kogayonon_resources::LightType::Directional ) == 1 )
    return;

  Entity entity{ m_pRegistry.get(), entityId };
  entity.setType( EntityType::Light );

  m_lightUBO.incrementLightCount( kogayonon_resources::LightType::Directional );
  int index = m_lightSSBO.addLight( kogayonon_resources::LightType::Directional );
  entity.addComponent<DirectionalLightComponent>( DirectionalLightComponent{ .directionalLightIndex = index } );
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

auto Scene::getPointLight( uint32_t index ) -> kogayonon_resources::PointLight&
{
  assert( index >= 0 && "index must not be negative" );
  return m_lightSSBO.getPointLights().at( index );
}

auto Scene::getDirectionalLight( uint32_t index ) -> kogayonon_resources::DirectionalLight&
{
  assert( index >= 0 && "index must not be negative" );
  return m_lightSSBO.getDirectionalLights().at( index );
}

void Scene::updateLightBuffers()
{
  m_lightSSBO.update();
  m_lightUBO.update();
}

} // namespace kogayonon_core