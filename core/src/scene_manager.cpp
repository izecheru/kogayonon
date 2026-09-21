#include "core/scene/scene_manager.hpp"
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "utilities/json_serializer/json_serializer.hpp"
#include "core/scene/scene.hpp"

core::SceneManager::SceneManager( EventDispatcher* pDispatcher, bool saveAllScenes )
    : m_eventHandler{ std::make_unique<core::SceneEventHandler>( pDispatcher ) }
    , m_saveAllScenes{ saveAllScenes }
{
}

core::SceneManager::~SceneManager()
{
}

auto core::SceneManager::addScene( std::string_view name ) -> Scene*
{
    std::string sceneName = name.empty() ? "defaultScene" : std::string{ name };
    std::unique_ptr<Scene> scene = std::make_unique<Scene>( sceneName );
    m_scenes.emplace( sceneName, std::move( scene ) );

    setCurrentScene( sceneName );
    return m_scenes.at( sceneName ).get();
}

void core::SceneManager::removeScene( const std::string& name )
{
    auto it = m_scenes.find( name );
    if ( it != m_scenes.end() )
    {
        m_scenes.erase( it );
    }
}

auto core::SceneManager::getCurrentScene() -> Scene*
{
    if ( m_scenes.empty() )
    {
        throw std::runtime_error( "No scenes present!!!" );
    }

    auto it = m_scenes.find( m_currentScene );

    if ( it == m_scenes.end() )
    {
        throw std::runtime_error( "Current scene is not set!!!" );
    }

    return it->second.get();
}

auto core::SceneManager::getScenes() -> std::unordered_map<std::string, std::unique_ptr<Scene>>&
{
    return m_scenes;
}

void core::SceneManager::setCurrentScene( const std::string& sceneName )
{
    // TODO this might not be ok, i'd like to set it with a Scene* or reference
    m_currentScene = sceneName;
}

auto core::SceneManager::getEventHandler() -> SceneEventHandler*
{
    return m_eventHandler.get();
}

auto core::SceneManager::saveScenes() -> void
{
    auto saveEntityTransformComponent = []( utilities::JsonSerializer* s, core::TransformComponent& transform ) {
        s->startObject( "transform" )
            .saveVec3( "translation", transform.translation )
            .saveVec3( "rotation", transform.rotation )
            .saveVec3( "scale", transform.scale )
            .endObject();
    };

    auto saveEntityMeshComponent = []( utilities::JsonSerializer* s, core::MeshComponent& meshComponent ) {
        if ( !meshComponent.pMesh )
            return;

        s->startObject( "mesh" ).addKeyValuePair( "path", meshComponent.pMesh->getPath() ).endObject();
    };

    if ( !m_saveAllScenes )
    {
        Scene* scene = getCurrentScene();
        std::string sceneFilename = scene->getName() + ".kscene";
        std::filesystem::path scenePath = std::filesystem::current_path() / "editor" / "scenes" / sceneFilename;

        // this also creates the output file
        std::unique_ptr<utilities::JsonSerializer> serializer =
            std::make_unique<utilities::JsonSerializer>( scenePath.string() );

        serializer->startDocument().startArray( "entities" );

        auto view = scene->getEnttRegistry().view<core::IdentifierComponent>(
            entt::exclude<core::DirectionalLightComponent, core::PerspectiveCameraComponent> );

        view.each( [&]( const entt::entity& id, core::IdentifierComponent& idComponent ) {
            Entity entity{ scene->getRegistry(), id };

            // this could be useful since we can also unhash it so we can create each entity in order
            // uint32_t entityId = static_cast<uint32_t>( entity.getEntityId() );
            // std::string idHash = std::to_string( std::hash<uint32_t>{}( entityId ) );

            serializer->startObject();

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
        return;
    }

    // serialize all scenes
    for ( const auto& [sceneName, scene] : m_scenes )
    {
        std::string sceneFilename = sceneName + ".kscene";
        std::filesystem::path scenePath = std::filesystem::current_path() / "editor" / "scenes" / sceneFilename;

        // this also creates the output file
        std::unique_ptr<utilities::JsonSerializer> serializer =
            std::make_unique<utilities::JsonSerializer>( scenePath.string() );

        serializer->startDocument().startArray( "entities" );

        // serialize except cameras and lights for now
        auto view = scene->getEnttRegistry().view<core::IdentifierComponent>(
            entt::exclude<core::DirectionalLightComponent, core::PerspectiveCameraComponent> );

        view.each( [&]( const entt::entity& id, core::IdentifierComponent& idComponent ) {
            Entity entity{ scene->getRegistry(), id };

            // this could be useful since we can also unhash it so we can create each entity in order
            // uint32_t entityId = static_cast<uint32_t>( entity.getEntityId() );
            // std::string idHash = std::to_string( std::hash<uint32_t>{}( entityId ) );

            serializer->startObject();

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
}
