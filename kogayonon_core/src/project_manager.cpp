#include "core/project/project_manager.hpp"

namespace kogayonon_core
{
static void ProjectManager::createProject( const std::string& name )
{
  m_project = KogayononProject{};
}
} // namespace kogayonon_core