#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/entity_types.hpp"
#include "core/ecs/registry.hpp"
#include "core/utils/meta_utilities.hpp"
using namespace entt::literals;

namespace core
{

/**
 * @brief Just a wrapper for entity related functions for the registry
 */
class Entity
{
  public:
    explicit Entity( Registry* registry, const std::string& name );
    explicit Entity( Registry* registry );
    explicit Entity( Registry* registry, entt::entity entity );
    explicit Entity( Registry* registry, entt::entity entity, const std::string& name );

    Entity( const Entity& other );
    Entity( Entity&& other ) noexcept;

    Entity& operator=( const Entity& other );
    Entity& operator=( Entity&& other ) noexcept;

    virtual ~Entity() = default;

    void setName( const std::string& name );
    void setGroup( const std::string& group );
    void setType( const EntityType& type );

    auto getName() -> std::string;
    auto getGroup() -> std::string;
    auto getType() -> EntityType;

    bool isType( const EntityType& type );
    bool isGroup( const std::string& group );

    inline void removeEntity()
    {
        auto& registry = m_registry->getRegistry();
        registry.destroy( m_entity );
    }

    template <typename TComponent>
    inline bool hasComponent();

    template <typename TComponent>
    inline auto tryGetComponent() -> TComponent*;

    template <typename TComponent>
    inline auto getComponent() -> TComponent&;

    template <typename TComponent, typename... Args>
    inline auto addComponent( Args&&... args ) -> TComponent&;

    template <typename TComponent>
    inline void removeComponent();

    template <typename TComponent, typename... Args>
    inline void replaceComponent( Args&&... args );

    inline auto getEntityId() const -> entt::entity
    {
        return m_entity;
    }

    static void createLuaBindings( sol::state& lua );

  private:
    Registry* m_registry;
    entt::entity m_entity;
};

template <typename TComponent>
bool has_component( Entity& entity )
{
    return entity.hasComponent<TComponent>();
}

template <typename TComponent>
auto add_component( Entity& entity, const sol::table& comp, sol::this_state currentState )
{
    auto& component =
        entity.addComponent<TComponent>( comp.valid() ? std::move( comp.as<TComponent>() ) : TComponent{} );
    // this is what we need the current lua state for
    return sol::make_reference( currentState, std::ref( component ) );
}

template <typename TComponent>
auto get_component( Entity& entity, sol::this_state currentState )
{
    auto& component = entity.getComponent<TComponent>();
    return sol::make_reference( currentState, std::ref( component ) );
}

template <typename TComponent>
auto emplace_component( Entity& entity, const sol::table& comp, sol::this_state currentState )
{
    auto& component =
        entity.addComponent<TComponent>( comp.valid() ? std::move( comp.as<TComponent&&>() ) : TComponent{} );

    // this is what we need the current lua state for
    return sol::make_reference( currentState, std::ref( component ) );
}

template <typename TComponent>
auto remove_component( Entity& entity )
{
    entity.removeComponent<TComponent>();
}

template <typename TComponent>
void registerMetaComponent()
{
    entt::meta_factory<TComponent>()
        .type( entt::type_hash<TComponent>::value() )
        .template func<&add_component<TComponent>>( "add_component"_hs )
        .template func<&get_component<TComponent>>( "get_component"_hs )
        .template func<&has_component<TComponent>>( "has_component"_hs )
        .template func<&remove_component<TComponent>>( "remove_component"_hs )
        .template func<&emplace_component<TComponent>>( "emplace_component"_hs );
}

#include "core/ecs/entity.inl"
} // namespace core