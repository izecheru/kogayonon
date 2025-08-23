#pragma once
#include "event.h"

namespace kogayonon
{
  class EventDispatcher
  {
  private:
    Event& m_event;

  public:
    EventDispatcher(Event& event) : m_event(event) {}

    template <typename T, typename F>
    bool dispatch(const F& func)
    {
      if (m_event.getEventType() == T::getStaticType())
      {
        m_event.m_handled |= func(static_cast<T&>(m_event));
        return true;
      }
      return false;
    }
  };
} // namespace kogayonon