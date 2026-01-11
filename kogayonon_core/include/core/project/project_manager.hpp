#pragma once
#include <string>
#include "core/project/project.hpp"

namespace kogayonon_core
{
class ProjectManager
{
public:
  static void createProject( const std::string& name, const std::filesystem::path& path );
  static auto getTitle() -> std::string;
  static auto getPath() -> std::filesystem::path;

private:
  ProjectManager() = delete;
  ~ProjectManager() = delete;
  ProjectManager operator=( const ProjectManager& ) = delete;

private:
  static inline KogayononProject m_project;
};
} // namespace kogayonon_core