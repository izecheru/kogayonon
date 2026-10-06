#pragma once
#include "core/scene/scene_event_handler.hpp"
#include "core/scene/scene.hpp"
#include "core/event/file_events.hpp"
#include <entt/entt.hpp>

namespace core
{
class SceneManager
{
  public:
    explicit SceneManager( EventDispatcher* pDispatcher, bool saveAllScenes = true );
    ~SceneManager();

    auto addScene( std::string_view name = "" ) -> Scene*;
    auto removeScene( const std::string& name ) -> void;
    auto getCurrentScene() -> Scene*;
    auto setCurrentScene( const std::string& sceneName ) -> void;
    auto getEventHandler() -> SceneEventHandler*;
    auto getScenes() -> std::unordered_map<std::string, std::unique_ptr<Scene>>&;
    auto switchToScene( const std::filesystem::path& p ) -> void;
    auto getAvailableScenes() -> std::vector<std::filesystem::path>&;
    auto applyPendingSwitch() -> void;

    auto saveScenes() -> void;

  private:
    auto saveScene( core::Scene* scene ) -> void;
    auto populateAvailableScenes() -> void;

    auto onFileEvent( core::FileEvent& e ) -> void;

  private:
    bool m_saveAllScenes;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_currentScene;
    std::unique_ptr<SceneEventHandler> m_eventHandler;
    std::vector<std::filesystem::path> m_availableScenes;

    std::filesystem::path m_pendingScenePath;
    bool m_pendingSwitch;
};
} // namespace core