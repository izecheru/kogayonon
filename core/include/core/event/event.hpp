#pragma once
#include "entt/meta/meta.hpp"

namespace core
{
// THIS might not be needed since a listner listens to a certain event not a certain event TYPE
enum EventType : uint32_t
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

    auto getEventType() const -> EventType
    {
        return m_type;
    }

    template <typename T, typename... Args>
    auto addUserData( Args&&... args ) -> void
    {
        m_userData = entt::meta_any( T{ std::forward<Args>( args )... } );
    }

    template <typename T>
    auto getUserData() -> T&
    {
        return m_userData.cast<T&>();
    }

  protected:
    entt::meta_any m_userData;
    EventType m_type{ EventType::None };
};
} // namespace core