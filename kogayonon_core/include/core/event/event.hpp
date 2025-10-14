#pragma once
#include <string>

namespace kogayonon_core
{
enum class EventType
{
  None = 0,
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMinimzed,
  AppTick,
  AppUpdate,
  AppRender,
  KeyPressed,
  KeyReleased,
  KeyTyped,
  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,
  MouseEntered,
  MouseClicked,

  SelectedEntity,
  SaveScene,

  FileModified,
  FileRenamed,
  FileCreated,
  FileDeleted,
};

// clang-format off
#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
								virtual EventType getEventType() const override { return getStaticType(); }

// clang-format on

class IEvent
{
public:
  IEvent() = default;
  virtual ~IEvent() = default;
  virtual EventType getEventType() const = 0;

  bool handled = false;
};
} // namespace kogayonon_core