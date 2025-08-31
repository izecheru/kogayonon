#include "scene_manager.h"

#include "klogger/klogger.h"

namespace kogayonon
{
Scene& SceneManager::getCurrentScene()
{
  return m_scenes.at(m_current_scene);
}

// should draw only the current scene
void SceneManager::setCurrentScene(int scene)
{
  auto& it = m_scenes.find(scene);
  // if we don't have this index then we don't have a scene to set
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