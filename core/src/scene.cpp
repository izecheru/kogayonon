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
#include "utilities/math/math.hpp"
using namespace utilities;

core::Scene::Scene( const std::string& name )
    : m_entityCount{ 0 }
    , m_name{ name }
    , m_pRegistry{ std::make_unique<Registry>() }

{
}

auto core::Scene::getRegistry() -> Registry*
{
  return m_pRegistry.get();
}

auto core::Scene::getEnttRegistry() -> entt::registry&
{
  return m_pRegistry->getRegistry();
}

auto core::Scene::getName() const -> std::string
{
  return m_name;
}

void core::Scene::changeName( const std::string& name )
{
  m_name = name;
}

void core::Scene::removeEntity( entt::entity ent )
{
  // then destroy the entity
  if ( m_pRegistry->getRegistry().valid( ent ) )
    m_pRegistry->getRegistry().destroy( ent );

  --m_entityCount;
}

auto core::Scene::addEntity() -> entt::entity
{
  Entity ent{ getRegistry(), "DefaultEntity" };
  ++m_entityCount;
  return ent.getEntityId();
}

void core::Scene::addMeshToEntity( entt::entity entity, resources::Mesh* pMesh )
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

void core::Scene::removeMeshFromEntity( entt::entity entity )
{
  m_registryModified = true;
  Entity ent{ m_pRegistry.get(), entity };

  ent.removeComponent<MeshComponent>();
  ent.removeComponent<IndexComponent>();
  ent.removeComponent<TransformComponent>();
}

void core::Scene::updateRigidbodyEntities()
{
  if ( !physics::NvidiaPhysx::getInstance().isRunning() )
    return;
}