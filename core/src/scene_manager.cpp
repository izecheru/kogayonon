#include "core/scene/scene_manager.hpp"
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "utilities/json_serializer/json_serializer.hpp"
#include "core/event/event_dispatcher.hpp"

core::SceneManager::SceneManager( EventDispatcher* pDispatcher, bool saveAllScenes )
    : m_eventHandler{ std::make_unique<core::SceneEventHandler>( pDispatcher ) }
    , m_saveAllScenes{ saveAllScenes }
{
    pDispatcher->addHandler<core::FileEvent, &SceneManager::onFileEvent>( *this );
    populateScenes();
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
        return nullptr;
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
    if ( !m_saveAllScenes )
    {
        Scene* scene = getCurrentScene();
        saveScene( scene );
    }
    else
    {
        // serialize all scenes
        for ( const auto& [sceneName, scene] : m_scenes )
        {
            saveScene( scene.get() );
        }
    }
}

auto core::SceneManager::saveScene( core::Scene* scene ) -> void
{
    scene->serialize();
}

auto core::SceneManager::onFileEvent( core::FileEvent& e ) -> void
{
    namespace fs = std::filesystem;
    static fs::path scenesPath = fs::current_path() / "editor" / "scenes";
    fs::path p{ e.getPath() };

    if ( p.parent_path() != scenesPath )
    {
        return;
    }

    populateScenes();
}

auto core::SceneManager::getAvailableScenes() -> std::vector<std::filesystem::path>&
{
    return m_availableScenes;
}

auto core::SceneManager::populateScenes() -> void
{
    namespace fs = std::filesystem;
    m_availableScenes.clear();

    static fs::path scenesPath = fs::current_path() / "editor" / "scenes";
    std::error_code ec{};
    fs::directory_iterator it{ scenesPath, fs::directory_options::skip_permission_denied, ec };

    if ( ec )
    {
        throw std::runtime_error( "could not iterate directory" );
    }

    for ( const fs::directory_entry& entry : it )
    {
        m_availableScenes.push_back( entry.path() );
    }
}

auto core::SceneManager::switchToScene( const std::filesystem::path& p ) -> void
{
    // save the current scene
    getCurrentScene()->serialize();

    // now clear everything that this scene had, buffers and images and all that
    // but if the new scene has them
    // just skip so we don't waste time loading again
}
