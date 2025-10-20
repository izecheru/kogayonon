#include "core/project/project_manager.hpp"

namespace kogayonon_core
{
void ProjectManager::createProject( const std::string& name, const std::filesystem::path& path )
{
  m_project = KogayononProject{ .title = name, .path = path };
}

std::string ProjectManager::getTitle()
{
  return m_project.title;
}

std::filesystem::path ProjectManager::getPath()
{
  return m_project.path;
}
} // namespace kogayonon_core