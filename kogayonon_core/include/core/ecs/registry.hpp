#pragma once
#include <entt/entt.hpp>
#include <memory>

namespace kogayonon_core
{
class Registry
{
public:
  Registry()
      : m_pRegistry{ std::make_shared<entt::registry>() }
  {
  }

  ~Registry() = default;

  inline bool isValid( entt::entity entity ) const
  {
    return m_pRegistry->valid( entity );
  }

  template <typename TComponent>
  inline auto getComponent( entt ::entity entityId ) -> TComponent&
  {
    return m_pRegistry->get<TComponent>( entityId );
  }

  template <typename TComponent>
  inline auto tryGetComponent( entt ::entity entityId ) -> TComponent*
  {
    return m_pRegistry->try_get<TComponent>( entityId );
  }

  template <typename TComponent>
  inline bool hasComponent( entt ::entity entityId )
  {
    return m_pRegistry->any_of<TComponent>( entityId );
  }

  inline auto createEntity() -> entt::entity
  {
    return m_pRegistry->create();
  }

  inline auto getRegistry() -> entt::registry&
  {
    return *m_pRegistry;
  }

  inline void clearRegistry()
  {
    m_pRegistry->clear();
  }

  template <typename TContext>
  inline auto addToContext( TContext context ) -> TContext&
  {
    return m_pRegistry->ctx().emplace<TContext>( context );
  }

  template <typename TContext>
  inline auto getContext() -> TContext&
  {
    return m_pRegistry->ctx().get<TContext>();
  }

  template <typename TContext>
  inline bool eraseContext()
  {
    return m_pRegistry->ctx().erase<TContext>();
  }

private:
  std::shared_ptr<entt::registry> m_pRegistry;
};
} // namespace kogayonon_core