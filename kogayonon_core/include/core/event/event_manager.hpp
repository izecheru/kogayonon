#pragma once

#include <functional>
#include <unordered_map>

#include "core/event/event.hpp"

namespace kogayonon_core
{
class EventManager
{
  using EventCallbackFn = std::function<bool( IEvent& )>;

public:
  EventManager() = default;
  ~EventManager() = default;

  template <typename T>
  void listenToEvent( EventCallbackFn func )
  {
    if ( m_callbacks.find( T::getStaticType() ) == m_callbacks.end() )
    {
      m_callbacks.insert( { T::getStaticType(), std::vector<EventCallbackFn>{} } );
    }
    m_callbacks.at( T::getStaticType() ).push_back( func );
  }

  template <typename T>
  void unsubscribe()
  {
  }

  void dispatchEventToListeners( IEvent& e )
  {
    if ( auto it = m_callbacks.find( e.getEventType() ); it != m_callbacks.end() )
    {
      for ( const auto& callback : it->second )
      {
        e.m_handled |= callback( e );
      }
    }
  }

private:
  std::unordered_map<EventType, std::vector<EventCallbackFn>> m_callbacks;
};
} // namespace kogayonon_core