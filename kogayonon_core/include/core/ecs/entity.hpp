#pragma once
#include <entt/entt.hpp>
#include "core/ecs/entity.hpp"
#include "core/ecs/entity_types.hpp"
#include "core/ecs/registry.hpp"

namespace kogayonon_core
{

class Entity
{
public:
  explicit Entity( Registry& registry );
  explicit Entity( Registry& registry, const std::string& name );
  explicit Entity( Registry& registry, entt::entity entity );
  explicit Entity( Registry& registry, entt::entity entity, const std::string& name );
  virtual ~Entity() = default;

  void setName( const std::string& name );
  void setGroup( const std::string& group );
  void setType( const EntityType& type );

  bool isType( const EntityType& type );
  bool isGroup( const std::string& group );

  template <typename TComponent>
  inline bool hasComponent()
  {
    auto& registry = m_registry.getRegistry();
    return registry.any_of<TComponent>( m_entity );
  }

  inline void removeEntity()
  {
    auto& registry = m_registry.getRegistry();
    registry.destroy( m_entity );
  }

  template <typename TComponent>
  inline auto getView() -> entt::view<TComponent>&
  {
    auto& registry = m_registry.getRegistry();
    return registry.view<TComponent>();
  }

  template <typename TComponent>
  inline auto tryGetComponent() -> TComponent*
  {
    auto& registry = m_registry.getRegistry();
    return registry.try_get<TComponent>( m_entity );
  }

  template <typename TComponent>
  inline auto getComponent() -> TComponent&
  {
    auto& registry = m_registry.getRegistry();
    return registry.get<TComponent>( m_entity );
  }

  template <typename TComponent, typename... Args>
  inline void addComponent( Args&&... args )
  {
    auto& registry = m_registry.getRegistry();
    registry.emplace<TComponent>( m_entity, std::forward<Args>( args )... );
  }

  template <typename TComponent>
  inline void removeComponent()
  {
    auto& registry = m_registry.getRegistry();
    registry.remove<TComponent>( m_entity );
  }

  template <typename TComponent, typename... Args>
  inline void replaceComponent( Args&&... args )
  {
    auto& registry = m_registry.getRegistry();
    if ( registry.all_of<TComponent>( m_entity ) )
    {
      registry.replace<TComponent>( m_entity, std::forward<Args>( args )... );
    }
    else
    {
      registry.emplace<TComponent>( m_entity, std::forward<Args>( args )... );
    }
  }

  inline auto getEntityId() const -> entt::entity
  {
    return m_entity;
  }

private:
  entt::entity m_entity;
  Registry& m_registry;
};
} // namespace kogayonon_core