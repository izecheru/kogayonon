#pragma once
#include "entt/meta/meta.hpp"

namespace rendering
{
class Blackboard
{
  public:
    Blackboard() = default;
    ~Blackboard() = default;

    template <typename T, typename... Args>
    auto addToStorage( Args&&... args ) -> void;

    template <typename T>
    auto removeFromStorage() -> void;

    template <typename T>
    auto get() -> T&;

  private:
    std::unordered_map<entt::id_type, entt::meta_any> m_storage;
};

#include "renderer/blackboard.inl"
} // namespace rendering