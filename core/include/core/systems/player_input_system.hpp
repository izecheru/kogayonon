#pragma once
#include "entt/entity/fwd.hpp"
#include "utilities/input/key_codes.hpp"
#include "core/input/mouse_events.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "glm/glm.hpp"

namespace core
{
class KeyMapManager;
class Registry;
} // namespace core

namespace core
{
class EventDispatcher;

class PlayerInputSystem
{
  public:
    explicit PlayerInputSystem( EventDispatcher* dispatcher, Registry* registry );
    ~PlayerInputSystem();

    auto update( entt::registry& reg, float delta ) -> void;

  private:
    std::unique_ptr<KeyMapManager> m_keyMapManager;
    Registry* m_registry;
};
} // namespace core