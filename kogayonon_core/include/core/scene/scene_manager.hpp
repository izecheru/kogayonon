#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace kogayonon_core
{
class Scene;
}

namespace kogayonon_core
{
class SceneManager
{
public:
  static void addScene( std::shared_ptr<Scene> scene );
  static void removeScene( const std::string& name );
  static auto getCurrentScene() -> std::weak_ptr<Scene>;
  static auto getScenes() -> std::unordered_map<std::string, std::shared_ptr<Scene>>&;
  static void setCurrentScene( const std::string& sceneName );

private:
  SceneManager() = delete;
  ~SceneManager() = delete;
  SceneManager operator=( const SceneManager& ) = delete;

private:
  static inline std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
  static inline std::string m_currentScene{ "none" };
};
} // namespace kogayonon_core