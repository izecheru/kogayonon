#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <sol/sol.hpp>
using namespace entt::literals;

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

  void removeEntity( entt::entity entity )
  {
    if ( isValid( entity ) )
      m_pRegistry->destroy( entity );
  }

  template <typename TComponent, typename... Args>
  inline auto emplaceComponent( entt::entity entity, Args&&... args ) -> TComponent&
  {
    return m_pRegistry->emplace_or_replace<TComponent>( entity, std::forward<Args>( args )... );
  }

  template <typename TComponent>
  inline auto removeComponent( entt::entity entity )
  {
    m_pRegistry->remove<TComponent>( entity );
  }

  template <typename TComponent, typename... Args>
  auto addComponent( entt::entity entity, Args&&... args ) -> TComponent&
  {
    return m_pRegistry->emplace<TComponent>( entity, std::forward<Args>( args )... );
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
    return *m_pRegistry.get();
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

  auto storage()
  {
    return m_pRegistry->storage();
  }

  static void createLuaBindings( sol::state& lua );

private:
  std::shared_ptr<entt::registry> m_pRegistry;
};

} // namespace kogayonon_core