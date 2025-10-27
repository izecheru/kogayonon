#pragma once
#include <cinttypes>
#include <vector>
#include "resources/mesh.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_core
{

/**
 * @brief An object has one or many meshes, wether we treat it as a single entity or replicate the submesh hierarchy
 * depends on the user desire to do so
 */
struct MeshComponent
{
  kogayonon_resources::Mesh* pMesh;
  bool staticMesh{ false };
  bool loaded{ false };
};
} // namespace kogayonon_core