#pragma once
#include <string>
#include <unordered_map>

#include "asset_manager/loader/model.h"

namespace kogayonon
{
/**
 * @brief Scene object that is to be drawn on the SceneViewportWindow frame buffer
 */
class Scene
{
public:
  Scene() = default;
  ~Scene() = default;

  Model& addModelRef(std::string name, Model& m);
  Texture& addTextureRef(std::string name, Texture& t);

  void draw();

private:

  // audio and all other stuff like effects should go here
  // might make a resource manager for scene or something like that
  std::unordered_map<std::string, Model&> m_model_refs;
  std::unordered_map<std::string, Texture&> m_texture_refs;
};
} // namespace kogayonon