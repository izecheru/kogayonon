#include "core/ecs/entity.hpp"
#include "core/ecs/components/name_component.hpp"

namespace kogayonon_core
{
Entity::Entity( Registry& registry )
    : Entity{ registry, "Entity" }
{
}

Entity::Entity( Registry& registry, const std::string& name )
    : m_entity{ registry.createEntity() }
    , m_registry{ registry }
{
  addComponent<NameComponent>( NameComponent{ .name = name } );
}

Entity::Entity( Registry& registry, entt::entity entity )
    : m_entity{ entity }
    , m_registry{ registry }
{
}

Entity::Entity( Registry& registry, entt::entity entity, const std::string& name )
    : m_entity{ entity }
    , m_registry{ registry }
{
  addComponent<NameComponent>( NameComponent{ .name = name } );
}
} // namespace kogayonon_core