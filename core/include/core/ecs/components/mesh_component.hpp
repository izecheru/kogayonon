#pragma once
#include <cinttypes>
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include "precompiled/pch.hpp"
#include "resources/mesh.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace core
{
struct MeshComponent
{
  resources::Mesh* pMesh;
  bool staticMesh{ false };
  bool loaded{ false };

  static void createLuaBindings( sol::state& lua )
  {
    lua.new_usertype<MeshComponent>(
      "MeshComponent",
      "typeId",
      entt::type_hash<MeshComponent>::value,
      sol::call_constructor,
      sol::factories( []() { return MeshComponent{ .pMesh = nullptr, .staticMesh = false, .loaded = false }; },
                      []( resources::Mesh* pMesh, bool staticMesh, bool loaded ) {
                        return MeshComponent{ .pMesh = pMesh, .staticMesh = staticMesh, .loaded = loaded };
                      } ),
      "pMesh",
      &MeshComponent::pMesh,
      "staticMesh",
      &MeshComponent::staticMesh,
      "loaded",
      &MeshComponent::loaded );
  }
};
} // namespace core