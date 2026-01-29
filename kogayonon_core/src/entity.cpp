#include "core/ecs/entity.hpp"
#include "core/ecs/components/identifier_component.hpp"

namespace kogayonon_core
{
Entity::Entity( Registry* registry, const std::string& name )
    : m_entity{ registry->createEntity() }
    , m_registry{ registry }
{
  addComponent<IdentifierComponent>(
    IdentifierComponent{ .name = name, .type = EntityType::None, .group = "DeafultGroup" } );
}

Entity::Entity( Registry* registry )
    : Entity{ registry, "EntityName" }
{
}

Entity::Entity( Registry* registry, entt::entity entity )
    : m_registry{ registry }
    , m_entity{ entity }
{
  addComponent<IdentifierComponent>(
    IdentifierComponent{ .name = "DefaultName", .type = EntityType::None, .group = "DeafultGroup" } );
}

Entity::Entity( Registry* registry, entt::entity entity, const std::string& name )
    : m_registry{ registry }
    , m_entity{ entity }
{
  addComponent<IdentifierComponent>( IdentifierComponent{ .name = name } );
}

Entity::Entity( const Entity& other )
    : m_entity{ other.m_entity }
    , m_registry{ other.m_registry }
{
}

Entity::Entity( Entity&& other ) noexcept
    : m_entity{ other.m_entity }
    , m_registry{ other.m_registry }
{
  other.m_entity = entt::null;
  other.m_registry = nullptr;
}

Entity& Entity::operator=( const Entity& other )
{
  if ( this != &other )
  {
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

void Entity::createLuaBindings( sol::state& lua )
{
  // TODO make more constructors, getters setters for
  // those are the entity types
  lua.new_enum<EntityType>( "EntityType",
                            { { "None", EntityType::None },
                              { "Camera", EntityType::Camera },
                              { "EditorCamera", EntityType::EditorCamera },
                              { "Object", EntityType::Object } } );

  // expose Entity to lua
  lua.new_usertype<Entity>(
    "Entity",
    sol::call_constructor,
    sol::constructors<Entity( const Entity& ), Entity( Entity&& ), Entity( Registry*, const std::string& )>(),
    // component functions exposed to lua
    "addComponent",
    []( Entity& self, const sol::table& component, sol::this_state currentState ) -> sol::object {
      if ( !component.valid() )
        return sol::lua_nil_t{};

      auto metaAny = invokeMetaFunc( getTypeId( component ), "add_component"_hs, self, component, currentState );

      return metaAny ? metaAny.cast<sol::reference>() : sol::lua_nil_t{};
    },

    "hasComponent",
    []( Entity& self, const sol::table& component ) {
      if ( !component.valid() )
        return false;

      const auto metaAny = invokeMetaFunc( deduceType( component ), "has_component"_hs, self );
      return metaAny ? metaAny.cast<bool>() : false;
    },

    "emplaceComponent",
    []( Entity& self, const sol::table& comp, sol::this_state currentState ) -> sol::object {
      if ( !comp.valid() )
        return sol::lua_nil_t{};

      const auto metaAny = invokeMetaFunc( getTypeId( comp ), "emplace_component"_hs, self, comp, currentState );
      return metaAny ? metaAny.cast<sol::reference>() : sol::lua_nil_t{};
    },

    "removeComponent",
    []( Entity& self, const sol::table& component ) {
      if ( component.valid() )
        invokeMetaFunc( deduceType( component ), "remove_component"_hs, self );
    },

    "getComponent",
    []( Entity& self, const sol::table& component, sol::this_state currentState ) -> sol::object {
      if ( !component.valid() )
        return sol::lua_nil_t{};

      auto metaAny = invokeMetaFunc( getTypeId( component ), "get_component"_hs, self, component, currentState );

      return metaAny ? metaAny.cast<sol::reference>() : sol::lua_nil_t{};
    },
    // i named this with get instead of just entityId cause i want the func() like syntax in lua to remember what name
    // i gave to the exposed to lua variable
    "getEntityId",
    &Entity::getEntityId,
    // getters and setters
    "name",
    sol::property( &Entity::getName, &Entity::setName ),
    "type",
    sol::property( &Entity::getType, &Entity::setType ),
    "group",
    sol::property( &Entity::getGroup, &Entity::setGroup ) );
}

} // namespace kogayonon_core