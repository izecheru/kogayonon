#pragma once
#include <glm/glm.hpp>

#include "core/key_codes.h"
#include "core/mouse_codes.h"

#include <unordered_set>

namespace kogayonon
{
  class Input
  {
  public:
    static bool isKeyPressed(KeyCode key);
    static bool isMouseButtonPressed(MouseCode button);
    static glm::vec2 getMousePos();
    static float getMouseX();
    static float getMouseY();
  };
}

