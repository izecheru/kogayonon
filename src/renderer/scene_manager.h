#pragma once
#include <unordered_map>

#include "scene.h"

namespace kogayonon
{
class SceneManager
{
public:
  SceneManager() = default;
  ~SceneManager() = default;

  // scene getter and setter as well as switching and other stuff related to it
  Scene& getCurrentScene();
  void setCurrentScene(int scene);
  void addScene(int scene);
  void removeScene(int scene);
  // ---------------------

private:
  int m_current_scene = 0;
  std::unordered_map<int, Scene> m_scenes;
};
} // namespace kogayonon