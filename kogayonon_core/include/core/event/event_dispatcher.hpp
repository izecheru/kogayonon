#pragma once
#include "event.hpp"

namespace kogayonon_core
{
class EventDispatcher
{
private:
  IEvent& m_event;

public:
  EventDispatcher( IEvent& event )
      : m_event( event )
  {
  }

  template <typename T, typename F>
  bool dispatchEventToListeners( const F& func )
  {
    if ( m_event.getEventType() == T::getStaticType() )
    {
      m_event.m_handled |= func( static_cast<T&>( m_event ) );
      return true;
    }
    return false;
  }
};
} // namespace kogayonon_core