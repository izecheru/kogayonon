#pragma once
#include <entt/entt.hpp>
#include <memory>
using namespace entt::literals;

namespace core
{
class Registry
{
  public:
    Registry()
        : m_registry{}
    {
    }

    ~Registry() = default;

    inline bool isValid( entt::entity entity ) const
    {
        return m_registry.valid( entity );
    }

    void removeEntity( entt::entity entity )
    {
        if ( isValid( entity ) )
            m_registry.destroy( entity );
    }

    template <typename TComponent, typename... Args>
    inline auto emplaceComponent( entt::entity entity, Args&&... args ) -> TComponent&
    {
        return m_registry.emplace_or_replace<TComponent>( entity, std::forward<Args>( args )... );
    }

    template <typename TComponent>
    inline auto removeComponent( entt::entity entity )
    {
        m_registry.remove<TComponent>( entity );
    }

    template <typename TComponent, typename... Args>
    auto addComponent( entt::entity entity, Args&&... args ) -> TComponent&
    {
        return m_registry.emplace<TComponent>( entity, std::forward<Args>( args )... );
    }

    template <typename TComponent>
    inline auto getComponent( entt ::entity entityId ) -> TComponent&
    {
        return m_registry.get<TComponent>( entityId );
    }

    template <typename TComponent>
    inline auto tryGetComponent( entt ::entity entityId ) -> TComponent*
    {
        return m_registry.try_get<TComponent>( entityId );
    }

    template <typename TComponent>
    inline bool hasComponent( entt ::entity entityId )
    {
        return m_registry.any_of<TComponent>( entityId );
    }

    inline auto createEntity() -> entt::entity
    {
        return m_registry.create();
    }

    inline auto getRegistry() -> entt::registry&
    {
        return m_registry;
    }

    inline void clearRegistry()
    {
        m_registry.clear();
    }

    template <typename TContext>
    inline auto addToContext( TContext context ) -> void
    {
        m_registry.ctx().emplace<TContext>( context );
    }

    template <typename TContext>
    inline auto getContext() -> TContext&
    {
        return m_registry.ctx().get<TContext>();
    }

    template <typename TContext>
    inline bool eraseContext()
    {
        return m_registry.ctx().erase<TContext>();
    }

    auto storage()
    {
        return m_registry.storage();
    }

  private:
    entt::registry m_registry;
};

} // namespace core