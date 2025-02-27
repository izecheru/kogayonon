#pragma once
#include "core/asset_manager/model_loader/model.h"
#include <cgltf/cgltf.h>

namespace kogayonon
{
  class ModelLoader :public Singleton<ModelLoader>
  {
  public:
    void loadTexture(cgltf_texture* texture, const std::string& type, std::vector<Texture>& textures, const std::string& model_dir);
    void buildModel(const std::string& path, std::vector<Mesh>& meshes);

  private:
    std::string m_current_model_path = "";
  };
}
