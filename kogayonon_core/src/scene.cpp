#include "core/scene/scene.h"
#include "core/ecs/registry.h"

namespace kogayonon_core
{
Scene::Scene(const std::string& name) : m_pRegistry(std::make_unique<Registry>()), m_name(name)
{
}

Registry& Scene::getRegistry()
{
    return *m_pRegistry.get();
}

std::string Scene::getName() const
{
    return m_name;
}

void Scene::changeName(const std::string& name)
{
    m_name = name;
}
} // namespace kogayonon_core