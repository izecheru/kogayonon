#pragma once
#include <string>
#include "core/project/project.hpp"

namespace kogayonon_core
{
class ProjectManager
{
public:
  static void createProject( const std::string& name );

private:
  ProjectManager() = default;
  ~ProjectManager() = default;
  ProjectManager operator=( const ProjectManager& ) = delete;

private:
  static inline std::string m_projectName;
  static inline KogayononProject m_project;
};
} // namespace kogayonon_core
