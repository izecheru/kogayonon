#pragma once
#include "core/scene/scene_event_handler.hpp"
#include "precompiled/pch.hpp"
#include "core/scene/scene.hpp"
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

    auto saveScenes() -> void;

  private:
    auto saveScene( core::Scene* scene ) -> void;

  private:
    bool m_saveAllScenes;
    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
    std::string m_currentScene;
    std::unique_ptr<SceneEventHandler> m_eventHandler;
};
} // namespace core