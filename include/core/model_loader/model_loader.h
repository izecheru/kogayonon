#pragma once
#include "core/model_loader/model.h"

namespace kogayonon
{
  class ModelLoader :public Singleton<ModelLoader>
  {
  public:
    void buildModel(const std::string& path, std::vector<Mesh>& meshes);

  private:
    std::string m_current_model_path;
    std::unordered_map<std::string, Texture> m_loaded_textures;
  };
}
