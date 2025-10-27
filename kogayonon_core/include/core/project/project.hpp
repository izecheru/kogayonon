#pragma once
#include <filesystem>
#include <string>

namespace kogayonon_core
{
struct KogayononProject
{
  // title of the project
  std::string title{ "none" };

  // path of the project file
  std::filesystem::path path;
};
} // namespace kogayonon_core