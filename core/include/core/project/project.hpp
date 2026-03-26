#pragma once
#include <filesystem>
#include <string>

namespace core
{
struct KogayononProject
{
  // title of the project
  std::string title{ "none" };

  // path of the project file
  std::filesystem::path path;
};
} // namespace core