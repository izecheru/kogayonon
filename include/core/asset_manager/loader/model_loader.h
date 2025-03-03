#pragma once

#include "core/singleton/singleton.h"
#include <unordered_map>
#include <string>
#include "model.h"
#include <mutex>

namespace kogayonon
{
  class ModelLoader :public Singleton<ModelLoader>
  {
  public:
    bool pushModel(const std::string& path, std::function<void(Model&)>callback, std::mutex& t_mutex, std::unordered_map<std::string, Model>& models_map);
    void parseGltf(const std::string& path, std::vector<Mesh>& meshes);
  };
}
