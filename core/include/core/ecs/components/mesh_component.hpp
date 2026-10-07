#pragma once
#include <cinttypes>
#include <entt/entt.hpp>
#include "resources/mesh.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace core
{
struct MeshComponent
{
    std::string meshPath{ "" };
    resources::Mesh* pMesh{ nullptr };
};
} // namespace core