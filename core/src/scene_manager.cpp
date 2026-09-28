#include "core/scene/scene_manager.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/main_registry.hpp"
#include "rapidjson/istreamwrapper.h"
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
    populateAvailableScenes();
}

core::SceneManager::~SceneManager()
{
}

auto core::SceneManager::addScene( std::string_view name ) -> Scene*
{
    std::string sceneName = name.empty() ? "defaultScene" : std::string{ name };
    auto scene = std::make_unique<Scene>( sceneName );
    m_scenes.emplace( sceneName, std::move( scene ) );

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

    return m_currentScene;
}

auto core::SceneManager::getScenes() -> std::unordered_map<std::string, std::unique_ptr<Scene>>&
{
    return m_scenes;
}

void core::SceneManager::setCurrentScene( const std::string& sceneName )
{
    m_currentScene = m_scenes[sceneName].get();
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

    populateAvailableScenes();
}

auto core::SceneManager::getAvailableScenes() -> std::vector<std::filesystem::path>&
{
    return m_availableScenes;
}

auto core::SceneManager::populateAvailableScenes() -> void
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
    getCurrentScene()->serialize();

    core::Scene* currentScene = getCurrentScene();
    std::string currentSceneName = currentScene->getName();
    m_scenes.erase( currentSceneName );

    core::AssetManager* assetManager = core::MainRegistry::getInstance().getAssetManager();

    assetManager->recreate();

    core::Scene* newScene = addScene( p.stem().string() );
    setCurrentScene( newScene->getName() );
    newScene->deserialize( p );

    graphics::VulkanContext* ctx = core::MainRegistry::getInstance().getVulkanContext();
    VkExtent2D extent = ctx->swapchain->getSwapchainExtent();

    core::DirectionalLightComponent directionalLight{};
    core::Entity direciontalLightEnt{ newScene->getRegistry(), "DefaultDirecitonalLight" };
    direciontalLightEnt.addComponent<core::DirectionalLightComponent>( directionalLight );

    core::PerspectiveCameraComponent cameraComponent{};
    cameraComponent.props.farView = 500.0f;
    cameraComponent.ubo.view =
        glm::lookAt( cameraComponent.props.eye, cameraComponent.props.center, cameraComponent.props.up );
    cameraComponent.ubo.projection = glm::perspective( glm::radians( cameraComponent.props.fov ),
                                                       extent.width / (float)( extent.height ),
                                                       cameraComponent.props.nearView,
                                                       cameraComponent.props.farView );
    cameraComponent.ubo.projection[1][1] *= -1;
    cameraComponent.props.extent = { (float)extent.width, (float)extent.height };
    cameraComponent.isUsed = true;

    core::Entity entity{ newScene->getRegistry(), "DefaultCamera" };
    entity.addComponent<core::PerspectiveCameraComponent>( cameraComponent );
}
