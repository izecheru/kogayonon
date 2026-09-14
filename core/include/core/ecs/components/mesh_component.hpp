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
  resources::Mesh* pMesh{ nullptr };
};
} // namespace core