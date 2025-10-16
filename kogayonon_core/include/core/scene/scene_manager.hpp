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
/**
 * @brief SceneManager singleton, we want this in a lot of places so a singleton should be the best bet, I should also
 * be able to save the scene and load it
 */

class SceneManager
{
public:
  static void addScene( std::shared_ptr<Scene> scene );
  static void removeScene( const std::string& name );
  static std::weak_ptr<Scene> getCurrentScene();
  static void setCurrentScene( const std::string& sceneName );

private:
  SceneManager() = delete;
  ~SceneManager() = delete;
  SceneManager operator=( const SceneManager& ) = delete;

private:
  static inline std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
  static inline std::string m_currentScene;
};
} // namespace kogayonon_core