#pragma once

#include <entt/entt.hpp>
#include "core/event/event.hpp"

namespace kogayonon_core
{

class EventDispatcher
{
public:
  EventDispatcher()
      : m_pDispatcher( std::make_shared<entt::dispatcher>() )
  {
  }

  ~EventDispatcher() = default;

  template <typename TEvent, auto Func, typename TEventHandler>
  auto addHandler( TEventHandler& handler );

  template <typename TEvent>
  bool hasHandlers( TEvent& event );

  template <typename TEvent>
  auto emitEvent( TEvent& event );

  template <typename TEvent>
  auto emitEvent( TEvent&& event );

  template <typename TEvent, auto Func, typename THandler>
  void removeListener( TEvent& event, THandler& handler );

  template <typename TEvent, typename THandler>
  void removeAllListeners( TEvent& event, THandler& handler );

private:
  std::shared_ptr<entt::dispatcher> m_pDispatcher;
};

} // namespace kogayonon_core

#include "core/event/event_dispatcher.inl"
