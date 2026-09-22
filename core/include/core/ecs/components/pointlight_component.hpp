#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <yaml-cpp/yaml.h>
#include "resources/pointlight.hpp"
#include "utilities/utils/yaml_utils.hpp"

namespace core
{
struct PointLightComponent
{
    uint32_t pointLightIndex{ 0u };
};
} // namespace core
