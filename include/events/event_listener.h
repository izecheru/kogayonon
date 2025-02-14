#pragma once
#include <functional>
#include "core/singleton/singleton.h"
#include "event.h"

namespace kogayonon
{
  class EventListener :public Singleton<EventListener>
  {
  public:

    EventListener() {}
    using EventCallback = std::function<void(Event&)>;

    template<typename T>
    void subscribe(EventCallback callback) {
      auto type = T::getStaticType();
      m_listeners[type].push_back(callback);
    }

    void publish(Event& event) {
      auto type = event.getEventType();
      if (m_listeners.find(type) != m_listeners.end())
      {
        for (auto& callback : m_listeners[type])
        {
          callback(event);
        }
      }
    }

  private:
    std::unordered_map<EventType, std::vector<EventCallback>> m_listeners;
  };
}
