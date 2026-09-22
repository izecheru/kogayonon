#include "core/ecs/entity.hpp"
#include "core/ecs/components/identifier_component.hpp"

namespace core
{
Entity::Entity( Registry* registry, const std::string& name )
    : m_registry{ registry }
    , m_entity{ registry->createEntity() }
{
    addComponent<IdentifierComponent>(
        IdentifierComponent{ .name = name, .type = EntityType::None, .group = "DefaultGroup" } );
}

Entity::Entity( Registry* registry )
    : m_registry{ registry }
    , m_entity{ registry->createEntity() }
{
}

Entity::Entity( Registry* registry, entt::entity entity )
    : m_registry{ registry }
    , m_entity{ entity }
{
}

core::Entity::Entity( Registry* registry, entt::entity entity, const std::string& name )
    : Entity{ registry, entity }
{
    removeComponent<IdentifierComponent>();
    addComponent<IdentifierComponent>(
        IdentifierComponent{ .name = name, .type = EntityType::None, .group = "DefaultGroup" } );
}

Entity::Entity( const Entity& other )
    : m_registry{ other.m_registry }
    , m_entity{ other.m_entity }
{
    auto& id = m_registry->getComponent<IdentifierComponent>( other.getEntityId() );

    addComponent<IdentifierComponent>( IdentifierComponent{ .name = id.name, .type = id.type, .group = id.group } );
}

Entity::Entity( Entity&& other ) noexcept
    : m_registry{ other.m_registry }
    , m_entity{ other.m_entity }
{
    auto& id = m_registry->getComponent<IdentifierComponent>( other.getEntityId() );
    addComponent<IdentifierComponent>( IdentifierComponent{ .name = id.name, .type = id.type, .group = id.group } );

    other.m_entity = entt::null;
    other.m_registry = nullptr;
}

Entity& Entity::operator=( const Entity& other )
{
    if ( this != &other )
    {
        auto& id = m_registry->getComponent<IdentifierComponent>( other.getEntityId() );
        addComponent<IdentifierComponent>( IdentifierComponent{ .name = id.name, .type = id.type, .group = id.group } );
        this->m_entity = other.getEntityId();
        this->m_registry = other.m_registry;
    }

    return *this;
}

Entity& Entity::operator=( Entity&& other ) noexcept
{
    if ( this != &other )
    {
        this->m_entity = other.m_entity;
        this->m_registry = other.m_registry;

        auto& id = m_registry->getComponent<IdentifierComponent>( other.getEntityId() );
        addComponent<IdentifierComponent>( IdentifierComponent{ .name = id.name, .type = id.type, .group = id.group } );

        other.m_entity = entt::null;
        other.m_registry = nullptr;
    }

    return *this;
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

auto Entity::getName() -> std::string
{
    return getComponent<IdentifierComponent>().name;
}

auto Entity::getGroup() -> std::string
{
    return getComponent<IdentifierComponent>().group;
}

auto Entity::getType() -> EntityType
{
    return getComponent<IdentifierComponent>().type;
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

} // namespace core
