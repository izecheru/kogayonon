#include "core/scene/scene_manager.hpp"
#include "core/scene/scene.hpp"

core::SceneManager::SceneManager( EventDispatcher* pDispatcher )
    : m_eventHandler{ std::make_unique<core::SceneEventHandler>( pDispatcher ) }
{
}

auto core::SceneManager::addScene( std::string_view name ) -> Scene*
{
  auto sceneName = name.empty() ? "defaultScene" : std::string{ name };
  auto scene = std::make_unique<Scene>( sceneName );
  m_scenes.emplace( name, std::move( scene ) );

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
    return {};

  if ( !m_scenes.contains( m_currentScene ) )
    return {};

  return m_scenes.at( m_currentScene ).get();
}

auto core::SceneManager::getScenes() -> std::unordered_map<std::string, std::unique_ptr<Scene>>&
{
  return m_scenes;
}

void core::SceneManager::setCurrentScene( const std::string& sceneName )
{
  m_currentScene = sceneName;
}

auto core::SceneManager::getEventHandler() -> SceneEventHandler*
{
  return m_eventHandler.get();
}
