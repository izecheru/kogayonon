#pragma once
#include "core/event/event.hpp"
#include <entt/entt.hpp>

namespace core
{

/**
 * @brief Enum for ruling out instances where  a window would trigger an entity selection and get the event and process
 * it itself
 */
enum class SelectEntityEventSource
{
  None,
  Viewport_Window,
  Properties_Window,
  Hierarchy_Window
};

class SelectEntityEvent : public IEvent
{
public:
  SelectEntityEvent();
  ~SelectEntityEvent() = default;

  explicit SelectEntityEvent( const entt::entity& ent, const SelectEntityEventSource& source );
  explicit SelectEntityEvent( const SelectEntityEventSource& source );

  /**
   * @brief Get selected entity Id
   * @return
   */
  auto getEntityId() const -> entt::entity;

  /**
   * @brief We get the event source
   * @return An enum value that can be used for event filtering
   */
  auto getEventSource() const -> SelectEntityEventSource;

private:
  entt::entity m_entity;
  SelectEntityEventSource m_source;
};

class DeleteEntityEvent : public IEvent
{
public:
  explicit DeleteEntityEvent( const entt::entity& entityId );
  DeleteEntityEvent() = default;

private:
  entt::entity m_entityId{ entt::null };
};

class AddEntityEvent : public IEvent
{
public:
  AddEntityEvent()
  {
    m_name = "Default";
  }

  explicit AddEntityEvent( const std::string_view name )
      : m_name{ name }
  {
  }

private:
  std::string m_name;
};

} // namespace core