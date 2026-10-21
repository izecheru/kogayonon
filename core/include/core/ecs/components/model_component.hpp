#pragma once
#include <entt/entt.hpp>
#include "resources/model.hpp"
#include "resources/texture.hpp"
#include "resources/vertex.hpp"

namespace core
{

struct ModelComponent
{
    std::string modelPath{ "" };
    resources::Model* pModel{ nullptr };
};

} // namespace core