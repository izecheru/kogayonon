#pragma once
#include "core/key_codes.h"
#include "core/mouse_codes.h"
#include <glm/ext/vector_float2.hpp>

namespace kogayonon
{
  class Input
  {
  public:
    bool isKeyPressed(KeyCode key);
    bool isMouseButtonPressed(MouseCode button);
    glm::vec2 getMousePos();
    float getMouseX();
    float getMouseY();
  };
}
