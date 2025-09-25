#include "core/ecs/entity.hpp"
#include "core/ecs/components/name_component.hpp"

namespace kogayonon_core
{
Entity::Entity( Registry& registry )
    : Entity{ registry, "Entity" }
{
}

Entity::Entity( Registry& registry, const std::string& name )
    : m_registry{ registry }
    , m_entity{ registry.createEntity() }
{
  addComponent<NameComponent>( name );
}

Entity::Entity( Registry& registry, entt::entity entity )
    : m_registry{ registry }
    , m_entity{ entity }
{
}
} // namespace kogayonon_core