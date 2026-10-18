#pragma once
#include <entt/entt.hpp>
#include "resources/model.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace core
{

struct ModelComponent
{
    std::string meshPath{ "" };
    resources::Model* pMesh{ nullptr };
};

} // namespace core