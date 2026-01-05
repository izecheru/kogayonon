#include "core/ecs/entity.hpp"
#include "core/ecs/components/identifier_component.hpp"

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
  addComponent<IdentifierComponent>(
    IdentifierComponent{ .name = name, .type = EntityType::None, .group = "MainGroup" } );
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
  addComponent<IdentifierComponent>(
    IdentifierComponent{ .name = name, .type = EntityType::None, .group = "MainGroup" } );
}

void Entity::setName( const std::string& name )
{
  auto& idComponent = getComponent<IdentifierComponent>();
  idComponent.name = name;
}

void Entity::setGroup( const std::string& group )
{
  auto& idComponent = getComponent<IdentifierComponent>();
  idComponent.group = group;
}

void Entity::setType( const EntityType& type )
{
  auto& idComponent = getComponent<IdentifierComponent>();
  idComponent.type = type;
}

bool Entity::isType( const EntityType& type )
{
  auto& idComponent = getComponent<IdentifierComponent>();
  return idComponent.type == type;
}

bool Entity::isGroup( const std::string& group )
{
  auto& idComponent = getComponent<IdentifierComponent>();
  return idComponent.group == group;
}

} // namespace kogayonon_core