#include "core/asset_manager/asset_manager.h"

#include <stb_image.h>

#include <glm/glm.hpp>

#include "core/asset_manager/loader/model_loader.h"
#include "core/asset_manager/loader/texture_loader.h"
#include "core/context_manager/context_manager.h"
#include "core/klogger/klogger.h"
#include "core/serialize/mesh_serializer.h"
#include "core/task/task_manager.h"

namespace kogayonon
{
  // Adds an already serialized model to the model map
  void AssetManager::addModel(const std::string& path)
  {
    ContextManager::task_manager()->enqueue([this, path]() {
      {
        std::unique_lock lock(m_model_map_mutex);
        Model model;
        std::ifstream in{};
        MeshSerializer::getInstance()->deserializeMeshes(path, model, in);

        // we set it loaded
        model.setLoaded();
        m_models.insert({path, model});
      }
    });
  }

  void AssetManager::addTexture(const std::string& path) {}

  // Adds a model to the map and should serialize it for later use
  void AssetManager::addModel(const cgltf_data* data, const std::string& model_path)
  {
    ContextManager::task_manager()->enqueue([this, data, model_path]() {
      {
        std::unique_lock lock(m_model_map_mutex);
        Model model;
        ModelLoader::getInstance()->assignModelMeshes(data, model.getMeshes());
        std::ofstream out{};
        KLogger::log(LogType::INFO, "Loaded ", model.getMeshes().size(), " meshes for ", model_path);

        // TODO serialize the model here

        // we set it loaded
        model.setLoaded();
        m_models.insert({model_path, model});
      }
    });
  }

  void AssetManager::addTexture(const std::string& model_path, const cgltf_data* data)
  {
    TextureLoader::getInstance()->pushTexture(
        model_path,
        [](const Texture& texture) // called after loading the texture
        { KLogger::log(LogType::INFO, "Texture ", texture.path, " loaded"); },
        m_texture_map_mutex, m_loaded_textures, data);
  }

  void AssetManager::initializeModel(const std::string& path)
  {
    if (path.find(".bin") != std::string::npos)
    {
      // we have to deserialize the model and so on
      addModel(path);
      addTexture(path);
    }
    else
    {
      // we got a gltf file to parse
      cgltf_options options = {};
      if (cgltf_result result = cgltf_parse_file(&options, path.c_str(), &m_data); result != cgltf_result_success)
      {
        KLogger::log(LogType::ERROR, "Failed to load glTF file:", path);
      }

      if (auto result = cgltf_load_buffers(&options, m_data, path.c_str()); result != cgltf_result_success)
      {
        throw std::runtime_error("Reulst is bad");
      }
      cgltf_validate(m_data);
      assert(m_data != nullptr);
      addModel(m_data, path);
      addTexture(path, m_data);
    }
  }

  Model& AssetManager::getModel(const std::string& path)
  {
    if (m_models.count(path) <= 0)
    {
      throw std::runtime_error("Map does not have that model");
    }
    return m_models.at(path);
  }

  std::unordered_map<std::string, Model>& AssetManager::getModelMap()
  {
    return m_models;
  }
} // namespace kogayonon