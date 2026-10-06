#define GLM_ENABLE_EXPERIMENTAL
#include "core/ecs/components/camera_component.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/istreamwrapper.h"
#include "physics/jolt_physics.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/scene/scene.hpp"
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
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
    // update bodies
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

auto core::Scene::deserialize( const std::filesystem::path& p ) -> void
{
    // TODO write this block of code as a json deserializer
    std::ifstream in{ p, std::ios::in | std::ios::binary };
    rapidjson::IStreamWrapper isw{ in };
    rapidjson::Document doc{};
    doc.ParseStream( isw );
    if ( doc.HasParseError() )
    {
        KERROR( "Error at parsing json file for default scene" );
    }

    auto getVec3 = []( const rapidjson::Value& v ) -> glm::vec3 {
        if ( !v.IsArray() || v.Size() != 3 )
        {
            throw std::runtime_error( "Expected array with size 3" );
        }

        return glm::vec3{ v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat() };
    };

    for ( const auto& e : doc["entities"].GetArray() )
    {
        core::Entity ent{ getRegistry() };

        if ( e.HasMember( "identifier" ) )
        {
            std::string name = e["identifier"]["name"].GetString();
            std::string group = e["identifier"]["group"].GetString();

            ent.addComponent<core::IdentifierComponent>(
                core::IdentifierComponent{ .name = name, .type = core::EntityType::Object, .group = group } );
        }

        if ( e.HasMember( "transform" ) )
        {
            glm::vec3 translation = getVec3( e["transform"]["translation"] );
            glm::vec3 rotation = getVec3( e["transform"]["rotation"] );
            glm::vec3 scale = getVec3( e["transform"]["scale"] );
            core::TransformComponent transform{ .translation = translation, .rotation = rotation, .scale = scale };
            transform.computeMatrix();
            ent.addComponent<core::TransformComponent>( transform );
        }

        if ( e.HasMember( "mesh" ) )
        {
            core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();
            std::filesystem::path meshPath = e["mesh"]["path"].GetString();
            resources::Mesh* mesh = assetManager->loadMesh( meshPath.stem().string(), meshPath.string() );
            ent.addComponent<core::MeshComponent>( core::MeshComponent{ .pMesh = mesh } );
        }
    }
}

auto core::Scene::serialize() -> void
{
    auto saveEntityIdComponent = []( utilities::JsonSerializer* s, const core::IdentifierComponent& idComponent ) {
        s->startObject( "identifier" )
            .addKeyValuePair( "name", idComponent.name )
            .addKeyValuePair( "group", idComponent.group )
            .endObject();
    };

    auto saveEntityTransformComponent = []( utilities::JsonSerializer* s, const core::TransformComponent& transform ) {
        s->startObject( "transform" )
            .saveVec3( "translation", transform.translation )
            .saveVec3( "rotation", transform.rotation )
            .saveVec3( "scale", transform.scale )
            .endObject();
    };

    auto saveEntityMeshComponent = []( utilities::JsonSerializer* s, const core::MeshComponent& meshComponent ) {
        if ( !meshComponent.pMesh )
            return;

        s->startObject( "mesh" ).addKeyValuePair( "path", meshComponent.pMesh->getPath() ).endObject();
    };

    std::string sceneFilename = m_name + ".kscene";
    std::filesystem::path scenePath = std::filesystem::current_path() / "editor" / "scenes" / sceneFilename;

    // this also creates the output file
    std::unique_ptr<utilities::JsonSerializer> serializer =
        std::make_unique<utilities::JsonSerializer>( scenePath.string() );

    serializer->startDocument().startArray( "entities" );

    // serialize except cameras and lights for now
    auto view = getEnttRegistry().view<core::IdentifierComponent>(
        entt::exclude<core::DirectionalLightComponent, core::PerspectiveCameraComponent> );

    view.each( [&]( const entt::entity& id, core::IdentifierComponent& idComponent ) {
        Entity entity{ getRegistry(), id };

        // this could be useful since we can also unhash it so we can create each entity in order
        // uint32_t entityId = static_cast<uint32_t>( entity.getEntityId() );
        // std::string idHash = std::to_string( std::hash<uint32_t>{}( entityId ) );

        serializer->startObject();

        saveEntityIdComponent( serializer.get(), idComponent );

        if ( entity.hasComponent<core::MeshComponent>() )
        {
            saveEntityMeshComponent( serializer.get(), entity.getComponent<core::MeshComponent>() );
        }

        if ( entity.hasComponent<core::TransformComponent>() )
        {
            saveEntityTransformComponent( serializer.get(), entity.getComponent<core::TransformComponent>() );
        }

        serializer->endObject();
    } );

    serializer->endArray().endDocument();
}
