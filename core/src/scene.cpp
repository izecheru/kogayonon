#define GLM_ENABLE_EXPERIMENTAL
#include "utilities/time_tracker/time_tracker.hpp"
#include "core/ecs/components/cubemap_component.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "rapidjson/istreamwrapper.h"
#include "physics/jolt_physics.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/scene/scene.hpp"
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/components/player_component.hpp"
#include "core/ecs/components/player_input_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "tracy/Tracy.hpp"

using namespace utilities;

core::Scene::Scene( const std::string& name )
    : m_entityCount{ 0 }
    , m_name{ name }
    , m_pRegistry{ std::make_unique<Registry>() }
    , m_playing{ false }
    , m_enabledEditorCamera{ true }

{
    physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

    core::PlayerComponent playerComp{ .settings = JPH::CharacterVirtualSettings{} };
    core::PlayerInputComponent playerInputComp{};

    playerComp.character = jolt->createCharacter(
        &playerComp.settings, JPH::RVec3Arg{ 0.0f, 1.0f, 0.0f }, JPH::QuatArg{ 0.0f, 0.0f, 0.0f, 1.0f }, 0 );

    Entity player{ m_pRegistry.get(), "player" };

    player.addComponent<core::PerspectiveCameraComponent>( core::PerspectiveCameraComponent{} );
    player.addComponent<core::PlayerComponent>( playerComp );
    player.addComponent<core::PlayerInputComponent>( playerInputComp );

    m_inputSystem = std::make_unique<PlayerInputSystem>( core::MainRegistry::getInstance().getEventDispatcher(),
                                                         m_pRegistry.get() );
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

auto core::Scene::addMeshToEntity( entt::entity entity, resources::Model* pMesh ) -> void
{
    std::lock_guard lock{ m_registryMutex };
    m_registryModified = true;
    Entity ent{ m_pRegistry.get(), entity };
    ent.setType( EntityType::Object );
    ent.replaceComponent<ModelComponent>( ModelComponent{ .pModel = pMesh } );

    if ( !ent.hasComponent<TransformComponent>() )
        ent.addComponent<TransformComponent>();
}

void core::Scene::removeMeshFromEntity( entt::entity entity )
{
    m_registryModified = true;
    Entity ent{ m_pRegistry.get(), entity };

    ent.removeComponent<ModelComponent>();
    ent.removeComponent<IndexComponent>();
    ent.removeComponent<TransformComponent>();
}

auto core::Scene::onUpdate() -> void
{
    ZoneScopedN( "Scene::onUpdate" );

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

    // if there are mesh components that are done loading use them
    auto meshView = m_pRegistry->getRegistry().view<core::ModelComponent>();

    meshView.each( []( const entt::entity& entityId, core::ModelComponent& modelComp ) {
        core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

        if ( !modelComp.pModel )
        {
            resources::Model* mesh = assetManager->getMesh( modelComp.modelPath );
            if ( !mesh )
            {
                return;
            }
            modelComp.pModel = mesh;
        }
    } );

    utilities::TimeTracker* timeTracker = core::MainRegistry::getInstance().getTimeTracker();

    if ( m_playing )
    {
        m_inputSystem->update( getEnttRegistry(), timeTracker->getDurationInSeconds( DELTA_TIME ) );

        if ( m_enabledEditorCamera )
            m_enabledEditorCamera = false;
    }
    else
    {
        if ( !m_enabledEditorCamera )
        {
            auto cameraView = m_pRegistry->getRegistry().view<core::PerspectiveCameraComponent>();

            cameraView.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComponent ) {
                Entity entity{ m_pRegistry.get(), entityId };
                if ( entity.hasComponent<core::PlayerComponent>() )
                {
                    if ( cameraComponent.isUsed )
                        cameraComponent.isUsed = false;

                    return;
                }

                if ( !cameraComponent.isUsed )
                    cameraComponent.isUsed = true;
            } );

            m_enabledEditorCamera = true;
        }
    }
}

auto core::Scene::deserialize( const std::filesystem::path& p ) -> void
{
    ZoneScopedN( "Scene::deserialize" );
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

    core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

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
            std::filesystem::path modelPath = e["mesh"]["path"].GetString();
            resources::Model* mesh = assetManager->loadModel( modelPath.stem().string(), modelPath.string() );
            ent.addComponent<core::ModelComponent>(
                core::ModelComponent{ .modelPath = modelPath.string(), .pModel = mesh } );
        }

        if ( e.HasMember( "cubemap" ) )
        {
            resources::Texture* cubemap = assetManager->loadCubemap( e["cubemap"]["path"].GetString() );
            const bool isUsed = e["cubemap"]["isUsed"].GetBool();
            ent.addComponent<core::CubemapComponent>(
                core::CubemapComponent{ .cubemapTexture = cubemap, .isUsed = isUsed } );
        }
    }
}

auto core::Scene::serialize() -> void
{
    ZoneScopedN( "Scene::serialize" );

    auto saveCubemap = []( utilities::JsonSerializer* s, const core::CubemapComponent& cubemapComp ) {
        s->startObject( "cubemap" )
            .addKeyValuePair( "path", cubemapComp.cubemapTexture->path )
            .addKeyValuePair( "isUsed", cubemapComp.isUsed )
            .endObject();
    };

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

    auto saveEntitymodelComponent = []( utilities::JsonSerializer* s, const core::ModelComponent& modelComponent ) {
        if ( !modelComponent.pModel )
            return;

        s->startObject( "mesh" ).addKeyValuePair( "path", modelComponent.pModel->getPath() ).endObject();
    };

    std::string sceneFilename = m_name + ".kscene";
    std::filesystem::path scenePath = std::filesystem::current_path() / "editor" / "scenes" / sceneFilename;

    // this also creates the output file
    std::unique_ptr<utilities::JsonSerializer> serializer =
        std::make_unique<utilities::JsonSerializer>( scenePath.string() );

    serializer->startArray( "entities" );

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

        if ( entity.hasComponent<core::ModelComponent>() )
        {
            saveEntitymodelComponent( serializer.get(), entity.getComponent<core::ModelComponent>() );
        }

        if ( entity.hasComponent<core::TransformComponent>() )
        {
            saveEntityTransformComponent( serializer.get(), entity.getComponent<core::TransformComponent>() );
        }

        if ( entity.hasComponent<core::CubemapComponent>() )
        {
            saveCubemap( serializer.get(), entity.getComponent<core::CubemapComponent>() );
        }

        serializer->endObject();
    } );

    serializer->endArray();
}

auto core::Scene::startPlaying() -> void
{
    m_playing = true;
}

auto core::Scene::stopPlaying() -> void
{
    m_playing = false;
}

auto core::Scene::getPlaying() const -> bool
{
    return m_playing;
}
