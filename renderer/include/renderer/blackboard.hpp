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
    inline auto addToStorage( Args&&... args ) -> void;

    template <typename T>
    inline auto removeFromStorage() -> void;

    template <typename T>
    inline T& get();

  private:
    std::unordered_map<entt::id_type, entt::meta_any> m_storage;
};

#include "renderer/blackboard.inl"
} // namespace rendering