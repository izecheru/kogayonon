#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include "core/scene/scene.hpp"

namespace kogayonon_core
{
struct KogayononProject
{
  // name of the project
  std::string name;

  // path of the project file
  std::filesystem::path path;

  // map of scenes that the project has
  std::unordered_map<std::string, std::shared_ptr<Scene>> scenes;
};
} // namespace kogayonon_core
