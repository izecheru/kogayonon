#pragma once
#include <string>

namespace kogayonon_core
{
enum class EventType
{
  None = 0,

  // Window events
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMinimzed,

  AppTick,
  AppUpdate,
  AppRender,

  // Input events
  KeyPressed,
  KeyReleased,
  KeyTyped,
  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,
  MouseEntered,
  MouseClicked,

  // Scene related events
  SelectedEntity,
  SaveScene,

  // Project events
  ProjectLoad,

  // File events
  FileModified,
  FileRenamed,
  FileCreated,
  FileDeleted,
};

class IEvent
{
public:
  IEvent() = default;
  virtual ~IEvent() = default;
};
} // namespace kogayonon_core