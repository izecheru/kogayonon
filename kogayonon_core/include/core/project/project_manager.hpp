#pragma once
#include <string>
#include "core/project/project.hpp"

namespace kogayonon_core
{
class ProjectManager
{
public:
  static void createProject( const std::string& name, const std::filesystem::path& path );
  static std::string getTitle();
  static std::filesystem::path getPath();

private:
  ProjectManager() = delete;
  ~ProjectManager() = delete;
  ProjectManager operator=( const ProjectManager& ) = delete;

private:
  static inline KogayononProject m_project;
};
} // namespace kogayonon_core