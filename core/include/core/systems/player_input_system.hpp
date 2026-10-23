#pragma once
#include "entt/entity/fwd.hpp"
#include "utilities/input/key_codes.hpp"
#include "core/input/mouse_events.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "glm/glm.hpp"

namespace core
{
class EventDispatcher;

class PlayerInputSystem
{
  public:
    explicit PlayerInputSystem( EventDispatcher* dispatcher );
    ~PlayerInputSystem() = default;

    auto update( entt::registry& reg, float delta ) -> void;

  private:
    auto disableEditorCamera( entt::registry& reg ) -> void;
};
} // namespace core