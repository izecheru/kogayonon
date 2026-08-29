#pragma once
#include "precompiled/pch.hpp"
#include <entt/entt.hpp>

namespace rendering
{
class Blackboard
{
public:
  Blackboard() = default;
  ~Blackboard() = default;

  template <typename T, typename... Args>
  inline auto addToStorage( Args&&... args ) -> void
  {
    constexpr entt::id_type hash = entt::type_hash<T>::value();
    if ( m_storage.contains( hash ) )
    {
      return;
    }

    m_storage[hash] = entt::meta_any( T( std::forward<Args>( args )... ) );
  }

  template <typename T>
  inline T& get()
  {
    return m_storage[entt::type_hash<T>::value()].cast<T&>();
  }

private:
  std::unordered_map<entt::id_type, entt::meta_any> m_storage;
};
} // namespace rendering