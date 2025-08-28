#include "scene_manager.h"

#include "klogger/klogger.h"

namespace kogayonon
{
int SceneManager::getCurrentScene()
{
  auto& it = m_scenes.find(m_current_scene);
  if (it != m_scenes.end())
  {
    return it->first;
  }

  return 0;
}

void SceneManager::setCurrentScene(int scene)
{
  auto& it = m_scenes.find(scene);
  if (it != m_scenes.end())
  {
    m_current_scene = it->first;
  }
}

void SceneManager::addScene(int scene)
{
  auto& it = m_scenes.find(scene);
  if (it != m_scenes.end())
  {
    m_scenes.emplace(scene, Scene());
  }
  else
  {
    KLogger::error("Scene already exists");
  }
}

void SceneManager::removeScene(int scene)
{
  auto& it = m_scenes.find(scene);
  if (it != m_scenes.end())
  {
    m_scenes.erase(it);
  }
}

} // namespace kogayonon