#pragma once
#include "event_dispatcher.hpp"

namespace kogayonon_core
{
template <typename TEvent, auto Func, typename TEventHandler>
auto EventDispatcher::addHandler( TEventHandler& handler )
{
  m_pDispatcher->sink<TEvent>().template connect<Func>( handler );
}

template <typename TEvent>
bool EventDispatcher::hasHandlers( TEvent& event )
{
  return !m_pDispatcher->sink<TEvent>().empty();
}

template <typename TEvent>
auto EventDispatcher::emitEvent( TEvent& event )
{
  m_pDispatcher->trigger( event );
}

template <typename TEvent>
auto EventDispatcher::emitEvent( TEvent&& event )
{
  m_pDispatcher->trigger( event );
}

template <typename TEvent, auto Func, typename THandler>
void EventDispatcher::removeListener( THandler& handler )
{
  m_pDispatcher->sink<TEvent>().template disconnect<Func>( handler );
}

template <typename TEvent, typename THandler>
void EventDispatcher::removeAllListeners( THandler& handler )
{
  m_pDispatcher->sink<TEvent>().template disconnect( handler );
}

} // namespace kogayonon_core
