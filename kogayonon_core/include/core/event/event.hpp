#pragma once
#include <string>

namespace kogayonon_core
{
// THIS might not be needed since a listner listens to a certain event not a certain event TYPE
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
  ProjectSave,
  ProjectCreate,

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

  inline virtual EventType getEventType()
  {
    return m_type;
  }

private:
  EventType m_type{ EventType::None };
};
} // namespace kogayonon_core