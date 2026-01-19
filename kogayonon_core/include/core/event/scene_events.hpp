#pragma once
#include <entt/entt.hpp>
#include "core/event/event.hpp"

namespace kogayonon_core
{

/**
 * @brief Enum for ruling out instances where  a window would trigger an entity selection and get the event and process
 * it itself
 */
enum class SelectEntityEventSource
{
  None,
  ViewportWindow,
  PropertiesWindow,
  HierarchyWindow
};

class SelectEntityEvent : public IEvent
{
public:
  SelectEntityEvent();
  ~SelectEntityEvent() = default;

  explicit SelectEntityEvent( entt::entity ent, SelectEntityEventSource source );

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

} // namespace kogayonon_core