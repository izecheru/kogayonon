#pragma once
#include <entt/entt.hpp>
#include <memory>

namespace kogayonon_core {
class Registry
{
  public:
    Registry() : m_registry(std::make_shared<entt::registry>()) {}

    ~Registry() = default;

    inline bool isValid(entt::entity entity) const
    {
        return m_registry->valid(entity);
    }

    inline entt::registry& getRegistry()
    {
        return *m_registry;
    }

    inline void clearRegistry()
    {
        m_registry->clear();
    }

    template <typename TContext>
    inline TContext& addToContext(TContext context)
    {
        return m_registry->ctx().emplace<TContext>(context);
    }

    template <typename TContext>
    inline TContext& getContext()
    {
        return m_registry->ctx().get<TContext>();
    }

    template <typename TContext>
    inline bool eraseContext()
    {
        return m_registry->ctx().erase<TContext>();
    }

  private:
    std::shared_ptr<entt::registry> m_registry;
};
} // namespace kogayonon_core