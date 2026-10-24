#pragma once
#include "glm/glm.hpp"

namespace core
{
struct PlayerInputComponent
{
    glm::vec3 move{ 0.0f };
    glm::vec3 look{ 0.0f };

    bool forward{ false };
    bool left{ false };
    bool right{ false };
    bool back{ false };
    bool jump{ false };
    bool sprint{ false };
};
} // namespace core