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

  inline entt::entity createEntity()
  {
    return m_pRegistry->create();
  }

  inline entt::registry& getRegistry()
  {
    return *m_pRegistry;
  }

  inline void clearRegistry()
  {
    m_pRegistry->clear();
  }

  template <typename TContext>
  inline TContext& addToContext( TContext context )
  {
    return m_pRegistry->ctx().emplace<TContext>( context );
  }

  template <typename TContext>
  inline TContext& getContext()
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