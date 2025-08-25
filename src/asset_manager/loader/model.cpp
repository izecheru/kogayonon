#include "asset_manager/loader/model.h"

#include <filesystem>

#include "asset_manager/asset_manager.h"
#include "asset_manager/loader/mesh.h"
#include "asset_manager/loader/model_loader.h"
#include "context_manager/context_manager.h"
#include "serialiser/mesh_serialiser.h"
#include "shader/shader.h"

namespace kogayonon
{
  Model::Model(const std::string& path_to_model)
  {
    this->m_path = path_to_model;
    init(path_to_model);
  }

  void Model::init(const std::string& path) const
  {
    auto mgr = ContextManager::getFromContext<AssetManager>(Context::AssetManagerContext);
    mgr->initializeModel(path);
  }

  std::vector<Mesh>& Model::getMeshes()
  {
    return m_meshes;
  }
} // namespace kogayonon