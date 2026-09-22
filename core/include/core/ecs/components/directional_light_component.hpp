#pragma once
#include <yaml-cpp/yaml.h>
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include "resources/directional_light.hpp"
#include "utilities/utils/yaml_utils.hpp"

namespace core
{
struct DirectionalLightComponent
{
    float nearPlane{ 0.1f };
    float farPlane{ 300.0f };
    float orthoSize{ 70.0f };
    float positionFactor{ 20.0f };
};
} // namespace core
