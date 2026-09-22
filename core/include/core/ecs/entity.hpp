#pragma once
#include <entt/entt.hpp>
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

  private:
    Registry* m_registry;
    entt::entity m_entity;
};

#include "core/ecs/entity.inl"
} // namespace core