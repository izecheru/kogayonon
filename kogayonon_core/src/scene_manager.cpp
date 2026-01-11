#include "core/scene/scene_manager.hpp"
#include "core/scene/scene.hpp"

namespace kogayonon_core
{
void kogayonon_core::SceneManager::addScene( std::shared_ptr<Scene> scene )
{
  auto name = scene->getName();
  m_scenes.emplace( name, std::move( scene ) );
}

void SceneManager::removeScene( const std::string& name )
{
  auto it = m_scenes.find( name );
  if ( it != m_scenes.end() )
  {
    m_scenes.erase( it );
  }
}

auto SceneManager::getCurrentScene() -> std::weak_ptr<Scene>
{
  if ( m_scenes.empty() )
    return {};

  if ( !m_scenes.contains( m_currentScene ) )
    return {};

  return std::weak_ptr<Scene>( m_scenes.at( m_currentScene ) );
}

auto SceneManager::getScenes() -> std::unordered_map<std::string, std::shared_ptr<Scene>>&
{
  return m_scenes;
}

void SceneManager::setCurrentScene( const std::string& sceneName )
{
  m_currentScene = sceneName;
}
} // namespace kogayonon_core