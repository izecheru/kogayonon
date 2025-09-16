#pragma once
#include <entt/entt.hpp>
#include "core/ecs/registry.h"

namespace kogayonon_core
{
class Entity
{
  public:
    Entity(Registry& registry);
    Entity(Registry& registry, const std::string& name);
    Entity(Registry& registry, entt::entity entity);
    virtual ~Entity() = default;

    template <typename TComponent>
    inline bool hasComponent()
    {
        auto& registry = m_registry.getRegistry();
        return registry.all_of<TComponent>(m_entity);
    }

    template <typename TComponent>
    inline entt::view<TComponent>& getView()
    {
        auto& registry = m_registry.getRegistry();
        return registry.view<TComponent>();
    }

    template <typename TComponent>
    inline TComponent& getComponent()
    {
        auto& registry = m_registry.getRegistry();
        return registry.get<TComponent>(m_entity);
    }

    template <typename TComponent, typename... Args>
    inline void addComponent(Args&&... args)
    {
        auto& registry = m_registry.getRegistry();
        registry.emplace<TComponent>(m_entity, std::forward<Args>(args)...);
    }

    template <typename TComponent>
    inline void removeComponent()
    {
        auto& registry = m_registry.getRegistry();
        registry.remove<TComponent>(m_entity);
    }

    template <typename TComponent, typename... Args>
    inline void replaceComponent(Args&&... args)
    {
        auto& registry = m_registry.getRegistry();
        if (registry.all_of<TComponent>(m_entity))
        {
            return registry.replace<TComponent>(m_entity, std::forward<Args>(args)...);
        }
        else
        {
            return registry.emplace<TComponent>(m_entity, std::forward<Args>(args)...);
        }
    }

  private:
    entt::entity m_entity;
    Registry& m_registry;
};
} // namespace kogayonon_core