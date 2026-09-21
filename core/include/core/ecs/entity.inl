#include "core/ecs/entity.hpp"

template <typename TComponent>
inline auto core::Entity::hasComponent() -> bool
{
    auto& registry = m_registry->getRegistry();
    return registry.any_of<TComponent>( m_entity );
}

template <typename TComponent>
inline auto core::Entity::tryGetComponent() -> TComponent*
{
    auto& registry = m_registry->getRegistry();
    return registry.try_get<TComponent>( m_entity );
}

template <typename TComponent>
inline auto core::Entity::getComponent() -> TComponent&
{
    auto& registry = m_registry->getRegistry();
    return registry.get<TComponent>( m_entity );
}

template <typename TComponent, typename... Args>
inline auto core::Entity::addComponent( Args&&... args ) -> TComponent&
{
    auto& registry = m_registry->getRegistry();
    if ( hasComponent<TComponent>() )
        return getComponent<TComponent>();

    return registry.emplace<TComponent>( m_entity, std::forward<Args>( args )... );
}

template <typename TComponent>
inline auto core::Entity::removeComponent() -> void
{
    auto& registry = m_registry->getRegistry();
    registry.remove<TComponent>( m_entity );
}

template <typename TComponent, typename... Args>
inline auto core::Entity::replaceComponent( Args&&... args ) -> void
{
    auto& registry = m_registry->getRegistry();
    if ( registry.all_of<TComponent>( m_entity ) )
    {
        registry.replace<TComponent>( m_entity, std::forward<Args>( args )... );
    }
    else
    {
        registry.emplace<TComponent>( m_entity, std::forward<Args>( args )... );
    }
}
