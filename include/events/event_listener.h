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
    using EventCallback = std::function<bool(Event&)>;

    template<typename T>
    void addCallback(EventCallback callback) {
      auto type = T::getStaticType();
      m_callbacks[type].push_back(callback);
    }

    void dispatch(Event& event) {
      auto it = m_callbacks.find(event.getEventType());
      if (it != m_callbacks.end()) {
        for (const auto& callback : it->second) {
          event.m_handled |= callback(event);
        }
      }
    }

  private:
    std::unordered_map<EventType, std::vector<EventCallback>> m_callbacks;
  };
}
