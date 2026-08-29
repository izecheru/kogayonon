#pragma once
#include "core/scene/scene_event_handler.hpp"
#include "precompiled/pch.hpp"
#include <entt/entt.hpp>

namespace core
{
class Scene;
class EventDispatcher;
} // namespace core

namespace core
{
class SceneManager
{
public:
  SceneManager( EventDispatcher* pDispatcher );
  ~SceneManager() = default;

  auto addScene( std::string_view name = "" ) -> Scene*;
  auto removeScene( const std::string& name ) -> void;
  auto getCurrentScene() -> Scene*;
  auto getScenes() -> std::unordered_map<std::string, std::unique_ptr<Scene>>&;
  auto setCurrentScene( const std::string& sceneName ) -> void;

  auto getEventHandler() -> SceneEventHandler*;

private:
  std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
  std::string m_currentScene{ "none" };
  std::unique_ptr<SceneEventHandler> m_eventHandler;
};
} // namespace core