#include "core/scene/scene.hpp"
#include "core/ecs/registry.hpp"

namespace kogayonon_core
{
Scene::Scene( const std::string& name )
    : m_name( name )
    , m_pRegistry( std::make_unique<Registry>() )
{
}

Registry& Scene::getRegistry()
{
  return *m_pRegistry.get();
}

entt::registry& Scene::getEnttRegistry()
{
  return m_pRegistry->getRegistry();
}

std::string Scene::getName() const
{
  return m_name;
}

void Scene::changeName( const std::string& name )
{
  m_name = name;
}

void Scene::removeEntity( entt::entity ent )
{
  auto& registry = m_pRegistry->getRegistry();
  registry.destroy( ent );
}

void Scene::addDefaultEntity()
{
  Entity ent{ *m_pRegistry, "DefaultEntity" };
}
} // namespace kogayonon_core