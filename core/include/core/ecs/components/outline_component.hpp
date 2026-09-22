#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace core
{
struct OutlineComponent
{
    glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
};
} // namespace core