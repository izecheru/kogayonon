#define GLM_ENABLE_EXPERIMENTAL
#include "physics/jolt_physics.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/scene/scene.hpp"
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/outline_component.hpp"
#include "core/ecs/components/pointlight_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "resources/light_types.hpp"
#include "resources/pointlight.hpp"
#include "utilities/math/math.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
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

auto core::Scene::changeName( const std::string& name ) -> void
{
    m_name = name;
}

auto core::Scene::removeEntity( entt::entity ent ) -> void
{
    m_pRegistry->removeComponent<IdentifierComponent>( ent );

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

auto core::Scene::addMeshToEntity( entt::entity entity, resources::Mesh* pMesh ) -> void
{
    std::lock_guard lock{ m_registryMutex };
    m_registryModified = true;
    Entity ent{ m_pRegistry.get(), entity };
    ent.setType( EntityType::Object );
    ent.replaceComponent<MeshComponent>( MeshComponent{ .pMesh = pMesh } );

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

auto core::Scene::onUpdate() -> void
{
    // update jolt bodies
    auto rigidView = m_pRegistry->getRegistry().view<core::RigidbodyComponent, core::TransformComponent>();
    rigidView.each( []( const entt::entity& entityId,
                        core::RigidbodyComponent& rigidBodyComponent,
                        core::TransformComponent& transformComponent ) {
        physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

        if ( !jolt->isRunning() )
        {
            return;
        }

        if ( rigidBodyComponent.data.type == physics::RigidbodyType::Static )
        {
            return;
        }

        JPH::BodyInterface& bodyInterface = jolt->getPhysicsSystem().GetBodyInterface();
        JPH::RVec3 pos = bodyInterface.GetCenterOfMassPosition( rigidBodyComponent.body );
        JPH::Quat rot = bodyInterface.GetRotation( rigidBodyComponent.body );
        auto eulerRot = rot.GetEulerAngles();
        transformComponent.translation = glm::vec3{ pos.GetX(), pos.GetY(), pos.GetZ() };

        transformComponent.rotation = glm::vec3{
            glm::degrees( eulerRot.GetX() ), glm::degrees( eulerRot.GetY() ), glm::degrees( eulerRot.GetZ() ) };

        transformComponent.computeMatrix();
    } );
}
