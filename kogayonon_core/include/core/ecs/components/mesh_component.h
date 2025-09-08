#pragma once
#include "resources/mesh.h"

namespace kogayonon_core
{
struct MeshComponent
{
    explicit MeshComponent();
    kogayonon_resources::Mesh m_mesh;
};
} // namespace kogayonon_core
