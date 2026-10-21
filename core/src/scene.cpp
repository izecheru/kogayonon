#define GLM_ENABLE_EXPERIMENTAL
#include "utilities/time_tracker/time_tracker.hpp"
#include "core/input/mouse_events.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "core/ecs/components/cubemap_component.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/player_component.hpp"
#include "rapidjson/istreamwrapper.h"
#include "physics/jolt_physics.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/scene/scene.hpp"
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "tracy/Tracy.hpp"

using namespace utilities;

core::Scene::Scene( const std::string& name )
    : m_entityCount{ 0 }
    , m_name{ name }
    , m_pRegistry{ std::make_unique<Registry>() }

{
    core::EventDispatcher* eventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
    eventDispatcher->addHandler<core::MouseMovedEvent, &Scene::onMouseMoved>( *this );
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

    auto playerView = m_pRegistry->getRegistry().view<core::PerspectiveCameraComponent, core::PlayerComponent>();

    physics::JoltPhysics* jolt = core::MainRegistry::getInstance().getJoltPhysics();

    if ( jolt->isRunning() )
    {
        bool activePlayer{ false };
        playerView.each( [&]( const entt::entity& entityId,
                              core::PerspectiveCameraComponent& cameraComp,
                              core::PlayerComponent& playerComp ) {
            if ( !cameraComp.isUsed )
                cameraComp.isUsed = true;

            activePlayer = true;

            JPH::Vec3 currentVelocity = playerComp.character->GetLinearVelocity();

            utilities::TimeTracker* timeTracker = core::MainRegistry::getInstance().getTimeTracker();
            float delta = timeTracker->getDurationInSeconds( DELTA_TIME );

            constexpr float speed{ 4.0f };

            float yawRad = glm::radians( cameraComp.props.yaw );

            glm::vec3 forward{ cos( yawRad ), 0.0f, sin( yawRad ) };
            glm::vec3 right{ -sin( yawRad ), 0.0f, cos( yawRad ) };

            glm::vec3 moveDir{ 0.0f };

            if ( KeyboardState::getKeyState( KeyScanCode::W ) )
                moveDir += forward;

            if ( KeyboardState::getKeyState( KeyScanCode::S ) )
                moveDir -= forward;

            if ( KeyboardState::getKeyState( KeyScanCode::D ) )
                moveDir += right;

            if ( KeyboardState::getKeyState( KeyScanCode::A ) )
                moveDir -= right;

            if ( glm::length( moveDir ) > 0.001f )
                moveDir = glm::normalize( moveDir );

            JPH::Vec3 desiredVelocity{ moveDir.x * speed, currentVelocity.GetY(), moveDir.z * speed };

            if ( playerComp.character->GetGroundState() != JPH::CharacterBase::EGroundState::OnGround )
            {
                desiredVelocity += jolt->getPhysicsSystem().GetGravity() * delta;
            }

            playerComp.character->SetLinearVelocity( desiredVelocity );

            jolt->updateCharacter( playerComp.character, delta );

            JPH::RVec3 newPos = playerComp.character->GetPosition();

            // update camera to match character
            cameraComp.props.eye = { newPos.GetX(), newPos.GetY(), newPos.GetZ() };

            if ( m_mouseCoord.x != 0 || m_mouseCoord.y != 0 )
            {
                constexpr float sensitivity{ 0.6f };
                cameraComp.props.yaw += m_mouseCoord.x * sensitivity;
                cameraComp.props.pitch -= m_mouseCoord.y * sensitivity;
                m_mouseCoord = { 0.0f, 0.0f };
            }

            // renderer will update this later
            cameraComp.props.changed = true;
        } );

        // deactivate other cameras
        if ( activePlayer )
        {
            auto cameraView = m_pRegistry->getRegistry().view<core::PerspectiveCameraComponent>(
                entt::exclude<core::PlayerComponent> );

            cameraView.each( []( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComp ) {
                if ( cameraComp.isUsed )
                    cameraComp.isUsed = false;
            } );
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

auto core::Scene::onMouseMoved( core::MouseMovedEvent& e ) -> void
{
    m_mouseCoord = { e.getXRel(), e.getYRel() };
}
