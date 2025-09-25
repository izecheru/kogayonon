#pragma once
#include <entt/entt.hpp>
#include "core/ecs/registry.hpp"

namespace kogayonon_core
{
class Entity
{
public:
  explicit Entity( Registry& registry );
  explicit Entity( Registry& registry, const std::string& name );
  explicit Entity( Registry& registry, entt::entity entity );
  virtual ~Entity() = default;

  template <typename TComponent>
  inline bool hasComponent()
  {
    auto& registry = m_registry.getRegistry();
    return registry.all_of<TComponent>( m_entity );
  }

  template <typename TComponent>
  inline entt::view<TComponent>& getView()
  {
    auto& registry = m_registry.getRegistry();
    return registry.view<TComponent>();
  }

  template <typename TComponent>
  inline TComponent* tryGetComponent()
  {
    auto& registry = m_registry.getRegistry();
    return registry.try_get<TComponent>( m_entity );
  }

  template <typename TComponent>
  inline TComponent& getComponent()
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

  inline entt::entity getEnttEntity()
  {
    return m_entity;
  }

private:
  entt::entity m_entity;
  Registry& m_registry;
};
} // namespace kogayonon_core